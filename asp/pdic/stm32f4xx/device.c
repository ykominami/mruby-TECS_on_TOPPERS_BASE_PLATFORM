/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: device.c 698 2016-01-18 13:01:16Z roi $
 */
/*
 * 
 * カップラーメンタイマ用デバイス操作関数群の定義
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))


uint_t dipsw_value;
void (*exti_func[NUM_EXTI_FUNC])(void) = { 0 };

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
#if !defined(TOPPERS_STM32F401_NUCLEO) && !defined(TOPPERS_STM32F446_NUCLEO64)
	TADR_GPIOF_BASE,			/* index 5 */
	TADR_GPIOG_BASE,			/* index 6 */
#else
	0,							/* index 5 */
	0,							/* index 6 */
#endif
	TADR_GPIOH_BASE,			/* index 7 */
#if !defined(TOPPERS_STM32F401_NUCLEO) && !defined(TOPPERS_STM32F446_NUCLEO64)
	TADR_GPIOI_BASE,			/* index 8 */
#endif
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
	sil_modw_mem((uint32_t *)(base+TOFF_GPIO_MODER), (GPIO_MODER_MODER2 << (pin * 2)), ((init->mode & GPIO_MODE) << (pin * 2)));

	/*  出力モード設定 */
	if(init->mode == GPIO_MODE_OUTPUT || init->mode == GPIO_MODE_AF){
		sil_modw_mem((uint32_t *)(base+TOFF_GPIO_OSPEEDR), (GPIO_OSPEEDER_OSPEEDR2 << (pin * 2)), (init->speed << (pin * 2)));
		sil_modw_mem((uint32_t *)(base+TOFF_GPIO_OTYPER), (GPIO_OTYPER_OT << pin), (init->otype << pin));
	}

	/*  プルアップ、プルダウン設定 */
	sil_modw_mem((uint32_t *)(base+TOFF_GPIO_PUPDR), (GPIO_PUPDR_PUPDR2 << (pin * 2)), (init->pull << (pin * 2)));

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
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_PAR), DstAddress);
		sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMAS_M0AR), SrcAddress);
	}
	else{	/* デバイスからメモリ */
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
			if(hdma->errorcallback != NULL)
				hdma->errorcallback(hdma);
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
			if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & (uint32_t)(DMA_SxCR_DBM)) != 0){
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
			if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & (uint32_t)(DMA_SxCR_DBM)) != 0){
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


/*
 * LED接続ポート
 * 
 * 拡張I/OボードのLED1-3はプログラマブル入出力ポート0に
 * にインバータを介して接続されている．
 */

#if defined(TOPPERS_STM32F4_DISCOVERY)
#define TADR_LED_BASE   TADR_GPIOD_BASE
#define LEDGIOPEN       RCC_AHB1ENR_GPIODEN
#define	PIN_START       PINPOSITION12
#define PIN_END         PINPOSITION15
#define OFF_LEDON       TOFF_GPIO_BSRRL
#define OFF_LEDOFF      TOFF_GPIO_BSRRH
#elif defined(TOPPERS_STM32F401_NUCLEO) || defined(TOPPERS_STM32F446_NUCLEO64)
#define TADR_LED_BASE   TADR_GPIOA_BASE
#define LEDGIOPEN       RCC_AHB1ENR_GPIOAEN
#if defined(PSHIELD)
#define	PIN_START       PINPOSITION6
#define PIN_END         PINPOSITION7
#else
#define	PIN_START       PINPOSITION5
#define PIN_END         PINPOSITION5
#endif
#define OFF_LEDON       TOFF_GPIO_BSRRL
#define OFF_LEDOFF      TOFF_GPIO_BSRRH
#elif defined(TOPPERS_STM32F446_NUCLEO144)
#define TADR_LED_BASE   TADR_GPIOB_BASE
#define LEDGIOPEN       RCC_AHB1ENR_GPIOBEN
#define	PIN_START       PINPOSITION0
#define PIN_END         PINPOSITION14
#define OFF_LEDON       TOFF_GPIO_BSRRL
#define OFF_LEDOFF      TOFF_GPIO_BSRRH
#elif defined(TOPPERS_STM32_E407)
#define TADR_LED_BASE   TADR_GPIOC_BASE
#define LEDGIOPEN       RCC_AHB1ENR_GPIOCEN
#define	PIN_START       PINPOSITION13
#define PIN_END         PINPOSITION13
#define OFF_LEDON       TOFF_GPIO_BSRRH
#define OFF_LEDOFF      TOFF_GPIO_BSRRL
#endif

#if defined(PIN_START)
/*
 *  LED接続ポート初期化
 */ 
void
led_init(intptr_t exinf)
{
	GPIO_Init_t GPIO_Init_Data;
	unsigned char pinpos;

	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), LEDGIOPEN);

	for(pinpos = PIN_START ; pinpos <= PIN_END ; pinpos++){
		if(((1<<pinpos) & LED_MASK) == 0)
			continue;
		/* モード設定 */
		GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;
		/* プルアップ プロダウン設定 */
		GPIO_Init_Data.pull      = GPIO_PULLUP;
		/* 出力モード設定 */
		GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
        /* スピード設定 */
		GPIO_Init_Data.speed     = GPIO_SPEED_FAST;
		gpio_setup(TADR_LED_BASE, &GPIO_Init_Data, pinpos);

		/* 消灯 */
		sil_wrh_mem((uint16_t *)(TADR_LED_BASE+OFF_LEDOFF), 1 << pinpos);
	}
}

/*
 *  LED接続ポート読み出し
 */
uint_t
led_in(void)
{
	return sil_rew_mem((uint32_t *)(TADR_LED_BASE+TOFF_GPIO_IDR)) & LED_MASK;
}

/*
 *  LED接続ポート書き込み
 */ 
void
led_out(unsigned short led_data)
{
	unsigned short reg1, reg2;

	/* 設定値はLED接続ビット以外は変更しない */	
	reg1 = ~led_data & LED_MASK;
	reg2 = led_data & LED_MASK;

	/* 書き込みデータを生成して書き込み */	
	sil_wrh_mem((uint16_t *)(TADR_LED_BASE+OFF_LEDOFF), reg1);
	sil_wrh_mem((uint16_t *)(TADR_LED_BASE+OFF_LEDON), reg2);
}

/*
 * LEDとスイッチの個別設定・読み込み関数群
 */

/*
 *  LEDの状態保存用変数
 */	
unsigned short LedState;


/*
 *  LED点灯制御
 */
void 
set_led_state(unsigned short led, unsigned char state){
	if (state == ON) {
		LedState = LedState | led;
	} else {
		LedState = LedState & ~led;
	}
	led_out(LedState);
}

#endif

#if defined(TOPPERS_STM32F4_DISCOVERY)
#define TADR_PSW_BASE   TADR_GPIOA_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOAEN
#define	PSW_PIN         PINPOSITION0
#elif defined(TOPPERS_STM32F401_NUCLEO) || defined(TOPPERS_STM32F446_NUCLEO64)
#if defined(PSHIELD)
#define TADR_PSW_BASE   TADR_GPIOB_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOBEN
#define	PSW_PIN         PINPOSITION4
#else
#define TADR_PSW_BASE   TADR_GPIOC_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOCEN
#define	PSW_PIN         PINPOSITION13
#endif
#elif defined(TOPPERS_STM32F446_NUCLEO144)
#define TADR_PSW_BASE   TADR_GPIOC_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOCEN
#define	PSW_PIN         PINPOSITION13
#elif defined(TOPPERS_STM32_E407)
#define TADR_PSW_BASE   TADR_GPIOA_BASE
#define PSWGIOPEN       RCC_AHB1ENR_GPIOAEN
#define	PSW_PIN         PINPOSITION0
#endif

#if defined(PSW_PIN)
/*
 * PUSHスイッチ接続ポート初期化
 */
void
switch_push_init(intptr_t exinf)
{
	GPIO_Init_t GPIO_Init_Data;

	/* クロック設定 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), PSWGIOPEN);
	/* 入力ポート、モード設定 */
	GPIO_Init_Data.mode      = GPIO_MODE_IT_FALLING;
	/* プルアップ プロダウン設定 */
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	gpio_setup(TADR_PSW_BASE, &GPIO_Init_Data, PSW_PIN);
}


/*
 * PUSHスイッチ割込みクリア
 */
void
switch_push_clear(void)
{
	sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), (1<<PSW_PIN));
}


/*
 * PUSHスイッチコールバック関数設定
 */
void
setup_sw_func(intptr_t exinf)
{
	exti_func[PSW_PIN-EXTI_BASENO] = (void (*)(void))exinf;
}


#ifndef USE_USERSW_INT

/*
 * INT0(SW1)割込みハンドラ
 */
void
sw_dev_int(void)
{
	uint32_t estatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR));
	uint32_t istatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR));
	uint32_t i;

	for(i = 0 ; i < NUM_EXTI_FUNC ; i++){
		if((istatus & (1<<(i+EXTI_BASENO))) != 0 && (estatus & (1<<(i+EXTI_BASENO))) != 0){
			if(exti_func[i] != NULL)
				(exti_func[i])();
		}
	}
    sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), istatus);
}
#endif
#endif


#if (defined(TOPPERS_STM32F401_NUCLEO) || defined(TOPPERS_STM32F446_NUCLEO64)) && defined(PSHIELD)
#define TADR_DPI_BASE   TADR_GPIOA_BASE
#define DIPGIOPEN       RCC_AHB1ENR_GPIOAEN
#define	DIP_PIN0        PINPOSITION8
#define	DIP_PIN1        PINPOSITION9
#else
#endif

/*
 *  DIPSWの初期化
 */
void
switch_dip_init(intptr_t exinf)
{
#if defined(TADR_DPI_BASE)
	GPIO_Init_t GPIO_Init_Data;

	/* クロック設定 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), DIPGIOPEN);
	/* 入力ポート、モード設定 */
	GPIO_Init_Data.mode      = GPIO_MODE_INPUT;
	/* プルアップ プロダウン設定 */
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	gpio_setup(TADR_PSW_BASE, &GPIO_Init_Data, DIP_PIN0);
	gpio_setup(TADR_PSW_BASE, &GPIO_Init_Data, DIP_PIN1);
#endif
	dipsw_value = 0;
}

/*
 *  DIPSWの取出し
 */
uint_t
switch_dip_sense(void)
{
#if defined(TADR_DPI_BASE)
	dipsw_value = (sil_rew_mem((uint32_t *)(TADR_DPI_BASE+TOFF_GPIO_IDR)) ^ 0xffff) >> DIP_PIN0;
#endif
	return dipsw_value;
}


