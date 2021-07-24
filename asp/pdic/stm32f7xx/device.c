/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2018 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id: device.c 698 2018-05-20 20:32:26Z roi $
 */
/*
 * STM32F7xx用デバイスドライバ
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "kernel_cfg.h"
#include "device.h"


/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

#define byte2bcd(value) ((((value)/10)<<4) | ((value) % 10))
#define bcd2byte(value) ((((value)>>4) * 10) + ((value) & 0xF))

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

uint_t dipsw_value;
void (*exti0_func[1])(void) = { 0 };
void (*exti15_func[NUM_EXTI15_FUNC])(void) = { 0 };


/*
 *  GPIOモードの内部定義
 */
#define GPIO_MODE               0x00000003
#define EXTI_MODE               0x10000000
#define GPIO_MODE_IT            0x00010000
#define GPIO_MODE_EVT           0x00020000
#define RISING_EDGE             0x00100000
#define FALLING_EDGE            0x00200000
#define GPIO_OUTPUT_TYPE        0x00000010

static const uint32_t gpio_index[] = {
	TADR_GPIOA_BASE,			/* index 0 */
	TADR_GPIOB_BASE,			/* index 1 */
	TADR_GPIOC_BASE,			/* index 2 */
	TADR_GPIOD_BASE,			/* index 3 */
	TADR_GPIOE_BASE,			/* index 4 */
	TADR_GPIOF_BASE,			/* index 5 */
	TADR_GPIOG_BASE,			/* index 6 */
	TADR_GPIOH_BASE,			/* index 7 */
	TADR_GPIOI_BASE,			/* index 8 */
	TADR_GPIOJ_BASE,			/* index 9 */
	TADR_GPIOK_BASE				/* index 10 */
};

#define NUM_OF_GPIOPORT (sizeof(gpio_index)/sizeof(uint32_t))

/*
 *  GPIOの初期設定関数
 */
void
gpio_setup(uint32_t base, GPIO_Init_t *init, uint32_t pin)
{
	uint32_t iocurrent = 1<<pin;
	uint32_t temp = 0x00;
	uint32_t index;

	for(index = 0 ; index < NUM_OF_GPIOPORT ; index++){
		if(gpio_index[index] == base)
			break;
	}
	if(index == NUM_OF_GPIOPORT)
		return;

	/*
	 *  GPIOモード設定
	 */
	/* アルタネート・ファンクション・モード設定 */
	temp = sil_rew_mem((uint32_t *)(base+TOFF_GPIO_AFR0+(pin>>3)*4));
	temp &= ~((uint32_t)0xF << ((uint32_t)(pin & (uint32_t)0x07) * 4)) ;
	if(init->mode == GPIO_MODE_AF)
		temp |= ((uint32_t)(init->alternate) << (((uint32_t)pin & (uint32_t)0x07) * 4));
	sil_wrw_mem((uint32_t *)(base+TOFF_GPIO_AFR0+(pin>>3)*4), temp);

	/*  入出力モード設定 */
	sil_modw_mem((uint32_t *)(base+TOFF_GPIO_MODER), (GPIO_MODER_MODER0 << (pin * 2)), ((init->mode & GPIO_MODE) << (pin * 2)));

	/*  出力モード設定 */
	if(init->mode == GPIO_MODE_OUTPUT || init->mode == GPIO_MODE_AF){
		sil_modw_mem((uint32_t *)(base+TOFF_GPIO_OSPEEDR), (GPIO_OSPEEDER_OSPEEDR0 << (pin * 2)), (init->speed << (pin * 2)));
		sil_modw_mem((uint32_t *)(base+TOFF_GPIO_OTYPER), (GPIO_OTYPER_OT_0 << pin), (init->otype << pin));
	}

	/*  プルアップ、プルダウン設定 */
	sil_modw_mem((uint32_t *)(base+TOFF_GPIO_PUPDR), (GPIO_PUPDR_PUPDR0 << (pin * 2)), (init->pull << (pin * 2)));

	/*
	 *  EXTIモード設定
	 */
	if((init->mode & EXTI_MODE) == EXTI_MODE){
		/*  SYSCFGクロック設定 */
		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_SYSCFGEN);
		temp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR));

		temp = sil_rew_mem((uint32_t *)(TADR_SYSCFG_BASE+TOFF_SYSCFG_EXTICR0+(pin & 0x0C)));
		temp &= ~(0x0F << (4 * (pin & 0x03)));
		temp |= (index << (4 * (pin & 0x03)));
		sil_wrw_mem((uint32_t *)(TADR_SYSCFG_BASE+TOFF_SYSCFG_EXTICR0+(pin & 0x0C)), temp);

		if((init->mode & GPIO_MODE_IT) == GPIO_MODE_IT)
			sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR), iocurrent);
		else
			sil_andw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR), iocurrent);

		if((init->mode & GPIO_MODE_EVT) == GPIO_MODE_EVT)
			sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_EMR), iocurrent);
		else
			sil_andw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_EMR), iocurrent);

		if((init->mode & RISING_EDGE) == RISING_EDGE)
			sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_RTSR), iocurrent);
		else
			sil_andw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_RTSR), iocurrent);

		if((init->mode & FALLING_EDGE) == FALLING_EDGE)
			sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_FTSR), iocurrent);
		else
			sil_andw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_FTSR), iocurrent);
	}
}


/*
 *  アドレス指定Dキャッシュインバリデート関数
 *  parameter1  addr    address (aligned to CACHE_LINE_SIZE(32)-byte boundary)
 *  parameter2  dsize   size of memory block (in number of bytes)
 */
void
invalidatedcache_by_addr(uint8_t *addr, int32_t dsize)
{
	int32_t  op_size = dsize + (CACHE_LINE_SIZE-1);
	uint32_t op_addr = (((uint32_t)addr) & ~(CACHE_LINE_SIZE-1));

	asm("dsb 0xF":::"memory");
	while (op_size > 0) {
		sil_wrw_mem((uint32_t *)(TADR_SCB_BASE+TOFF_SCB_DCIMVAC), op_addr);
		op_addr += CACHE_LINE_SIZE;
		op_size -= CACHE_LINE_SIZE;
	}
	asm("dsb 0xF":::"memory");
	asm("isb 0xF":::"memory");
}

/*
 *  アドレス指定Dキャッシュフラッシュ関数
 *  parameter1  addr    address (aligned to CACHE_LINE_SIZE(32)-byte boundary)
 *  parameter2  dsize   size of memory block (in number of bytes)
 */
void flushdcache_by_addr(uint8_t *addr, int32_t dsize)
{
	int32_t  op_size = dsize + (CACHE_LINE_SIZE-1);
	uint32_t op_addr = (((uint32_t)addr) & ~(CACHE_LINE_SIZE-1));

	asm("dsb 0xF":::"memory");
	while (op_size > 0) {
		sil_wrw_mem((uint32_t *)(TADR_SCB_BASE+TOFF_SCB_DCCMVAC), op_addr);
		op_addr += CACHE_LINE_SIZE;
		op_size -= CACHE_LINE_SIZE;
	}
	asm("dsb 0xF":::"memory");
	asm("isb 0xF":::"memory");
}

/*
 *  アドレス指定Dキャッシュフラッシュ&インバリデート関数
 *  parameter1  addr    address (aligned to CACHE_LINE_SIZE(32)-byte boundary)
 *  parameter2  dsize   size of memory block (in number of bytes)
 */
void flushinvalidatedcache_by_addr(uint8_t *addr, int32_t dsize)
{
	int32_t  op_size = dsize + (CACHE_LINE_SIZE-1);
	uint32_t op_addr = (((uint32_t)addr) & ~(CACHE_LINE_SIZE-1));

	asm("dsb 0xF":::"memory");
	while (op_size > 0) {
		sil_wrw_mem((uint32_t *)(TADR_SCB_BASE+TOFF_SCB_DCCIMVAC), op_addr);
		op_addr += CACHE_LINE_SIZE;
		op_size -= CACHE_LINE_SIZE;
	}
	asm("dsb 0xF":::"memory");
	asm("isb 0xF":::"memory");
}


/*
 *  DMACの設定関数
 */

#define NUM_STMDMA      16
#define NUM_DMAINDEX    5
#define INDEX_TC        0
#define INDEX_HT        1
#define INDEX_TE        2
#define INDEX_FE        3
#define INDEX_DME       4

#define DMA_TRS_TIMEOUT 2000	/* 2秒 */

static const uint32_t dma_base[NUM_STMDMA] = {
	TADR_DMA1_STM0_BASE,
	TADR_DMA1_STM1_BASE,
	TADR_DMA1_STM2_BASE,
	TADR_DMA1_STM3_BASE,
	TADR_DMA1_STM4_BASE,
	TADR_DMA1_STM5_BASE,
	TADR_DMA1_STM6_BASE,
	TADR_DMA1_STM7_BASE,
	TADR_DMA2_STM0_BASE,
	TADR_DMA2_STM1_BASE,
	TADR_DMA2_STM2_BASE,
	TADR_DMA2_STM3_BASE,
	TADR_DMA2_STM4_BASE,
	TADR_DMA2_STM5_BASE,
	TADR_DMA2_STM6_BASE,
	TADR_DMA2_STM7_BASE
};

static const uint32_t dma_flag[NUM_STMDMA][NUM_DMAINDEX] = {
	{	0x00000020, 0x00000010, 0x00000008, 0x00800001, 0x00800004 },
	{	0x00000800, 0x00000400, 0x00000200, 0x00000040, 0x00000100 },
	{	0x00200000, 0x00100000, 0x00080000, 0x00010000, 0x00040000 },
	{	0x08000000, 0x04000000, 0x02000000, 0x00400000, 0x01000000 },
	{	0x00000020, 0x00000010, 0x00000008, 0x00800001, 0x00800004 },
	{	0x00000800, 0x00000400, 0x00000200, 0x00000040, 0x00000100 },
	{	0x00200000, 0x00100000, 0x00080000, 0x00010000, 0x00040000 },
	{	0x08000000, 0x04000000, 0x02000000, 0x00400000, 0x01000000 },
	{	0x00000020, 0x00000010, 0x00000008, 0x00800001, 0x00800004 },
	{	0x00000800, 0x00000400, 0x00000200, 0x00000040, 0x00000100 },
	{	0x00200000, 0x00100000, 0x00080000, 0x00010000, 0x00040000 },
	{	0x08000000, 0x04000000, 0x02000000, 0x00400000, 0x01000000 },
	{	0x00000020, 0x00000010, 0x00000008, 0x00800001, 0x00800004 },
	{	0x00000800, 0x00000400, 0x00000200, 0x00000040, 0x00000100 },
	{	0x00200000, 0x00100000, 0x00080000, 0x00010000, 0x00040000 },
	{	0x08000000, 0x04000000, 0x02000000, 0x00400000, 0x01000000 }
};

static const uint32_t dma_faddr[NUM_STMDMA/4] = {
	(TADR_DMA1_BASE+TOFF_DMAI_LISR),
	(TADR_DMA1_BASE+TOFF_DMAI_HISR),
	(TADR_DMA2_BASE+TOFF_DMAI_LISR),
	(TADR_DMA2_BASE+TOFF_DMAI_HISR)
};

static DMA_Handle_t *pDmaHandler[NUM_STMDMA];


/*
 *  STREAM DMA初期化関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_init(DMA_Handle_t *hdma)
{
	uint32_t tmp = 0;
	uint32_t i;

	/* パラメータチェック */
	if(hdma == NULL)
		return E_PAR;

	for(i = 0 ; i < NUM_STMDMA ; i++){
		if(dma_base[i] == hdma->base)
			break;
	}
	if(i == NUM_STMDMA)
		return E_PAR;
	hdma->sdid = i;
	pDmaHandler[i] = hdma;

	/* DMA-CRレジスタ取得 */
	tmp = sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR));

	/* DMAモードビットをテンポラリィにクリア */
	tmp &= ((uint32_t)~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST | \
                      DMA_SxCR_PL    | DMA_SxCR_MSIZE  | DMA_SxCR_PSIZE  | \
                      DMA_SxCR_MINC  | DMA_SxCR_PINC   | DMA_SxCR_CIRC   | \
                      DMA_SxCR_DIR   | DMA_SxCR_CT     | DMA_SxCR_DBM));

	/* DMAモードをテンポラリィに設定 */
	tmp |=  hdma->Init.Channel           | hdma->Init.Direction        |
          hdma->Init.PeriphInc           | hdma->Init.MemInc           |
          hdma->Init.PeriphDataAlignment | hdma->Init.MemDataAlignment |
          hdma->Init.Mode                | hdma->Init.Priority;

	/* FIFOモードならバースト設定 */
	if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE){
		tmp |=  hdma->Init.MemBurst | hdma->Init.PeriphBurst;
	}

	/* DMAモード設定 */
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), tmp);

	/* DMA-FCRレジスタを取得 */
	tmp = sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR));

	/* FIFOモードをテンポラリィにクリア */
	tmp &= (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);

	/* FIFOモードをテンポラリィに設定 */
	tmp |= hdma->Init.FIFOMode;

	/* FIFOモードならスレッシュホールドを設定 */
	if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE){
		tmp |= hdma->Init.FIFOThreshold;
	}

	/* DMA-FCRレジスタに設定 */
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR), tmp);

	/* エラー状態をクリア */
	hdma->ErrorCode = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  STREAM DMA終了関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_deinit(DMA_Handle_t *hdma)
{
	int i;

	/* パラメータチェック */
	if(hdma == NULL)
		return E_PAR;

	for(i = 0 ; i < NUM_STMDMA ; i++){
		if(dma_base[i] == hdma->base)
			break;
	}
	if(i == NUM_STMDMA)
		return E_PAR;
	hdma->sdid = i;

	/* DMAイネーブル */
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_EN);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), 0);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_NDTR), 0);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_PAR), 0);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_M0AR), 0);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_M1AR), 0);
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR), 0x00000021);

	/* 各フラグをクリア */
	for(i = 0 ; i < NUM_DMAINDEX ; i++){
		sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][i]);
	}

	/* エラー状態をクリア */
	hdma->ErrorCode = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  STREAM DMA開始関数
 *  parameter1  hdma:       DMAハンドラへのポインタ
 *  parameter2  SrcAddress: ソースアドレス
 *  parameter3  DstAddress: デスティネーションアドレス
 *  parameter4  DataLength: 転送長
 *  return ER値
 */
ER
dma_start(DMA_Handle_t *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
	/* パラメータチェック */
	if(hdma == NULL)
		return E_PAR;

	/* DMA停止 */
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_EN);
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR), DMA_SxFCR_FEIE);
	hdma->status = DMA_STATUS_BUSY;

	/* DBMビットクリア */
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_DBM);
	/* データ長設定 */
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_NDTR), DataLength);

	if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH){
		/* メモリからデバイス */
		flushdcache_by_addr((uint8_t *)SrcAddress, DataLength*4);
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_PAR), DstAddress);
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_M0AR), SrcAddress);
	}
	else{	/* デバイスからメモリ */
		flushinvalidatedcache_by_addr((uint8_t *)DstAddress, DataLength*4);
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_PAR), SrcAddress);
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_M0AR), DstAddress);
	}

	/* 割込みイネーブル */
	sil_orw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
	sil_orw_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR), DMA_SxFCR_FEIE);
	/* DMA開始 */
	sil_orw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_EN);

	return E_OK;
}

/*
 *  STREAM DMA停止関数
 *  parameter1  hdma  : DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_end(DMA_Handle_t *hdma)
{
	uint32_t tick = 0;

	/* DMA停止 */
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_EN);

	/* DMA停止待ち */
	while((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_EN) != 0){
	    /* Check for the Timeout */
		if(tick > DMA_TRS_TIMEOUT){
			/* タイムアウトエラー設定 */
			hdma->ErrorCode |= DMA_ERROR_TIMEOUT;
			return E_TMOUT;
		}
    	dly_tsk(1);
	}
	hdma->status = 0;
	return E_OK;
}

/*
 *  STREAM DMA割込み処理関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 */
void
dma_inthandler(DMA_Handle_t *hdma)
{
	/*
	 *  転送エラー処理
	 */
	if((sil_rew_mem((uint32_t *)(dma_faddr[hdma->sdid/4])) & dma_flag[hdma->sdid][INDEX_TE]) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_TEIE) != 0){
			/* 転送エラー割込みクリア */
			sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_TEIE);
			/* 転送エラーフラグをクリア */
			sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_TE]);
			/* 転送エラー状態を設定 */
			hdma->ErrorCode |= DMA_ERROR_TE;
			hdma->status = DMA_STATUS_READY_ERROR;
			/* エラーコールバック */
			if(hdma->errorcallback != NULL)
				hdma->errorcallback(hdma);
		}
	}
	/*
	 *  FIFOエラー処理
	 */
	if((sil_rew_mem((uint32_t *)(dma_faddr[hdma->sdid/4])) & dma_flag[hdma->sdid][INDEX_FE]) != 0){
	    if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR)) & DMA_SxFCR_FEIE) != 0){
			/* FIFOエラー割込みをクリア */
			sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_FCR), DMA_SxFCR_FEIE);
			/* FIFOエラーフラグをクリア */
			sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_FE]);
			/* FIFOエラーを設定 */
			hdma->ErrorCode |= DMA_ERROR_FE;
			hdma->status = DMA_STATUS_READY_ERROR;
			/* エラーコールバック */
			if(hdma->errorcallback != NULL){
				hdma->errorcallback(hdma);
			}
		}
	}
	/*
	 *  ダイレクト・メモリ・エラー処理
	 */
	if((sil_rew_mem((uint32_t *)(dma_faddr[hdma->sdid/4])) & dma_flag[hdma->sdid][INDEX_DME]) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_DMEIE) != 0){
			/* DMEエラー割込みをクリア */
			sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_DMEIE);
			/* DMEエラーフラグをクリア */
			sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_DME]);
			/* DMEエラーを設定 */
			hdma->ErrorCode |= DMA_ERROR_DME;
			hdma->status = DMA_STATUS_READY_ERROR;
			/* エラーコールバック */
			if(hdma->errorcallback != NULL)
				hdma->errorcallback(hdma);
		}
	}
	/*
	 *  ハーフ転送終了処理
	 */
	if((sil_rew_mem((uint32_t *)(dma_faddr[hdma->sdid/4])) & dma_flag[hdma->sdid][INDEX_HT]) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_HTIE) != 0){
			/* マルチバッファモード */
			if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_DBM) != 0){
				/* ハーフ転送フラグをクリア */
				sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_HT]);

				/* メモリ０使用 */
				if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CT) == 0){
					/* ハーフメモリ０の状態設定 */
					hdma->status = DMA_STATUS_READY_HMEM0;
				}
				/* メモリ１使用 */
				else if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CT) != 0){
					/* ハーフメモリ１の状態設定 */
					hdma->status = DMA_STATUS_READY_HMEM1;
				}
			}
			else{
				if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CIRC) == 0){
					/* ハーフ転送割込みをクリア */
					sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_HTIE);
				}
				/* ハーフ転送完了フラグをクリア */
				sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_HT]);
				/* ハーフ転送ステータスに変更 */
				hdma->status = DMA_STATUS_READY_HMEM0;
			}
			/* ハーフ転送終了コールバック */
			if(hdma->xferhalfcallback != NULL)
				hdma->xferhalfcallback(hdma);
		}
	}
	/*
	 *  転送終了処理
	 */
	if((sil_rew_mem((uint32_t *)(dma_faddr[hdma->sdid/4])) & dma_flag[hdma->sdid][INDEX_TC]) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_TCIE) != 0){
			if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_DBM) != 0){
				/* 転送完了フラグをクリア */
				sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_TC]);
				/* メモリ１使用 */
				if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CT) == 0){
					/* メモリ1転送終了コールバック */
					if(hdma->xferm1callback != NULL)
						hdma->xferm1callback(hdma);
				}
				/* メモリ０使用 */
				else if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CT) != 0){
					/* 転送終了コールバック */
					if(hdma->xfercallback != NULL)
						hdma->xfercallback(hdma);
				}
			}
			else{
				if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CIRC) == 0){
					/* 転送割込みをクリア */
					sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR), DMA_SxCR_TCIE);
				}
				/* 転送完了フラグをクリア */
				sil_wrw_mem((uint32_t *)(dma_faddr[hdma->sdid/4]+8), dma_flag[hdma->sdid][INDEX_TC]);
				/* ステータスを終了へ */
				hdma->ErrorCode = DMA_ERROR_NONE;
				hdma->status = DMA_STATUS_READY_MEM0;
				/* 転送終了コールバック */
				if(hdma->xfercallback != NULL)
					hdma->xfercallback(hdma);
			}
		}
	}
}

/*
 *  STREAM DMA 割込みサービスルーチン
 */
void
stream_dma_isr(intptr_t exinf)
{
	dma_inthandler(pDmaHandler[(uint32_t)exinf]);
}


static DMA2D_Handle_t *pDma2dHandler;

/*
 *  DMA2D割込み関数
 */
void dma2d_inthandler(DMA2D_Handle_t *hdma2d)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_ISR));
	/*
	 *  転送エラー判定
	 */
	if((isr & DMA2D_ISR_TEIF) != 0){
		if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR)) & DMA2D_CR_TEIE) != 0){
			/* 転送エラー割込みDisable */
			sil_andw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_TEIE);
			hdma2d->ErrorCode |= DMA2D_ERROR_TE;	/* 転送エラー設定 */
			hdma2d->status = DMA2D_STATUS_ERROR;
			/* 転送エラー割込みクリア */
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_IFCR), DMA2D_IFSR_CTEIF);
			/* 転送エラーコールバック */
			if(hdma2d->errorcallback != NULL)
				hdma2d->errorcallback(hdma2d);
			isig_sem(DMA2DTRNSEM);
		}
	}
	/*
	 *  コンフィギュレーションエラー判定
	 */
	if((isr & DMA2D_ISR_CEIF) != 0){
		if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR)) & DMA2D_CR_CEIE) != 0){
			/* コンフィギュレーションエラーDisable */
			sil_andw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_CEIE);
			/* コンフィギュレーションエラー割込みクリア */
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_IFCR), DMA2D_IFSR_CCEIF);
			hdma2d->ErrorCode |= DMA2D_ERROR_CE;	/* コンフィギュレーションエラー設定 */
			hdma2d->status = DMA2D_STATUS_ERROR;
			/* コンフィギュレーションエラーコールバック */
			if(hdma2d->errorcallback != NULL)
				hdma2d->errorcallback(hdma2d);
			isig_sem(DMA2DTRNSEM);
		}
	}
	/*
	 *  転送終了割込み判定
	 */
	if((isr & DMA2D_ISR_TCIF) != 0){
		if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR)) & DMA2D_CR_TCIE) != 0){ 
			/* 転送終了割込みDisable */
			sil_andw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_TCIE);
			/* 転送終了割込みクリア */  
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_IFCR), DMA2D_IFSR_CTCIF);
			hdma2d->ErrorCode = DMA2D_ERROR_NONE;	/* エラー情報リセット */
			hdma2d->status = DMA2D_STATUS_TCOMP;
			/* 転送終了コールバック */
			if(hdma2d->xfercallback != NULL)
				hdma2d->xfercallback(hdma2d);
			isig_sem(DMA2DTRNSEM);
		}
	}
	/*
	 *  CLUTローディング転送割込み判定
	 */
	if((isr & DMA2D_ISR_CTCIF) != 0){
		if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR)) & DMA2D_CR_CTCIE) != 0){ 
			/* CLUT転送終了割込みDisable */
			sil_andw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_CTCIE);
			/* 転送終了割込みクリア */
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_IFCR), DMA2D_IFSR_CCTCIF);
			hdma2d->ErrorCode = DMA2D_ERROR_NONE;	/* エラー情報リセット */
			hdma2d->status = DMA2D_STATUS_CCOMP;
			/* 転送終了コールバック */
			if(hdma2d->xfercallback != NULL)
				hdma2d->xfercallback(hdma2d);
			isig_sem(DMA2DTRNSEM);
		}
	}
}

/*
 *  DMA2Dの初期設定
 *  parameter1 hdma2d    DMA2Dハンドラへのポインタ
 *  return               正常終了ならばE_OK
 */
ER
dma2d_init(DMA2D_Handle_t *hdma2d)
{
	volatile uint32_t tmp = 0;
	static bool_t init = false;

	if(hdma2d == NULL)
		return E_PAR;

	wai_sem(DMA2DSEM);	/* セマフォロック */
	pDma2dHandler = hdma2d;
	if(!init){
		/* クロック設定 */
		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2DEN);
		tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));
		(void)(tmp);
		init = true;
	}

	/* DMA2D CRレジスタ設定 */
	sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_MODE, hdma2d->Init.Mode);
	/* DMA2D OPFCCレジスタ設定 */
	sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OPFCCR), DMA2D_OPFCCR_CM, hdma2d->Init.ColorMode);
	/* DMA2D OORレジスタ設定 */
	sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OOR), DMA2D_OOR_LO, hdma2d->Init.OutputOffset);

#if defined(TOPPERS_STM32F767_NUCLEO144) || defined(TOPPERS_STM32F769_DISCOVERY)
	/* DMA2D OPFCCRレジスタ AI/RBSWAP設定 */
	sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OPFCCR), DMA2D_OPFCCR_AI, hdma2d->Init.AlphaInverted);
	sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OPFCCR), DMA2D_OPFCCR_RBS, hdma2d->Init.RedBlueSwap);
#endif

	/* エラー情報リセット */
	hdma2d->ErrorCode = DMA2D_ERROR_NONE;
	return E_OK;
}

/*
 *  DMA2Dレイヤー設定
 *  parameter1 hdma2d    DMA2Dハンドラへのポインタ
 *  parameter2 LayerIdx  DMA2Dレイヤーインデックス
 *  return               正常終了ならばE_OK
 */
ER
dma2d_configlayer(DMA2D_Handle_t *hdma2d, uint32_t LayerIdx)
{
	DMA2D_LayerCfg_t *pLayerCfg = &hdma2d->LayerCfg[LayerIdx];
	uint32_t tmp = 0;

	if(hdma2d->Init.Mode != DMA2D_R2M && hdma2d->Init.Mode != DMA2D_M2M){
		if(pLayerCfg->AlphaMode != DMA2D_NO_MODIF_ALPHA
			&& pLayerCfg->AlphaMode != DMA2D_REPLACE_ALPHA && pLayerCfg->AlphaMode != DMA2D_COMBINE_ALPHA){
			sig_sem(DMA2DSEM);
			return E_PAR;
		}
	}

	/* レイヤー0(background)DMA2D設定 */
	if(LayerIdx == 0){
		/* DMA2D BGPFCCRレジスタ設定 */
		tmp = sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_BGPFCCR));
		tmp &= (uint32_t)~(DMA2D_BGPFCCR_CM | DMA2D_BGPFCCR_AM | DMA2D_BGPFCCR_ALPHA); 
		if (pLayerCfg->InputColorMode == CM_A4 || pLayerCfg->InputColorMode == CM_A8)
			tmp |= (pLayerCfg->InputColorMode | (pLayerCfg->AlphaMode << 16) | ((pLayerCfg->InputAlpha) & 0xFF000000));
		else
			tmp |= (pLayerCfg->InputColorMode | (pLayerCfg->AlphaMode << 16) | (pLayerCfg->InputAlpha << 24));
		sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_BGPFCCR), tmp);

		/* DMA2D BGORレジスタ設定 */
		sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_BGOR), DMA2D_BGOR_LO, pLayerCfg->InputOffset);
		/* DMA2D BGCOLRレジスタ設定 */
		if ((pLayerCfg->InputColorMode == CM_A4) || (pLayerCfg->InputColorMode == CM_A8)){
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_BGCOLR), (pLayerCfg->InputAlpha & 0x00FFFFFF));
		}
	}
	/* レイヤー1(foreground)DMA2D設定 */
	else{
		/* DMA2D FGPFCCRレジスタ設定 */
		tmp = sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGPFCCR));
		tmp &= (uint32_t)~(DMA2D_FGPFCCR_CM | DMA2D_FGPFCCR_AM | DMA2D_FGPFCCR_ALPHA); 
		if (pLayerCfg->InputColorMode == CM_A4 || pLayerCfg->InputColorMode == CM_A8)
			tmp |= (pLayerCfg->InputColorMode | (pLayerCfg->AlphaMode << 16) | ((pLayerCfg->InputAlpha) & 0xFF000000));
		else
			tmp |= (pLayerCfg->InputColorMode | (pLayerCfg->AlphaMode << 16) | (pLayerCfg->InputAlpha << 24));
		sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGPFCCR), tmp);

		/* DMA2D FGORレジスタ設定 */
		sil_modw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGOR), DMA2D_FGOR_LO, pLayerCfg->InputOffset);
		/* DMA2D FGCOLRレジスタ設定 */
		if ((pLayerCfg->InputColorMode == CM_A4) || (pLayerCfg->InputColorMode == CM_A8)){
			sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGCOLR), (pLayerCfg->InputAlpha & 0x00FFFFFF));
		}
	}
	return E_OK;
}

/*
 *  DMA2D転送スタート
 *  parameter1 hdma2d    DMA2Dハンドラへのポインタ
 *  parameter2 pdata     ソースメモリバッファアドレス
 *  parameter3 DstAddr   デスティネーションメモリバッファアドレス
 *  parameter4 Width     転送データ幅
 *  parameter5 Height    転送データ高さ
 *  return               正常終了ならばE_OK
 */
ER
dma2d_start(DMA2D_Handle_t *hdma2d, uint32_t pdata, uint32_t DstAddress, uint32_t Width,  uint32_t Height)
{
	if(hdma2d == NULL)
		return E_PAR;
	/* DMA2D Disable */
	sil_andw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_START);

	/* DMA2Dデータサイズ設定 */
	sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_NLR), (Width << 16) | Height);

	/* DMA2Dのデスティネーションアドレス設定 */
	sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OMAR), DstAddress);

	/* DMA2Dメモリモード選択 */
	if (hdma2d->Init.Mode == DMA2D_R2M){
		uint32_t tmp = 0;
		uint32_t tmp1 = 0;
		uint32_t tmp2 = 0;
		uint32_t tmp3 = 0;
		uint32_t tmp4 = 0;

		tmp1 = pdata & DMA2D_OCOLR_ALPHA_1;
		tmp2 = pdata & DMA2D_OCOLR_RED_1;
		tmp3 = pdata & DMA2D_OCOLR_GREEN_1;
		tmp4 = pdata & DMA2D_OCOLR_BLUE_1;

		/* Prepare the value to be wrote to the OCOLR register according to the color mode */
		if (hdma2d->Init.ColorMode == DMA2D_ARGB8888){
			tmp = (tmp3 | tmp2 | tmp1| tmp4);
		}
		else if (hdma2d->Init.ColorMode == DMA2D_RGB888){
			tmp = (tmp3 | tmp2 | tmp4);  
		}
		else if (hdma2d->Init.ColorMode == DMA2D_RGB565){
			tmp2 = (tmp2 >> 19);
			tmp3 = (tmp3 >> 10);
			tmp4 = (tmp4 >> 3 );
			tmp  = ((tmp3 << 5) | (tmp2 << 11) | tmp4); 
		}
 		else if (hdma2d->Init.ColorMode == DMA2D_ARGB1555){
			tmp1 = (tmp1 >> 31);
			tmp2 = (tmp2 >> 19);
			tmp3 = (tmp3 >> 11);
			tmp4 = (tmp4 >> 3 );
			tmp  = ((tmp3 << 5) | (tmp2 << 10) | (tmp1 << 15) | tmp4);
		}
		else{ /* DMA2D_CMode = DMA2D_ARGB4444 */
			tmp1 = (tmp1 >> 28);
			tmp2 = (tmp2 >> 20);
			tmp3 = (tmp3 >> 12);
			tmp4 = (tmp4 >> 4 );
			tmp  = ((tmp3 << 4) | (tmp2 << 8) | (tmp1 << 12) | tmp4);
		}
		/* Write to DMA2D OCOLR register */
		sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_OCOLR), tmp);
	}
	else{ /* M2M, M2M_PFC or M2M_Blending DMA2D Mode */
		/* DMA2Dソースアドレス設定 */
		sil_wrw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGMAR), pdata);
	}
	/*
	 *  転送割込みを許可
	 */
	sil_orw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), (DMA2D_CR_TEIE | DMA2D_CR_CEIE | DMA2D_CR_TCIE));

	/* DMA2D Enable */
	sil_orw_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR), DMA2D_CR_START);
	hdma2d->status = DMA2D_STATUS_BUSY;
	return E_OK;
}

/*
 *  DMA2D転送待ち
 *  parameter1 hdma2d    DMA2Dハンドラへのポインタ
 *  parameter2 Timeout   待ち時間(MS)
 *  return               正常終了ならばE_OK
 */
ER
dma2d_waittransfar(DMA2D_Handle_t *hdma2d, uint32_t Timeout)
{
	ER       ercd = E_OK;
	uint32_t time = 0;

	/*
	 *  転送設定がある場合のポーリング待ち
	 */
	if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_CR)) & DMA2D_CR_START) != 0){
		while(hdma2d->status == DMA2D_STATUS_BUSY){
			if(time > Timeout){	/* タイムアウト時間をチェック */
				hdma2d->ErrorCode |= DMA2D_ERROR_TIMEOUT;	/* タイムアウトエラー設定 */
				sig_sem(DMA2DSEM);
				return E_TMOUT;
			}
			twai_sem(DMA2DTRNSEM, 1);
			time++;
		}
		if(hdma2d->status == DMA2D_STATUS_ERROR)
			ercd = E_OBJ;
	}
	/*
	 *  CLUTローディングの処理待ち
	 */
	time = 0;
	if((sil_rew_mem((uint32_t *)(hdma2d->base+TOFF_DMA2D_FGPFCCR)) & DMA2D_FGPFCCR_START) != 0){
		while(hdma2d->status == DMA2D_STATUS_BUSY){
			if(time > Timeout){	/* タイムアウト時間をチェック */
				hdma2d->ErrorCode |= DMA2D_ERROR_TIMEOUT;	/* タイムアウトエラー設定 */
				sig_sem(DMA2DSEM);
				return E_TMOUT;
			}
			twai_sem(DMA2DTRNSEM, 1);
			time++;
		}
		if(hdma2d->status == DMA2D_STATUS_ERROR)
			ercd = E_OBJ;
	}
	sig_sem(DMA2DSEM);	/* セマフォアンロック */
	return ercd;
}

/*
 *  DMA2D割込みハンドラ
 */
void dma2d_handler(void)
{
	dma2d_inthandler(pDma2dHandler);
}


#define RTC_ASYNCH_PREDIV       0x7F		/* LSE用 RTCクロック */
#define RTC_SYNCH_PREDIV        0x00FF

#define RTC_TR_RESERVED_MASK    (RTC_TR_SU | RTC_TR_ST | RTC_TR_MNU | RTC_TR_MNT | \
								 RTC_TR_HU | RTC_TR_HT | RTC_TR_PM)
#define RTC_DR_RESERVED_MASK    (RTC_DR_DU | RTC_DR_DT  | RTC_DR_MU | RTC_DR_MT | \
								 RTC_DR_WDU | RTC_DR_YU | RTC_DR_YT)
#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)  
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)

#define RTC_TIMEOUT_VALUE       (1000*1000)
#define LSI_TIMEOUT_VALUE       (100*1000)
#define RCC_DBP_TIMEOUT_VALUE   (100*1000)
#define RCC_LSE_TIMEOUT_VALUE   (5000*1000)

#define RTC_EXTI_LINE_WUPTIMER  ((uint32_t)EXTI_IMR_MR22)	/* External interrupt line 22 Connected to the RTC Wake-up event */

uint32_t rtcerrorcode;

static void (*rtcalarmA_callback)(void);	/* DMA transfer complete Memory1 callback */
static void (*rtcalarmB_callback)(void);	/* DMA transfer error callback */
static void (*rtcwakeup_callback)(void);	/* rtc wakeup callback */

/*
 *  RTCエントリモード設定
 */
static ER
rtc_requestInitMode(void)
{
	uint32_t tick = 0;

	/*
	 * イニシャルモード判定
	 */
	if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INITF) == 0){
	    /*
		 *  イニシャルモード設定
		 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), RTC_INIT_MASK);
	    while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INITF) == 0){
			if(tick > RTC_TIMEOUT_VALUE){
				return E_TMOUT;
			}
		}
		sil_dly_nse(1000);
		tick++;
	}
	return E_OK;
}

/*
 *  RTC用初期化
 */
void
rtc_init(intptr_t exinf)
{
	uint32_t tick = 0;
	volatile uint32_t tmpreg;

	rtcerrorcode = 0;
	/*
	 *  LSIをオフする
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CSR), RCC_CSR_LSION);
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CSR)) & RCC_CSR_LSIRDY) != 0){
		if( tick > LSI_TIMEOUT_VALUE){
			rtcerrorcode |= RTC_ERROR_LSI;
			return;
		}
		sil_dly_nse(1000);
		tick++;
	}

	/*
	 *  LSEをオンする
	 */
    /* Enable Power Clock*/
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR), RCC_APB1ENR_PWREN);
	tmpreg = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR));

	/* Enable write access to Backup domain */
	sil_orw_mem((uint32_t *)(TADR_PWR_BASE+TOFF_PWR_CR1), PWR_CR1_DBP);

	/* Wait for Backup domain Write protection disable */
	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_PWR_BASE+TOFF_PWR_CR1)) & PWR_CR1_DBP) == 0){
		if(tick > RCC_DBP_TIMEOUT_VALUE){
			rtcerrorcode |= RTC_ERROR_LSE;
			return;
		}
		sil_dly_nse(1000);
		tick++;
	}
    
    /*
	 *  LSEON / LSEBYPリセット
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_LSEBYP);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_LSEON);

	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR)) & RCC_BDCR_LSERDY) != 0){
		if(tick > RCC_LSE_TIMEOUT_VALUE){
			rtcerrorcode |= RTC_ERROR_LSE;
			return;
		}
		sil_dly_nse(1000);
		tick++;
	}
    
    /*
	 *  LSE初期設定
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_LSEBYP);
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_LSEON);

	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR)) & RCC_BDCR_LSERDY) == 0){
		if(tick > RCC_LSE_TIMEOUT_VALUE){
			rtcerrorcode |= RTC_ERROR_LSE;
			return;
		}
		sil_dly_nse(1000);
		tick++;
	}

    /*
	 *  RTCクロック設定
	 */
	if((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR)) & RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_0){
		/*
		 *  クロック電源設定
		 */
		tmpreg = (sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR)) & ~(RCC_BDCR_RTCSEL));

		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_BDRST);
		sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_BDRST);

		sil_wrw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), tmpreg);

		if((tmpreg & RCC_BDCR_LSERDY) != 0){
			tick = 0;
			/* Wait till LSE is ready */  
			while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR)) & RCC_BDCR_LSERDY) == 0){
				if(tick > RCC_LSE_TIMEOUT_VALUE){
					rtcerrorcode |= RTC_ERROR_RTC;
					return;
				}
				sil_dly_nse(1000);
				tick++;
			}
		}
		sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CFGR), RCC_CFGR_RTCPRE);
		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_RTCSEL_0);
	}

	/*
	 *  RTCのクロック設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_BDCR), RCC_BDCR_RTCEN);

	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	/*
	 *  RTC初期設定
	 */
	if(rtc_requestInitMode() != E_OK){
		rtcerrorcode |= RTC_ERROR_RTC;
	    /* プロテクション設定 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
		return;
	}
	else{ 
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), (RTC_CR_FMT | RTC_CR_OSEL | RTC_CR_POL));

		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_PRER), RTC_SYNCH_PREDIV);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_PRER), (RTC_ASYNCH_PREDIV << 16));

		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), RTC_ISR_INIT);
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_OR), RTC_OR_ALARMTYPE);

	    /* プロテクション設定 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
		return;
	}
}

/*
 *  RTCの時刻設定関数
 *
 *  時刻の設定はPONIXのtm構造体を使用する
 *  PONIXのインクルードがない場合を考慮し、同一項目のtm2をドライバとして定義する。
 */
ER
rtc_set_time(struct tm2 *pt)
{
	uint32_t timetmp = 0;
	uint32_t datetmp = 0;

	if(pt == NULL)
		return E_PAR;
	if(rtcerrorcode != 0)
		return E_SYS;

	SVC_PERROR(wai_sem(RTCSEM));

	datetmp = (((uint32_t)byte2bcd(pt->tm_year - 30) << 16) |
				((uint32_t)byte2bcd(pt->tm_mon) << 8) |
				((uint32_t)byte2bcd(pt->tm_mday)) |
				((uint32_t)pt->tm_wday << 13));
    timetmp = (uint32_t)(((uint32_t)byte2bcd(pt->tm_hour) << 16) |
				((uint32_t)byte2bcd(pt->tm_min) << 8) |
				((uint32_t)byte2bcd(pt->tm_sec)));

	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	/*
	 *  初期化モード設定
	 */
	if(rtc_requestInitMode() != E_OK){
		/* プロテクション設定 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
		SVC_PERROR(sig_sem(RTCSEM));
		return E_TMOUT;
	}
	else{
		/*
		 *  日付、時刻設定
		 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_DR), (datetmp & RTC_DR_RESERVED_MASK));
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_TR), (timetmp & RTC_TR_RESERVED_MASK));

		/*  初期化モード終了 */
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_BCK);
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), RTC_ISR_INIT);

		/*
		 *  同期設定
		 */
		if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR)) & RTC_CR_BYPSHAD) == 0){
			uint32_t tick = 0;

			sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), RTC_RSF_MASK);
    		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_RSF) == 0){
				if(tick > (RTC_TIMEOUT_VALUE/1000)){
					SVC_PERROR(sig_sem(RTCSEM));
					return E_TMOUT;
				}
				dly_tsk(1);
				tick++;
			}
		}

		/* プロテクション設定 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
	}
	SVC_PERROR(sig_sem(RTCSEM));
	return E_OK;
}

/*
 *  RTCの時刻取り出し関数
 *
 *  時刻の設定はPONIXのtm構造体を使用する
 *  PONIXのインクルードがない場合を考慮し、同一項目のtm2をドライバとして定義する。
 */
ER
rtc_get_time(struct tm2 *pt)
{
	uint32_t timetmp = 0;
	uint32_t datetmp = 0;

	if(pt == NULL)
		return E_PAR;
	if(rtcerrorcode != 0)
		return E_SYS;

	SVC_PERROR(wai_sem(RTCSEM));
	/*
	 *  時刻取得
	 */
	timetmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_TR)) & RTC_TR_RESERVED_MASK;
	pt->tm_hour = (uint8_t)bcd2byte((uint8_t)((timetmp & (RTC_TR_HT | RTC_TR_HU)) >> 16));
	pt->tm_min = (uint8_t)bcd2byte((uint8_t)((timetmp & (RTC_TR_MNT | RTC_TR_MNU)) >>8));
	pt->tm_sec = (uint8_t)bcd2byte((uint8_t)(timetmp & (RTC_TR_ST | RTC_TR_SU)));
  
	/*
	 *  日付取得
	 */
	datetmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_DR)) & RTC_DR_RESERVED_MASK;
	pt->tm_year = (uint8_t)bcd2byte((uint8_t)((datetmp & (RTC_DR_YT | RTC_DR_YU)) >> 16)) + 30;
	pt->tm_mon = (uint8_t)bcd2byte((uint8_t)((datetmp & (RTC_DR_MT | RTC_DR_MU)) >> 8));
	pt->tm_mday = (uint8_t)bcd2byte((uint8_t)(datetmp & (RTC_DR_DT | RTC_DR_DU)));
	pt->tm_wday = (uint8_t)((datetmp & (RTC_DR_WDU)) >> 13); 

	SVC_PERROR(sig_sem(RTCSEM));
	return E_OK;
}

/*
 *  RTCアラーム設定
 *  parameter1 : parm: Pointer to Alarm structure
 *  parameter2 : ptm: Pointer to struct tm2
 *  return ERコード
 */
ER
rtc_setalarm(RTC_Alarm_t *parm, struct tm2 *ptm)
{
	uint32_t tick = 0;
	uint32_t tmparm = 0, subsecond = 0;
	uint32_t tmp, day;

	if(parm == NULL || ptm == NULL)
		return E_PAR;
	if(rtcerrorcode != 0)
		return E_SYS;

	if(parm->dayselect == ALARMDAYSEL_DATE)
		day = ptm->tm_mday;
	else
		day = ptm->tm_wday;

	/*
	 *  ALARM-AB設定レジスタ値を取得
	 */
	SVC_PERROR(wai_sem(RTCSEM));
    tmparm = ((byte2bcd(ptm->tm_hour) << 16) | (byte2bcd(ptm->tm_min) << 8) |
              (byte2bcd(ptm->tm_sec)) | (byte2bcd(day) << 24) |
              ((uint32_t)parm->dayselect) | (parm->alarmmask)); 
	subsecond = (uint32_t)(parm->subsecond | parm->subsecondmask);

	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	if(parm->alarm == RTC_ALARM_A){
		/*
		 *  ALARM-A設定、レジスタ初期化
		 */
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRAE);
		tmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR));
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), (~(RTC_ISR_ALRAF | RTC_ISR_INIT) | tmp));
		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_ALRAWF) == 0){
			if(tick > (RTC_TIMEOUT_VALUE/1000)){
			    /* プロテクション設定 */
				sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
				SVC_PERROR(sig_sem(RTCSEM));
				return E_TMOUT;
			}
			dly_tsk(1);
			tick++;
		}

		/*
		 *  ALARM-A設定
		 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMAR), tmparm);
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMASSR), subsecond);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRAE);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRAIE);
		rtcalarmA_callback = parm->callback;
	}
	else{
		/*
		 *  ALARM-B設定、レジスタ初期化
		 */
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRBE);
		tmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR));
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), (~(RTC_ISR_ALRBF | RTC_ISR_INIT) | tmp));
		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_ALRBWF) == 0){
			if(tick > (RTC_TIMEOUT_VALUE/1000)){
			    /* プロテクション設定 */
				sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
				SVC_PERROR(sig_sem(RTCSEM));
				return E_TMOUT;
			}
			dly_tsk(1);
			tick++;
		}

		/*
		 *  ALARM-B設定
		 */
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMBR), tmparm);
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMBSSR), subsecond);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRBE);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRBIE);
		rtcalarmB_callback = parm->callback;
	}

	/*
	 * RTC ALARM用EXTI設定
	 */
	sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR), EXTI_IMR_MR17);
	sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_RTSR), EXTI_RTSR_TR17);

    /* プロテクション設定 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
	SVC_PERROR(sig_sem(RTCSEM));
	return E_OK;
}

/*
 *  RTCアラーム停止
 *  parameter1 : Alarm: アラーム設定
 *  return ERコード
 */
ER
rtc_stopalarm(uint32_t Alarm)
{
	uint32_t tick = 0;

	if(rtcerrorcode != 0)
		return E_SYS;
	SVC_PERROR(wai_sem(RTCSEM));
	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	if(Alarm == RTC_ALARM_A){
		/*
		 *  ALARM-A割込みイネーブル解除
		 */
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRAE);
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRAIE);
		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_ALRAWF) == 0){
			if(tick > (RTC_TIMEOUT_VALUE/1000)){
			    /* プロテクション設定 */
				sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
				SVC_PERROR(sig_sem(RTCSEM));
				return E_TMOUT;
			}
			dly_tsk(1);
			tick++;
		}
		rtcalarmA_callback = NULL;
	}
	else{
		/*
		 *  ALARM-B割込みイネーブル解除
		 */
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRBE);
		sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_ALRBIE);
		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_ALRBWF) == 0){
			if( tick > RTC_TIMEOUT_VALUE){
			    /* プロテクション設定 */
				sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
				SVC_PERROR(sig_sem(RTCSEM));
				return E_TMOUT;
			}
			dly_tsk(1);
			tick++;
		}
		rtcalarmB_callback = NULL;
	}
    /* プロテクション設定 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
	SVC_PERROR(sig_sem(RTCSEM));
	return E_OK; 
}

/*
 *  RTCアラーム設定値取得
 *  parameter1 : parm: Pointer to Alarm structure
 *  parameter2 : ptm: Pointer to struct tm2
 *  return ERコード
 */
ER
rtc_getalarm(RTC_Alarm_t *parm, struct tm2 *ptm, uint32_t Alarm)
{
	uint32_t tmparm = 0;
	uint32_t subsecond = 0;

	if(parm == NULL || ptm == NULL)
		return E_PAR;
	if(rtcerrorcode != 0)
		return E_SYS;

	SVC_PERROR(wai_sem(RTCSEM));
	if(Alarm == RTC_ALARM_A){
		/*
		  ALARM-A レジスタ取得
		 */
		parm->alarm = RTC_ALARM_A;
		tmparm = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMAR));
		subsecond = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMASSR)) & RTC_ALRMASSR_SS;
	}
	else{
		/*
		  ALARM-B レジスタ取得
		 */
		parm->alarm = RTC_ALARM_B;
		tmparm = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMBR));
		subsecond = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ALRMBSSR)) & RTC_ALRMBSSR_SS;
	}

	/*
	 *  レジスタからパラメータに変換
	 */
	ptm->tm_hour = bcd2byte((uint8_t)((tmparm & (RTC_ALRMAR_HT | RTC_ALRMAR_HU)) >> 16));
	ptm->tm_min  = bcd2byte((uint8_t)((tmparm & (RTC_ALRMAR_MNT | RTC_ALRMAR_MNU)) >> 8));
	ptm->tm_sec = bcd2byte((uint8_t)(tmparm & (RTC_ALRMAR_ST | RTC_ALRMAR_SU)));
	parm->subsecond = (uint32_t) subsecond;
	parm->dayselect = (uint32_t)(tmparm & RTC_ALRMAR_WDSEL);
	if(parm->dayselect == ALARMDAYSEL_DATE)
		ptm->tm_mday = bcd2byte((uint8_t)((tmparm & (RTC_ALRMAR_DT | RTC_ALRMAR_DU)) >> 24));
	else
		ptm->tm_wday = bcd2byte((uint8_t)((tmparm & (RTC_ALRMAR_DT | RTC_ALRMAR_DU)) >> 24));

	parm->alarmmask = (uint32_t)(tmparm & ALARMMASK_ALL);
	SVC_PERROR(sig_sem(RTCSEM));
	return E_OK;
}

/*
 *  RTC WAKEUPタイマ初期化
 *  parameter1 : parm: Pointer to wakeup structure
 *  parameter2 : func: callback function
 *  return ERコード
 */
ER
rtc_wakeup_init(RTC_Wakeup_t *parm, void (*func)(void))
{
	uint32_t prer, isr;

	if(parm == NULL)
		return E_PAR;
	parm->wakeuptimerPrescaler = (4 - (sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR)) & RTC_CR_WUCKSEL));
	prer = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_PRER));
	parm->asynchPrescaler = ((prer >> 16) & 0x7f) + 1;
	parm->synchPrescaler  = prer & 0xffff;

	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_WUTE);
	isr  = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INIT;
	isr |= ~(RTC_ISR_WUTF | RTC_ISR_INIT);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), isr);
	sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), RTC_EXTI_LINE_WUPTIMER);
	sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_WUTIE);
	/*
	 *  EXTI WAKEUPLINE設定
	 */
	sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_RTSR), RTC_EXTI_LINE_WUPTIMER);
	sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR), RTC_EXTI_LINE_WUPTIMER);

    /* プロテクション設定 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);

	rtcwakeup_callback = func;
	return E_OK;
}

/*
 *  RTC WAKEUP時間設定
 *  parameter1 : parm: WAKEUP時間
 *  return ERコード
 */
ER
rtc_setup_wakeuptime(uint32_t time)
{
	uint32_t timeout = LSI_TIMEOUT_VALUE;
	/*
	 *  WAUPTIMER設定停止
	 */
	if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR)) & RTC_CR_WUTE) != 0){
		while((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_WUTWF) == 0){
			timeout--;
			if(timeout == 0)
				return E_TMOUT;
			sil_dly_nse(1000);
		}
	}
	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);

	sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_WUTE);

	if(time != 0 && time < 0xFFFF){
		sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), RTC_EXTI_LINE_WUPTIMER);
		sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WUTR), time);
		sil_orw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_WUTE);
	}
    /* プロテクション設定 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
	return E_OK;
}

/*
 *  SSRの取り出し
 */
uint32_t
rtc_get_ssr(void)
{
	return sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_SSR));
}

/*
 *  RTC割込みハンドラ
 */
void rtc_handler(void)
{
	uint32_t tmp;

	if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_CR_ALRAE) != 0){
		/*
		 * ALARM-A割込み確認
		 */
		if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR)) & RTC_CR_ALRAIE) != 0){
			/*
			 *  ALARM-Aコールバック
			 */
			if(rtcalarmA_callback != NULL)
				rtcalarmA_callback();

			/* プロテクション解除 */
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);
			/*
			 *  ALARM-A割込みクリア
			 */
			tmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INIT;
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), (~(RTC_ISR_ALRAF | RTC_ISR_INIT) | tmp));
		    /* プロテクション設定 */
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
		}
	}
	if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_CR_ALRBE) != 0){
		/*
		 * ALARM-B割込み確認
		 */
		if((sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR)) & RTC_CR_ALRBIE) != 0){
			/*
			 *  ALARM-Bコールバック
			 */
			if(rtcalarmB_callback != NULL)
				rtcalarmB_callback();

			/* プロテクション解除 */
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);
			/*
			 *  ALARM-B割込みクリア
			 */
			tmp = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INIT;
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), (~(RTC_ISR_ALRBF | RTC_ISR_INIT) | tmp));
		    /* プロテクション設定 */
			sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);
		}
	}
	/*
	 *  EXTI RTCペンディングクリア
	 */
	sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), EXTI_PR_PR17);
}

/*
 *  WAKEUP割込みハンドラ
 */
void wakeup_handler(void)
{
	uint32_t isr;

	/* プロテクション解除 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xCA);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0x53);
	/*
	 *  WAKEUP TIMER停止
	 */
	sil_andw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_CR), RTC_CR_WUTE);

	/*
 	 *  WUTF flagクリア
	 */
	isr  = sil_rew_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR)) & RTC_ISR_INIT;
	isr |= ~(RTC_ISR_WUTF | RTC_ISR_INIT);
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_ISR), isr);

    /* プロテクション設定 */
	sil_wrw_mem((uint32_t *)(TADR_RTC_BASE+TOFF_RTC_WPR), 0xFF);

	/*
	 *  EXTIの割込みクリア
	 */
	sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), RTC_EXTI_LINE_WUPTIMER);

	/*
	 *  コールバック関数実行
	 */
	if(rtcwakeup_callback != NULL)
		rtcwakeup_callback();
}


/*
 *  LED接続ポート
 *
 *  拡張I/OボードのLED1-3はプログラマブル入出力ポート0に
 *  にインバータを介して接続されている．
 */

typedef struct gio_confstruct
{
	uint32_t    clkbit;
	uint32_t    giobase;
	uint32_t    pinpos;
	uint32_t    pinpp;
} gio_conftype;

#if defined(TOPPERS_STM32F7_DISCOVERY)
static const gio_conftype led_confdata[] = {
	{ RCC_AHB1ENR_GPIOIEN, TADR_GPIOI_BASE, PINPOSITION1, GPIO_PULLUP}
};
#elif defined(TOPPERS_STM32F769_DISCOVERY)
static const gio_conftype led_confdata[] = {
	{ RCC_AHB1ENR_GPIOJEN, TADR_GPIOJ_BASE, PINPOSITION13, GPIO_PULLUP},
	{ RCC_AHB1ENR_GPIOJEN, TADR_GPIOJ_BASE, PINPOSITION5, GPIO_PULLUP}
};
#elif defined(TOPPERS_STM32F723_DISCOVERY)
static const gio_conftype led_confdata[] = {
	{ RCC_AHB1ENR_GPIOAEN, TADR_GPIOA_BASE, PINPOSITION7, GPIO_PULLUP},
	{ RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, PINPOSITION1, GPIO_PULLUP}
};
#else
static const gio_conftype led_confdata[] = {
	{ RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, PINPOSITION0, GPIO_PULLUP},
	{ RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, PINPOSITION7, GPIO_PULLUP},
	{ RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, PINPOSITION14, GPIO_PULLUP}
};
#endif
#define NUM_LED   (sizeof(led_confdata)/sizeof(gio_conftype))

/*
 *  LED接続ポート初期化
 */ 
void
led_init(intptr_t exinf)
{
	const gio_conftype *pled = &led_confdata[0];
	GPIO_Init_t GPIO_Init_Data;
	uint32_t i;

	for(i = 0 ; i < NUM_LED ; pled++, i++){
		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), pled->clkbit);
		/* モード設定 */
		GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;
		/* プルアップ プロダウン設定 */
		GPIO_Init_Data.pull      = pled->pinpp;
		/* 出力モード設定 */
		GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
        /* スピード設定 */
		GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
		gpio_setup(pled->giobase, &GPIO_Init_Data, pled->pinpos);

		/* 消灯 */
		sil_wrw_mem((uint32_t *)(pled->giobase+TOFF_GPIO_BSRR), (1 << pled->pinpos)<<16);
	}
}

/*
 *  LED接続ポート読み出し
 */
uint_t
led_in(void)
{
	const gio_conftype *pled = &led_confdata[0];
	uint32_t data, i;

	for(i = 0, data = 0 ; i < NUM_LED ; pled++, i++){
		if((sil_rew_mem((uint32_t *)(pled->giobase+TOFF_GPIO_IDR)) & (1<<(pled->pinpos))) != 0)
			data |= (1<<i);
	}
	return data;
}

/*
 *  LED接続ポート書き込み
 */ 
void
led_out(unsigned int led_data)
{
	const gio_conftype *pled = &led_confdata[0];
	uint32_t reg1, reg2, i;

	/* 設定値はLED接続ビット以外は変更しない */
	for(i = 0 ; i < NUM_LED ; pled++, i++){
		reg1 = reg2 = 0;
		if((led_data & (1<<i)) != 0)
			reg2 = 1 << pled->pinpos;
		else
			reg1 = 1 << pled->pinpos;

		/* 書き込みデータを生成して書き込み */
		sil_wrw_mem((uint32_t *)(pled->giobase+TOFF_GPIO_BSRR), (reg1<<16) | reg2);
	}
}

/*
 * LEDとスイッチの個別設定・読み込み関数群
 */

/*
 *  LEDの状態保存用変数
 */	
unsigned int LedState;


/*
 *  LED点灯制御
 */
void 
set_led_state(unsigned int led, unsigned char state){
	if (state == ON) {
		LedState = LedState | led;
	} else {
		LedState = LedState & ~led;
	}
	led_out(LedState);
}


#if defined(TOPPERS_STM32F7_DISCOVERY)
#define TADR_PSW_BASE   TADR_GPIOI_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOIEN
#define	PSW_PIN         PINPOSITION11
#elif defined(TOPPERS_STM32F769_DISCOVERY) || defined(TOPPERS_STM32F723_DISCOVERY)
#define TADR_PSW_BASE   TADR_GPIOA_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOAEN
#define	PSW_PIN         PINPOSITION0
#else
#define TADR_PSW_BASE   TADR_GPIOC_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOCEN
#define	PSW_PIN         PINPOSITION13
#endif

/*
 * PUSHスイッチ接続ポート初期化
 */
void
switch_push_init(intptr_t exinf)
{
	GPIO_Init_t GPIO_Init_Data;
	volatile uint32_t temp;

	/* 入力ポートに設定 */
	/* Enable the BUTTON Clock */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), PSWGIOPEN);
	temp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));
	(void)(temp);
	/* モード設定 */
	GPIO_Init_Data.mode      = GPIO_MODE_IT_FALLING;
	/* プルアップ プロダウン設定 */
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	gpio_setup(TADR_PSW_BASE, &GPIO_Init_Data, PSW_PIN);
}

/*
 * PUSHスイッチコールバック関数設定
 */
void setup_sw_func(intptr_t exinf)
{
	exti_func[PSW_PIN-EXTI_BASENO] = (void (*)(void))exinf;
}

/*
 * EXTI0割込みハンドラ
 */
void
exti0_handler(void)
{
	uint32_t istatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR));

	if((istatus & (1<<0)) != 0){
		if(exti0_func[0] != NULL)
			(exti0_func[0])();
	}
    sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), istatus);
}

/*
 * EXTI15-10割込みハンドラ
 */
void
exti15_handler(void)
{
	/* EXTI line interrupt detected */
	uint32_t istatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR));
	uint32_t i;
	for(i = 0 ; i < NUM_EXTI15_FUNC ; i++){
		if((istatus & (1<<(i+EXTI15_BASENO))) != 0){
			if(exti15_func[i] != NULL)
				(exti15_func[i])();
		}
	}
    sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), istatus);
}


/*
 * DIPSWの取出し
 */
uint_t
switch_dip_sense(void)
{
	return dipsw_value;
}

