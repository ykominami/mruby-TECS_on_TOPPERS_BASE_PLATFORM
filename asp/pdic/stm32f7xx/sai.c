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
 *  @(#) $Id: sai.c 698 2016-03-26 19:04:52Z roi $
 */
/*
 * 
 *  AUDIOドライバ関数群
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include <string.h>
#include "device.h"
#include "sai.h"

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  AUDIOポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_AUDIO(id)         ((uint_t)((id) - 1))

#define SAI_TIMEOUT_VALUE       (10*1000)	/* 10ms */
#define PLLI2S_TIMEOUT_VALUE    100			/* Timeout value fixed to 100 ms  */
#define EXTERNAL_CLOCK_VALUE    12288000	/* Value of the Internal oscillator in Hz*/

#define _STDINT_H

#include "audio.h"
#include "wm8994.h"

/*
 *  AUDIOポート設定テーブル
 */
static const AUDIO_PortControlBlock audio_pcb[NUM_AUDIOPORT] = {
  {	TADR_SAI1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOEEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOGEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOJEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_SAI1EN,


	TADR_GPIOE_BASE, TADR_GPIOG_BASE, TADR_GPIOE_BASE, TADR_GPIOJ_BASE,
#ifdef TOPPERS_STM32F769_DISCOVERY
	TADR_DMA2_STM3_BASE, DMA_CHANNEL_0, TADR_DMA2_STM4_BASE, DMA_CHANNEL_1,
#else
	TADR_DMA2_STM1_BASE, DMA_CHANNEL_0, TADR_DMA2_STM4_BASE, DMA_CHANNEL_1,
#endif
    7, 4, 5, 6, 3, 12, 0x06, 0x06},

  {	TADR_SAI2_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOIEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOGEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOHEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_SAI2EN,


	TADR_GPIOI_BASE, TADR_GPIOI_BASE, TADR_GPIOG_BASE, TADR_GPIOH_BASE,
	TADR_DMA2_STM4_BASE, DMA_CHANNEL_3, TADR_DMA2_STM7_BASE, DMA_CHANNEL_0,
    4, 7, 5, 6, 10, 15, 0x0A, 0x0A}
};

static AUDIO_Handle_t  haudio_sai[NUM_AUDIOPORT];

/*
 *  SAI1割込みハンドラ
 */
static void
audio_exti1_int_handler(void)
{
	audio_irqhandler(&haudio_sai[0]);
}

/*
 *  SAI2割込みハンドラ
 */
static void
audio_exti2_int_handler(void)
{
	audio_irqhandler(&haudio_sai[1]);
}

/*
 *  SAIベースブロック取出し関数
 */
static uint32_t
audio_get_blockbase(AUDIO_Handle_t *haudio, uint32_t mode)
{
	if(haudio == NULL || mode > 1)
		return 0;
	return haudio->base + mode * SAI_WINDOW_SIZE + 4;
}

/*
 *  SAI BLOCKハードウェア起動
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
void
audio_enable(AUDIO_Handle_t *haudio, uint32_t mode)
{
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return;
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_SAIEN);
}


/*
 *  SAI BLOCKハードウェア停止
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
ER
audio_disable(AUDIO_Handle_t *haudio, uint32_t mode)
{
	uint32_t tick = 0;
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;
	sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_SAIEN);
	while((sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_CR1)) & SAI_xCR1_SAIEN) != 0){
		if(tick > SAI_TIMEOUT_VALUE){	/* タイムアウト監視 */
			haudio->ErrorCode |= AUDIO_ERROR_TIMEOUT;
			return E_TMOUT;
		}
		sil_dly_nse(1000);
		tick++;
	}
	return E_OK;
}

/*
 *  AUDIOドライバ初期化関数
 *  parammter1  id: ポートID
 *  return      AUDIOハンドラ
 */
AUDIO_Handle_t *audio_init(ID id)
{
	static DMA_Handle_t hdma_sai_tx;
	static DMA_Handle_t hdma_sai_rx;
	GPIO_Init_t  gpio_init_structure;
	const AUDIO_PortControlBlock *apcb;
	AUDIO_Handle_t *haudio;
	DMA_Handle_t *hdma;
	volatile uint32_t tmpreg;
	uint32_t no;

	if(id > NUM_AUDIOPORT)
		return NULL;
	no = INDEX_AUDIO(id);
	apcb = &audio_pcb[no];
	haudio = &haudio_sai[no];

	memset(haudio, 0, sizeof(AUDIO_Handle_t));
	haudio->base = apcb->base;
	haudio->pcb  = apcb;
	/*
	 *  SAIクロックイネーブル
	 */
	sil_orw_mem((uint32_t *)apcb->audioclockbase, apcb->audioclockbit);
    tmpreg = sil_rew_mem((uint32_t *)apcb->audioclockbase);

	/*
	 *  OUT GPIOクロックイネーブル
	 */
	sil_orw_mem((uint32_t *)apcb->outgioclockbase, apcb->outgioclockbit);
	tmpreg = sil_rew_mem((uint32_t *)apcb->outgioclockbase);

	/*
	 *  SAI FS, SCK, MCK,SDピン初期化
	 */
	gpio_init_structure.mode      = GPIO_MODE_AF;
	gpio_init_structure.pull      = GPIO_NOPULL;
	gpio_init_structure.otype     = GPIO_OTYPE_PP;
	gpio_init_structure.speed     = GPIO_SPEED_HIGH;
	gpio_init_structure.alternate = apcb->outaf;
	gpio_setup(apcb->giofsobase, &gpio_init_structure, apcb->outfspin);

	gpio_init_structure.alternate = apcb->outaf;
	gpio_setup(apcb->giofsobase, &gpio_init_structure, apcb->outsckin);

	gpio_init_structure.alternate =  apcb->outaf;
	gpio_setup(apcb->giofsobase, &gpio_init_structure, apcb->outsdpin);

	gpio_init_structure.alternate =  apcb->outaf;
	gpio_setup(apcb->giomcobase, &gpio_init_structure, apcb->outmsckpin);

	/*
	 *  OUT-DMAイネーブル
	 */
	sil_orw_mem((uint32_t *)apcb->outdmaclockbase, apcb->outdmaclockbit);
	tmpreg = sil_rew_mem((uint32_t *)apcb->outdmaclockbase);

	hdma = &hdma_sai_tx;
	hdma->Init.Channel             = apcb->outdmachannel;
	hdma->Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma->Init.MemInc              = DMA_MINC_ENABLE;
	hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
	hdma->Init.Mode                = DMA_CIRCULAR;
	hdma->Init.Priority            = DMA_PRIORITY_HIGH;
	hdma->Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
	hdma->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma->Init.MemBurst            = DMA_MBURST_SINGLE;
	hdma->Init.PeriphBurst         = DMA_PBURST_SINGLE;
	hdma->base = apcb->outdmabase;

	/*
	 *  OUT-DMA設定
	 */
	haudio->hdmatx  = hdma;
	hdma->localdata = haudio;
	dma_deinit(hdma);
	dma_init(hdma);

	/*
	 *  SD GPIOクロック設定
	 */
	sil_orw_mem((uint32_t *)apcb->ingioclockbase, apcb->ingioclockbit);
	tmpreg = sil_rew_mem((uint32_t *)apcb->ingioclockbase);

	/*
	 *  SD GPIO設定
	 */
	gpio_init_structure.mode      = GPIO_MODE_AF;
	gpio_init_structure.pull      = GPIO_NOPULL;
	gpio_init_structure.otype     = GPIO_OTYPE_PP;
	gpio_init_structure.speed     = GPIO_SPEED_FAST;
	gpio_init_structure.alternate = apcb->inaf;
	gpio_setup(apcb->giosdibase, &gpio_init_structure, apcb->insdpin);

	/*
	 *  AUDIO-割込みGPIOクロック設定
	 */
	sil_orw_mem((uint32_t *)apcb->intgioclockbase, apcb->intgioclockbit);
	tmpreg = sil_rew_mem((uint32_t *)apcb->intgioclockbase);

	/*
	 *  AUDIO-割込みピンGPIO設定
	 */
	gpio_init_structure.mode      = GPIO_MODE_INPUT;
	gpio_init_structure.pull      = GPIO_NOPULL;
	gpio_init_structure.speed     = GPIO_SPEED_FAST;
	gpio_setup(apcb->giointbase, &gpio_init_structure, apcb->inintpin);

	/*
	 *  IN-DMAクロック設定
	 */
	sil_orw_mem((uint32_t *)apcb->indmaclockbase, apcb->indmaclockbit);
	tmpreg = sil_rew_mem((uint32_t *)apcb->indmaclockbase);
	((void)(tmpreg));

	hdma = &hdma_sai_rx;
    hdma->Init.Channel             = apcb->indmachannel;
    hdma->Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma->Init.MemInc              = DMA_MINC_ENABLE;
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma->Init.Mode                = DMA_CIRCULAR;
    hdma->Init.Priority            = DMA_PRIORITY_HIGH;
    hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma->Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma->Init.PeriphBurst         = DMA_MBURST_SINGLE;
    hdma->base = apcb->indmabase;

#ifndef TOPPERS_STM32F769_DISCOVERY
	/*
	 *  IN-DMA設定
	 */
    haudio->hdmarx  = hdma;
    hdma->localdata = haudio;
    dma_deinit(hdma);
    dma_init(hdma);
#endif

	/*
	 * SAI割込み設定
	 */
	if(no == 0)
		exti15_func[apcb->inintpin-EXTI15_BASENO] = audio_exti1_int_handler;
	else
		exti15_func[apcb->inintpin-EXTI15_BASENO] = audio_exti2_int_handler;
	return haudio;
}

/*
 *  AUDIOドライバ終了設定関数
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 */
void
audio_deinit(AUDIO_Handle_t *haudio, uint32_t mode)
{
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase != 0 && haudio->pcb != NULL){
		if(mode == AUDIO_OUT_BLOCK && haudio->hdmatx != NULL){
			dma_deinit(haudio->hdmatx);	/* OUT-DMA停止 */
		}
		if(mode == AUDIO_IN_BLOCK && haudio->hdmarx != NULL){
			/* Deinitialize the DMA stream */
			dma_deinit(haudio->hdmarx);	/* IN-DMA停止 */
		}

		/*
		 *  SAIハード、クロック停止
		 */
		audio_disable(haudio, mode);
		sil_andw_mem((uint32_t *)haudio->pcb->audioclockbase, haudio->pcb->audioclockbit);
	}
}


/*
 *  AUDIO割込みフラグ取出し
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  pini:初期化構造体
 *  return      割込みフラグ
 */
static uint32_t
audio_interruptflag(AUDIO_Handle_t *haudio, SAI_Init_t *pini)
{
	if(pini->AudioMode == SAI_MODESLAVE_RX || pini->AudioMode == SAI_MODESLAVE_TX)
		return SAI_xIMR_OVRUDRIE | SAI_xIMR_AFSDETIE | SAI_xIMR_LFSDETIE;
	else
		return SAI_xIMR_OVRUDRIE | SAI_xIMR_WCKCFGIE;
}

/*
 *  SAIの入力クロック取得
 *  parameter1  haudio:AUDIOハンドラ
 *  return      SAI入力クロック
 */
static uint32_t
audio_get_inputclock(AUDIO_Handle_t *haudio)
{
	uint32_t tmp = 0;
	uint32_t cfgr = 0;
	/* This variable used to store the SAI clock frequency (value in Hz) */
	uint32_t frequency = 0;
	/* This variable used to store the VCO Input (value in Hz) */
	uint32_t vcoinput = 0;
	/* This variable used to store the SAI clock source */
	uint32_t configreg   = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLCFGR));
	uint32_t clocksource = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1));

    switch ((clocksource & (RCC_DCKCFGR1_SAI1SEL | RCC_DCKCFGR1_SAI2SEL))){
    case 0: /* PLLSAI is the clock source for SAI*/ 
		cfgr = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLSAICFGR));
        /* Configure the PLLSAI division factor */
        /* PLLSAI_VCO Input  = PLL_SOURCE/PLLM */ 
        if((configreg & RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI){
          /* In Case the PLL Source is HSI (Internal Clock) */
          vcoinput = (HSI_VALUE / (uint32_t)(configreg & RCC_PLLCFGR_PLLM));
        }
        else{
          /* In Case the PLL Source is HSE (External Clock) */
          vcoinput = ((HSE_VALUE / (uint32_t)(configreg & RCC_PLLCFGR_PLLM)));
        }
        /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN */
        /* SAI_CLK(first level) = PLLSAI_VCO Output/PLLSAIQ */
        tmp = (cfgr & RCC_PLLSAICFGR_PLLSAIQ) >> 24;
        frequency = (vcoinput * ((cfgr & RCC_PLLSAICFGR_PLLSAIN) >> 6))/(tmp);
        
        /* SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ */
        frequency = frequency/(((clocksource & RCC_DCKCFGR1_PLLSAIDIVQ) >> 8) + 1);
        break;
    case RCC_DCKCFGR1_SAI1SEL_0: /* PLLI2S is the clock source for SAI*/
    case RCC_DCKCFGR1_SAI2SEL_0: /* PLLI2S is the clock source for SAI*/
		cfgr = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLI2SCFGR));
        /* Configure the PLLI2S division factor */
        /* PLLI2S_VCO Input  = PLL_SOURCE/PLLM */ 
        if((configreg & RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI){
          /* In Case the PLL Source is HSI (Internal Clock) */
          vcoinput = (HSI_VALUE / (uint32_t)(configreg & RCC_PLLCFGR_PLLM));
        }
        else{
          /* In Case the PLL Source is HSE (External Clock) */
          vcoinput = ((HSE_VALUE / (uint32_t)(configreg & RCC_PLLCFGR_PLLM)));
        }

        /* PLLI2S_VCO Output = PLLI2S_VCO Input * PLLI2SN */
        /* SAI_CLK(first level) = PLLI2S_VCO Output/PLLI2SQ */
        tmp = (cfgr & RCC_PLLI2SCFGR_PLLI2SQ) >> 24;
        frequency = (vcoinput * ((cfgr & RCC_PLLI2SCFGR_PLLI2SN) >> 6))/tmp;

        /* SAI_CLK_x = SAI_CLK(first level)/PLLI2SDIVQ */
        frequency = frequency/((clocksource & RCC_DCKCFGR1_PLLI2SDIVQ) + 1);
        break;
    case RCC_DCKCFGR1_SAI1SEL_1: /* External clock is the clock source for SAI*/
    case RCC_DCKCFGR1_SAI2SEL_1: /* External clock is the clock source for SAI*/
        frequency = EXTERNAL_CLOCK_VALUE;
        break;
    default :
        break;
    }
	return frequency;
}

/*
 *  SAIクロックコンフィグレーション
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  AudioFreq: Audio周波数
 *  parameter3  Params: 補助パラメータポインタ
 */
void
audio_clockconfig(AUDIO_Handle_t *haudio, uint32_t AudioFreq, void *Params)
{
	uint32_t PLLI2SN, PLLI2SQ, PLLI2SDivQ;
	uint32_t tick = 0;

	/*
	 *  AUDIO周波数からPLL設定値の取得
	 */
	if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K)){
		/* Configure PLLI2S prescalers */
		/* PLLI2S_VCO: VCO_429M
		I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 429/2 = 214.5 Mhz
		I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 214.5/19 = 11.289 Mhz */
		PLLI2SN = 429;
		PLLI2SQ = 2;
		PLLI2SDivQ = 19;
	}
	else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */{
		/* I2S clock config
		PLLI2S_VCO: VCO_344M
		I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz
		I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */
		PLLI2SN = 344;
		PLLI2SQ = 7;
		PLLI2SDivQ = 1;
	}

	/*
	 *  SAI1/2クロック設定
	 */
	if(haudio->base == TADR_SAI1_BASE){
		sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_SAI1SEL, RCC_DCKCFGR1_SAI1SEL_0);
	}
	else{
		sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_SAI2SEL, RCC_DCKCFGR1_SAI2SEL_0);
	}
  
	/*
	 *  PLLI2Sデゼーブル
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLI2SON);

    /*
	 *  PLLI2S停止待ち
	 */
	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & RCC_CR_PLLI2SRDY) != 0){
		if(tick > PLLI2S_TIMEOUT_VALUE){
			return;		/* タイムアウトエラー */
		}
		dly_tsk(1);
		tick++;
	}

    /*
	 *  PLLI2SSN/PLLI2SN値設定
	 */
	sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLI2SCFGR), (RCC_PLLI2SCFGR_PLLI2SN | RCC_PLLI2SCFGR_PLLI2SQ), ((PLLI2SN << 6) | (PLLI2SQ << 24)));

	/*
	 *  PLLI2SDIVQ設定
	 */
	sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_PLLI2SDIVQ, (PLLI2SDivQ-1));

	/*
	 *  PLLI2Sイネーブル
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLI2SON);

    /*
	 *  PLLI2S再開待ち
	 */
	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & RCC_CR_PLLI2SRDY) == 0){
		if(tick > PLLI2S_TIMEOUT_VALUE){
			return;		/* タイムアウトエラー */
		}
		dly_tsk(1);
		tick++;
	}
}

/*
 *  AUDIO動作開始関数
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
ER
audio_start(AUDIO_Handle_t *haudio, uint32_t mode)
{
	ER ercd = E_OK;
	SAI_Init_t *pini;
	uint32_t gcr = 0;
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;

	if(mode == AUDIO_OUT_BLOCK)
		pini = &haudio->OutInit;
	else
		pini = &haudio->InInit;

	/*
	 *  SAIハードウェア停止
	 */
	ercd = audio_disable(haudio, mode);
	if(ercd != E_OK)
		return ercd;

	/*
	 *  ジェネラルコンフィグレーションレジスタ設定
	 */
	switch(pini->SynchroExt){
	case SAI_SYNCEXT_DISABLE:
		gcr = 0;
		break;
	case SAI_SYNCEXT_IN_ENABLE:
		gcr = SAI_GCR_SYNCIN_0;
		break;
	case SAI_SYNCEXT_OUTBLOCKA_ENABLE:
		gcr = SAI_GCR_SYNCOUT_0;
		break;
	case SAI_SYNCEXT_OUTBLOCKB_ENABLE:
		gcr = SAI_GCR_SYNCOUT_1;
		break;
	default :
		break;
	}
	sil_wrw_mem((uint32_t *)(haudio->base+TOFF_SAI_GCR), gcr);

	/*
	 *  マスタクロックデバイダ再設定
     *  MCKDIV[3:0] = SAI_CK_x / FS * 512
	 */
	if(pini->AudioFrequency != 0){
		uint32_t clock;

		clock = (((audio_get_inputclock(haudio) * 10) / ((pini->AudioFrequency) * 512)));
		pini->Mckdiv = clock / 10;
		if((clock % 10) > 8){	/* 9を切り上げ */
			pini->Mckdiv++;
		}
	}

	/*
	 *  SAI CR1設定
	 */
	sil_modw_mem((uint32_t *)(bbase+TOFF_SAI_CR1),
			(SAI_xCR1_MODE | SAI_xCR1_PRTCFG |  SAI_xCR1_DS | SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR |
			 SAI_xCR1_SYNCEN | SAI_xCR1_MONO | SAI_xCR1_OUTDRIV | SAI_xCR1_DMAEN | SAI_xCR1_NODIV | SAI_xCR1_MCKDIV),
			(pini->AudioMode | pini->Protocol | pini->DataSize | pini->FirstBit |
			 pini->ClockStrobing | pini->Synchro | pini->MonoStereoMode | pini->OutputDrive |
			 pini->NoDivider | (pini->Mckdiv << 20) | pini->CompandingMode));

	/*
	 *  SAI CR2設定
	 */
	sil_modw_mem((uint32_t *)(bbase+TOFF_SAI_CR2),
			(SAI_xCR2_FTH | SAI_xCR2_TRIS | SAI_xCR2_COMP),
			(pini->FIFOThreshold | pini->CompandingMode | pini->TriState));

	/*
	 *  SAI フレーム設定
	 */
	sil_modw_mem((uint32_t *)(bbase+TOFF_SAI_FRCR),
			(SAI_xFRCR_FRL | SAI_xFRCR_FSALL | SAI_xFRCR_FSDEF | SAI_xFRCR_FSPO | SAI_xFRCR_FSOFF),
			((pini->FrameLength - 1) | pini->FSOffset | pini->FSDefinition | 
			pini->FSPolarity | ((pini->ActiveFrameLength - 1) << 8)));

	/*
	 *  SAIブロックスロット設定
	 */
	sil_modw_mem((uint32_t *)(bbase+TOFF_SAI_SLOTR),
			(SAI_xSLOTR_FBOFF | SAI_xSLOTR_SLOTSZ | SAI_xSLOTR_NBSLOT | SAI_xSLOTR_SLOTEN),
			(pini->FirstBitOffset | pini->SlotSize | ((pini->SlotNumber - 1) <<  8)) | pini->SlotActive);

	/*
	 *  エラー設定初期化
	 */
	haudio->ErrorCode = AUDIO_ERROR_NONE;

	/*
	 *  ステータス設定
	 */
	haudio->status[mode] = AUDIO_STATUS_READY;
	return ercd;
}


/*
 *  AUDIO動作停止関数
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
ER
audio_end(AUDIO_Handle_t *haudio, uint32_t mode)
{
	ER ercd = E_OK;
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;

	/*
	 *  割込みとフラグクリア、SAI停止
	 */
	sil_wrw_mem((uint32_t *)(bbase+TOFF_SAI_IMR), 0);
	sil_wrw_mem((uint32_t *)(bbase+TOFF_SAI_CLRFR), 0xFFFFFFFF);
	ercd = audio_disable(haudio, mode);

	/*
	 *  FIFOをフラッシュ
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR2), SAI_xCR2_FFLUSH);

	/*
	 *  エラー設定初期化
	 */
	haudio->ErrorCode = AUDIO_ERROR_NONE;

	/*
	 *  ステータスをリセットに戻す
	 */
	haudio->status[mode] = AUDIO_STATUS_RESET;
	return ercd;
}

/*
 *  AUDIO 送信DMA完了コールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmatx_tran(DMA_Handle_t *hdma)
{
	uint32_t tick = 0;
	uint32_t bbase;

	AUDIO_Handle_t* haudio = (AUDIO_Handle_t*)((DMA_Handle_t* )hdma)->localdata;

	bbase = audio_get_blockbase(haudio, AUDIO_OUT_BLOCK);
	if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CIRC) == 0){
		/*
		 *  送信DMAを停止
		 */
		sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);

		/*
		 *  FIFOが空になるまで待ち
		 */
		while((sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_SR)) & SAI_xSR_FLVL) != 0){
			/* Check for the Timeout */
			if(tick  > SAI_TIMEOUT_VALUE){
				/*
				 *  タイムアウトエラー設定
				 */
				haudio->ErrorCode |= AUDIO_ERROR_TIMEOUT;
				if(haudio->errorcallback != NULL)
					haudio->errorcallback(haudio);
			}
			tick++;
			sil_dly_nse(1000);
		}

		/*
		 *  エラー割込み停止
		 */
		sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_IMR), audio_interruptflag(haudio, &haudio->OutInit));
		haudio->status[AUDIO_OUT_BLOCK] = AUDIO_STATUS_READY;
		haudio->XferCount = 0;
	}
	if(haudio->transcallback != NULL)
		haudio->transcallback(haudio);
}

/*
 *  AUDIO 送信DMAハーフコールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmatx_halftran(DMA_Handle_t *hdma)
{
	AUDIO_Handle_t* haudio = (AUDIO_Handle_t*)((DMA_Handle_t*)hdma)->localdata;

	if(haudio->transhalfcallback != NULL)
		haudio->transhalfcallback(haudio);
}

/*
 *  AUDIO 送信DMAエラーコールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmatx_error(DMA_Handle_t *hdma)
{
	AUDIO_Handle_t* haudio = ( AUDIO_Handle_t*)((DMA_Handle_t* )hdma)->localdata;

	/*
	 *  送信DMA停止
	 */
	audio_dmastop(haudio, AUDIO_OUT_BLOCK);

	/*
	 *  ステータスをレディに戻す
	 */
	haudio->status[AUDIO_OUT_BLOCK] = AUDIO_STATUS_READY;
	if(haudio->errorcallback != NULL)
		haudio->errorcallback(haudio);
	haudio->XferCount = 0;
}

/*
 *  AUDIO 受信DMA完了コールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmarx_recv(DMA_Handle_t *hdma)
{
	AUDIO_Handle_t* haudio = ( AUDIO_Handle_t* )((DMA_Handle_t* )hdma)->localdata;
	uint32_t bbase;

	bbase = audio_get_blockbase(haudio, AUDIO_IN_BLOCK);
	if((sil_rew_mem((uint32_t *)(hdma->base+TOFF_DMAS_CR)) & DMA_SxCR_CIRC) == 0){
		/*
		 *  受信DMA停止
		 */
		sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);

		/*
		 *  エラー割込み停止
		 */
		sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_IMR), audio_interruptflag(haudio, &haudio->InInit));

		haudio->status[AUDIO_IN_BLOCK] = AUDIO_STATUS_READY;
		haudio->XferCount = 0;
	}
	if(haudio->recevcallback != NULL)
		haudio->recevcallback(haudio);
}

/*
 *  AUDIO 受信DMAハーフコールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmarx_recvhalf(DMA_Handle_t *hdma)
{
	AUDIO_Handle_t* haudio = (AUDIO_Handle_t*)((DMA_Handle_t*)hdma)->localdata;

	if(haudio->recevhalfcallback != NULL)
		haudio->recevhalfcallback(haudio);
}

/*
 *  AUDIO 受信DMAエラーコールバック関数
 *  parameter1  hdma: DMAハンドラ
 */
static void
audio_dmarx_error(DMA_Handle_t *hdma)
{
	AUDIO_Handle_t* haudio = ( AUDIO_Handle_t* )((DMA_Handle_t* )hdma)->localdata;

	/*
	 *  受信DMA停止
	 */
	audio_dmastop(haudio, AUDIO_IN_BLOCK);

	/*
	 *  ステータスをレディに戻す
	 */
	haudio->status[AUDIO_IN_BLOCK]= AUDIO_STATUS_READY;
	if(haudio->errorcallback != NULL)
		haudio->errorcallback(haudio);
	haudio->XferCount = 0;
}


/*
 *  AUDIOデータ送信
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  pData:送信データ領域
 *  parameter3  Size:送信データサイズ
 *  return      ERコード
 */
ER
audio_transmit(AUDIO_Handle_t *haudio, uint8_t *pData, uint16_t Size)
{
	uint32_t bbase = audio_get_blockbase(haudio, AUDIO_OUT_BLOCK);

	if(pData == NULL || Size == 0 || bbase == 0)
		return E_PAR;

	if(haudio->status[AUDIO_OUT_BLOCK] != AUDIO_STATUS_READY)
		return E_OBJ;

	haudio->pBuffPtr = pData;
	haudio->XferSize = Size;
	haudio->XferCount = Size;
	haudio->status[AUDIO_OUT_BLOCK] = AUDIO_STATUS_BUSY_TX;

	/*
	 *  DMAコールバック関数設定
	 */
	haudio->hdmatx->xferhalfcallback = audio_dmatx_halftran;
	haudio->hdmatx->xfercallback = audio_dmatx_tran;
	haudio->hdmatx->errorcallback = audio_dmatx_error;

	/*
	 *  送信DMA起動
	 */
	dma_start(haudio->hdmatx, (uint32_t)pData, bbase+TOFF_SAI_DR, haudio->XferSize);

	/*
	 *  SAI起動
	 */
	if((sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_CR1)) & SAI_xCR1_SAIEN) == 0){
		sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_SAIEN);
	}

	/*
	 *  エラー割込み設定
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_IMR), audio_interruptflag(haudio, &haudio->OutInit));

	/*
	 *  SAI送信DMA要求
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);
	return E_OK;
}

/*
 *  AUDIOデータ受信
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  pData:受信データ領域
 *  parameter3  Size:受信データサイズ
 *  return      ERコード
 */
ER
audio_receive(AUDIO_Handle_t *haudio, uint8_t *pData, uint16_t Size)
{
	uint32_t bbase = audio_get_blockbase(haudio, AUDIO_IN_BLOCK);

	if(pData == NULL || Size == 0 || bbase == 0)
		return E_PAR;

	if(haudio->status[AUDIO_IN_BLOCK] != AUDIO_STATUS_READY)
		return E_OBJ;

	haudio->pBuffPtr = pData;
	haudio->XferSize = Size;
	haudio->XferCount = Size;

	haudio->status[AUDIO_IN_BLOCK] = AUDIO_STATUS_BUSY_RX;

	/*
	 *  DMAコールバック関数設定
	 */
	haudio->hdmarx->xferhalfcallback = audio_dmarx_recvhalf;
	haudio->hdmarx->xfercallback = audio_dmarx_recv;
	haudio->hdmarx->errorcallback = audio_dmarx_error;

	/*
	 *  受信DMA起動
	 */
	dma_start(haudio->hdmarx, bbase+TOFF_SAI_DR, (uint32_t)pData, haudio->XferSize);

	/*
	 *  SAI起動
	 */
	if((sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_CR1)) & SAI_xCR1_SAIEN) == 0){
		sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_SAIEN);
	}

	/*
	 *  エラー割込み設定
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_IMR), audio_interruptflag(haudio, &haudio->InInit));

	/*
	 *  SAI受信DMA要求
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);
	return E_OK;
}

/*
 *  AUDIOポーズ
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
  */
ER
audio_dmapause(AUDIO_Handle_t *haudio, uint32_t mode)
{
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;
	sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);
	return E_OK; 
}

/*
 *  AUDIOレジューム
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
ER
audio_dmaresume(AUDIO_Handle_t *haudio, uint32_t mode)
{
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;

	/*
	 *  SAI-DMA要求
	 */
	sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);

	/*
	 *  SAI起動
	 */
	if ((sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_CR1)) & SAI_xCR1_SAIEN) == 0){
		sil_orw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_SAIEN);
	}
	return E_OK;
}

/*
 *  AUDIO用DMA停止
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      ERコード
 */
ER
audio_dmastop(AUDIO_Handle_t *haudio, uint32_t mode)
{
	ER ercd = E_OK;
	uint32_t bbase = audio_get_blockbase(haudio, mode);

	if(bbase == 0)
		return E_PAR;
	/*
	 *  SAI-DMA停止
	 */
	sil_andw_mem((uint32_t *)(bbase+TOFF_SAI_CR1), SAI_xCR1_DMAEN);

	/*
	 *  送信モードなら送信DMA停止
	 */
	if(haudio->hdmatx != NULL && mode == AUDIO_OUT_BLOCK){
		dma_end(haudio->hdmatx);
	}

	/*
	 *  受信モードなら受信DMA停止
	 */
	if(haudio->hdmarx != NULL && mode == AUDIO_IN_BLOCK){
		dma_end(haudio->hdmarx);
	}

	/*
	 *  SAI停止
	 */
	ercd = audio_disable(haudio, mode);

	haudio->status[mode] = AUDIO_STATUS_READY;
	return ercd;
}

/*
 *  AUDIOハンドラ取出し
 *  parameter1: id ポートID
 */
AUDIO_Handle_t *
audio_gethandle(ID id)
{
	return &haudio_sai[INDEX_AUDIO(id)];
}

/*
 *  AUDIOステータスの取出し
 *  parameter1  haudio:AUDIOハンドラ
 *  parameter2  mode:入出力モード
 *  return      AUDIOステータス
 */
uint32_t
audio_status(AUDIO_Handle_t *haudio, uint32_t mode)
{
	return haudio->status[mode];
}

/*
 *  AUDIO割込みハンドラ
 *  parameter1  haudio:AUDIOハンドラ
 */
void
audio_irqhandler(AUDIO_Handle_t *haudio)
{
	uint32_t bbase, mode;
    uint32_t sr, imr;

	for(mode = AUDIO_OUT_BLOCK ; mode <= AUDIO_IN_BLOCK ; mode++){
		bbase = audio_get_blockbase(haudio, mode);
		sr = sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_SR));
		imr = sil_rew_mem((uint32_t *)(bbase+TOFF_SAI_IMR));

		/*
		 *  割込みステータス確認
		 */
		if(sr != 0x00000000){
			/*
			 *  オーバーラン/アンダーランチェック
			 */
			if((sr & SAI_xSR_OVRUDR) != 0 && (imr & SAI_xIMR_OVRUDRIE) != 0){
				sil_wrw_mem((uint32_t *)(bbase+TOFF_SAI_CLRFR), SAI_xSR_OVRUDR);
				haudio->ErrorCode = AUDIO_ERROR_OVRUDR;
				if(haudio->audioerrorcallback != NULL)
					haudio->audioerrorcallback(haudio, mode);
			}

			/*
			 *  MUTE割込みチェック
			 */
			if((sr & SAI_xSR_MUTEDET) != 0 && (imr & SAI_xIMR_MUTEDETIE) != 0){
				sil_wrw_mem((uint32_t *)(bbase+TOFF_SAI_CLRFR), SAI_xSR_MUTEDET);
				if(haudio->audiomutecallback != NULL)
					haudio->audiomutecallback(haudio, mode);
			}

			/*
			 *  AFSDET割込みチェック
			 */
			if((sr & SAI_xSR_AFSDET) != 0 && (imr & SAI_xIMR_AFSDETIE) != 0){
				haudio->ErrorCode = AUDIO_ERROR_AFSDET;
				if(haudio->audioerrorcallback != NULL)
					haudio->audioerrorcallback(haudio, mode);
			}

			/*
			 *  LFSDET割込みチェック
			 */
			if((sr & SAI_xSR_LFSDET) != 0 && (imr & SAI_xIMR_LFSDETIE) != 0){
				haudio->ErrorCode = AUDIO_ERROR_LFSDET;
				if(haudio->audioerrorcallback != NULL)
					haudio->audioerrorcallback(haudio, mode);
			}

			/*
			 *  WCKCFG割込みチェック
			 */
			if((sr & SAI_xSR_WCKCFG) != 0 && (imr & SAI_xIMR_WCKCFGIE) != 0){
				haudio->ErrorCode = AUDIO_ERROR_WCKCFG;
				if(haudio->audioerrorcallback != NULL)
					haudio->audioerrorcallback(haudio, mode);
			}
		}
	}
}


