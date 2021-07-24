/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: device.c 698 2017-06-02 09:35:16Z roi $
 */
/*
 * STM32F0xx用デバイスドライバ
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
void (*exti0_func[NUM_EXTI0_FUNC])(void) = { 0 };
void (*exti2_func[NUM_EXTI2_FUNC])(void) = { 0 };
void (*exti4_func[NUM_EXTI4_FUNC])(void) = { 0 };


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
		sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_SYSCFGCOMPEN);
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
#define NUM_CHDMA       12

#define DMA_TRS_TIMEOUT 2000	/* 2秒 */

static const uint32_t dma_base[NUM_CHDMA] = {
	TADR_DMA1_CH1_BASE,
	TADR_DMA1_CH2_BASE,
	TADR_DMA1_CH3_BASE,
	TADR_DMA1_CH4_BASE,
	TADR_DMA1_CH5_BASE,
	TADR_DMA1_CH6_BASE,
	TADR_DMA1_CH7_BASE,
	TADR_DMA2_CH1_BASE,
	TADR_DMA2_CH2_BASE,
	TADR_DMA2_CH3_BASE,
	TADR_DMA2_CH4_BASE,
	TADR_DMA2_CH5_BASE
};

static DMA_Handle_t *pDmaHandler[NUM_CHDMA];

/*
 *  CHANNEL DMA初期化関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_init(DMA_Handle_t *hdma)
{
	uint32_t tmp = 0;
	uint32_t i, index;

	/* パラメータチェック */
	if(hdma == NULL)
		return E_PAR;

	for(i = 0 ; i < NUM_CHDMA ; i++){
		if(dma_base[i] == hdma->cbase)
			break;
	}
	if(i == NUM_CHDMA)
		return E_PAR;
	pDmaHandler[i] = hdma;
	hdma->base = hdma->cbase & ~0x3FF;
	if(i > 6)		/* DMA2確認 */
		i -= 7;
	hdma->chid = i;
	index = i * 4;

	/* DMAリクエスト設定 */
	sil_modw_mem((uint32_t *)(hdma->base+TOFF_DMA_CSELR), (0xF << index), (hdma->Init.Request << index));

	/* DMA-CCRレジスタ取得 */
	tmp = sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR));

	/* DMAモードビットをテンポラリィにクリア */
	tmp &= ((uint32_t)~(DMA_CCR_DIR  | DMA_CCR_MEM2MEM | DMA_CCR_PINC  | \
                        DMA_CCR_MINC | DMA_CCR_PSIZE   | DMA_CCR_MSIZE | \
                        DMA_CCR_CIRC | DMA_CCR_PL));

	/* DMAモードをテンポラリィに設定 */
	tmp |=   hdma->Init.Direction        |
          hdma->Init.PeriphInc           | hdma->Init.MemInc           |
          hdma->Init.PeriphDataAlignment | hdma->Init.MemDataAlignment |
          hdma->Init.Mode                | hdma->Init.Priority;

	/* DMAモード設定 */
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), tmp);

	/* エラー状態をクリア */
	hdma->ErrorCode = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  CHANNEL DMA終了関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 *  return ER値
 */
ER
dma_deinit(DMA_Handle_t *hdma)
{
	int i, index;

	/* パラメータチェック */
	if(hdma == NULL)
		return E_PAR;

	for(i = 0 ; i < NUM_CHDMA ; i++){
		if(dma_base[i] == hdma->cbase)
			break;
	}
	if(i == NUM_CHDMA)
		return E_PAR;
	pDmaHandler[i] = NULL;
	if(i > 6)		/* DMA2確認 */
		i -= 7;
	hdma->chid = i;
	index = i * 4;

	/* DMAイネーブル */
	sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_EN);
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), 0);
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CNDTR), 0);
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CPAR), 0);
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CMAR), 0);

	/* 各フラグをクリア */
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_ISR_GIF << index));
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CTCIF << index));
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CHTIF << index));
	sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CTEIF << index));

	/* DMAリクエストクリア */
	sil_andw_mem((uint32_t *)(hdma->base+TOFF_DMA_CSELR), (0xF << index));

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
	sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_EN);
	hdma->status = DMA_STATUS_BUSY;

	/* データ長設定 */
	sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CNDTR), DataLength);

	if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH){
		/* メモリからデバイス */
		sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CPAR), DstAddress);
		sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CMAR), SrcAddress);
	}
	else{	/* デバイスからメモリ */
		sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CPAR), SrcAddress);
		sil_wrw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CMAR), DstAddress);
	}

	/* 割込みイネーブル */
	sil_orw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), (DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE));

	/* DMA開始 */
	sil_orw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_EN);

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
	sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_EN);

	/* DMA停止待ち */
	while((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_EN) != 0){
	    /* Check for the Timeout */
		if(tick > DMA_TRS_TIMEOUT){
			/* タイムアウトエラー設定 */
			hdma->ErrorCode |= DMA_ERROR_TIMEOUT;
			return E_TMOUT;
		}
    	dly_tsk(1);
	}
	hdma->status = DMA_ERROR_NONE;
	return E_OK;
}

/*
 *  STREAM DMA割込み処理関数
 *  parameter1  hdma: DMAハンドラへのポインタ
 */
void
dma_inthandler(DMA_Handle_t *hdma)
{
	uint32_t index = hdma->chid * 4;

	/*
	 *  転送エラー処理
	 */
	if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMA_ISR)) & (DMA_ISR_TEIF << index)) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_TEIE) != 0){
			/* 転送エラー割込みクリア */
			sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_TEIE);
			/* 転送エラーフラグをクリア */
			sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CTEIF << index));
			/* 転送エラー状態を設定 */
			hdma->ErrorCode |= DMA_ERROR_TE;
			hdma->status = DMA_STATUS_READY_ERROR;
			/* エラーコールバック */
			if(hdma->errorcallback != NULL)
				hdma->errorcallback(hdma);
		}
	}

	/*
	 *  ハーフ転送終了処理
	 */
	if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMA_ISR)) & (DMA_ISR_HTIF << index)) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_HTIE) != 0){
			if((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_CIRC) == 0){
				/* ハーフ転送割込みをクリア */
				sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_HTIE);
			}
			/* ハーフ転送完了フラグをクリア */
			sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CHTIF << index));
			/* ハーフ転送ステータスに変更 */
			hdma->status = DMA_STATUS_READY_HMEM0;
			/* ハーフ転送終了コールバック */
			if(hdma->xferhalfcallback != NULL)
				hdma->xferhalfcallback(hdma);
		}
	}

	/*
	 *  転送終了処理
	 */
	if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMA_ISR)) & (DMA_ISR_TCIF << index)) != 0){
		if((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_TCIE) != 0){
			if((sil_rew_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR)) & DMA_CCR_CIRC) == 0){
				/* 転送割込みをクリア */
				sil_andw_mem((uint32_t *)(hdma->cbase+TOFF_DMACH_CCR), DMA_CCR_TCIE);
			}
			/* 転送完了フラグをクリア */
			sil_wrw_mem((uint32_t *)(hdma->base+TOFF_DMA_IFCR), (DMA_IFCR_CTCIF << index));

			/* ステータスを終了へ */
			hdma->ErrorCode = DMA_ERROR_NONE;
			hdma->status = DMA_STATUS_READY_MEM0;
			/* 転送終了コールバック */
			if(hdma->xfercallback != NULL)
				hdma->xfercallback(hdma);
		}
	}
}

/*
 *  STREAM DMA 割込みサービスルーチン
 */
void
channel_dma_isr(intptr_t exinf)
{
	uint32_t index = (uint32_t)exinf;

	if(index == IRQ_VECTOR_DMA1_CH1){
		if(pDmaHandler[0] != NULL)
			dma_inthandler(pDmaHandler[0]);
	}
	else if(index == IRQ_VECTOR_DMA12_CH_LOW){
		if(pDmaHandler[1] != NULL)
			dma_inthandler(pDmaHandler[1]);
		if(pDmaHandler[2] != NULL)
			dma_inthandler(pDmaHandler[2]);
		if(pDmaHandler[7] != NULL)
			dma_inthandler(pDmaHandler[7]);
		if(pDmaHandler[8] != NULL)
			dma_inthandler(pDmaHandler[8]);
	}
	else if(index == IRQ_VECTOR_DMA12_CH_HIGH){
		if(pDmaHandler[3] != NULL)
			dma_inthandler(pDmaHandler[3]);
		if(pDmaHandler[4] != NULL)
			dma_inthandler(pDmaHandler[4]);
		if(pDmaHandler[5] != NULL)
			dma_inthandler(pDmaHandler[5]);
		if(pDmaHandler[6] != NULL)
			dma_inthandler(pDmaHandler[6]);
		if(pDmaHandler[9] != NULL)
			dma_inthandler(pDmaHandler[9]);
		if(pDmaHandler[10] != NULL)
			dma_inthandler(pDmaHandler[10]);
		if(pDmaHandler[11] != NULL)
			dma_inthandler(pDmaHandler[11]);
	}
}


/*
 *  LED接続ポート
 *
 *  拡張I/OボードのLED1-3はプログラマブル入出力ポート0に
 *  にインバータを介して接続されている．
 */

#define TADR_LED_BASE   TADR_GPIOA_BASE
#define LEDGIOPEN       RCC_AHBENR_GPIOAEN
#define	PIN_START       PINPOSITION5
#define PIN_END         PINPOSITION5

/*
 *  LED接続ポート初期化
 */ 
void
led_init(intptr_t exinf)
{
	GPIO_Init_t GPIO_Init_Data;
	unsigned char pinpos;

	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHBENR), LEDGIOPEN);

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
		GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
		gpio_setup(TADR_LED_BASE, &GPIO_Init_Data, pinpos);

		/* 消灯 */
		sil_wrw_mem((uint32_t *)(TADR_LED_BASE+TOFF_GPIO_BSRR), (1 << pinpos)<<16);
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
led_out(unsigned int led_data)
{
	unsigned int reg1, reg2;

	/* 設定値はLED接続ビット以外は変更しない */	
	reg1 = ~led_data & LED_MASK;
	reg2 = led_data & LED_MASK;

	/* 書き込みデータを生成して書き込み */
	sil_wrw_mem((uint32_t *)(TADR_LED_BASE+TOFF_GPIO_BSRR), (reg1<<16) | reg2);
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


#define TADR_PSW_BASE   TADR_GPIOC_BASE
#define PSWGIOPEN       RCC_AHBENR_GPIOCEN
#define	PSW_PIN         PINPOSITION13

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
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHBENR), PSWGIOPEN);
	temp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHBENR));
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
	uint32_t i;
	for(i = 0 ; i < NUM_EXTI0_FUNC ; i++){
		if((istatus & (1<<(i+EXTI0_BASENO))) != 0){
			if(exti0_func[i] != NULL)
				(exti0_func[i])();
		}
	}
    sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), istatus);
}

/*
 * EXTI2割込みハンドラ
 */
void
exti2_handler(void)
{
	uint32_t istatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR));
	uint32_t i;
	for(i = 0 ; i < NUM_EXTI2_FUNC ; i++){
		if((istatus & (1<<(i+EXTI2_BASENO))) != 0){
			if(exti2_func[i] != NULL)
				(exti2_func[i])();
		}
	}
    sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), istatus);
}

/*
 * EXTI4-10割込みハンドラ
 */
void
exti4_handler(void)
{
	uint32_t istatus = sil_rew_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR));
	uint32_t i;
	for(i = 0 ; i < NUM_EXTI4_FUNC ; i++){
		if((istatus & (1<<(i+EXTI4_BASENO))) != 0){
			if(exti4_func[i] != NULL)
				(exti4_func[i])();
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

