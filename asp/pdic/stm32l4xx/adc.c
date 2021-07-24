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
 *  @(#) $Id: adc.c 698 2017-07-11 21:03:58Z roi $
 */
/*
 *
 *  ADCドライバ関数群
 *
 */
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include <target_syssvc.h>
#include "device.h"
#include "adc.h"

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  ADCポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_ADC(adcid)    ((uint_t)((adcid) - 1))

#define INIT_CFGR           (ADC_CFGR_RES    | ADC_CFGR_ALIGN  | ADC_CFGR_CONT | \
							 ADC_CFGR_OVRMOD | ADC_CFGR_DISCEN | ADC_CFGR_DISCNUM | \
							 ADC_CFGR_EXTEN  | ADC_CFGR_EXTSEL)

#define INIT_CFGR_2         (ADC_CFGR_DMACFG | ADC_CFGR_AUTDLY)

#define INIT_CFGR2          (ADC_CFGR2_ROVSE | ADC_CFGR2_OVSR  | ADC_CFGR2_OVSS | \
							 ADC_CFGR2_TROVS | ADC_CFGR2_ROVSM)

#define INIT_IER            (ADC_IER_AWD3IE | ADC_IER_AWD2IE  | ADC_IER_AWD1IE | ADC_IER_JQOVFIE | \
							 ADC_IER_OVRIE  | ADC_IER_JEOSIE  | ADC_IER_JEOCIE | ADC_IER_EOSIE   | \
							 ADC_IER_EOCIE  | ADC_IER_EOSMPIE | ADC_IER_ADRDYIE)

#define INIT_ISR            (ADC_ISR_AWD3 | ADC_ISR_AWD2  | ADC_ISR_AWD1 | ADC_ISR_JQOVF | \
							 ADC_ISR_OVR  | ADC_ISR_JEOS  | ADC_ISR_JEOC | ADC_ISR_EOS   | \
							 ADC_ISR_EOC  | ADC_ISR_EOSMP | ADC_ISR_ADRDY)

#define INIT_OFR            (ADC_OFR1_OFFSET1 | ADC_OFR1_OFFSET1_CH | ADC_OFR1_OFFSET1_EN)

/*
 *  ADCタイムアウト時間
 */
#define ADC_ENABLE_TIMEOUT            (10*1000)	/* 10ms */
#define ADC_DISABLE_TIMEOUT           (10*1000)	/* 10ms */
#define ADC_STOP_CONVERSION_TIMEOUT   (10*1000)	/* 10ms */
#define ADC_CONVERSION_TIME_MAX_CPU_CYCLES 167168	/* ADC conversion completion time-out value */

/*
 *  ADC実行待ち時間(μsec)
 */
#define ADC_STAB_DELAY_US              10
/*
 *  ADC温度センサー実行待ち時間(μsec)
 */
#define ADC_TEMPSENSOR_DELAY_US        10




/*
 *  ADCコンバージョンモード定義
 */
#define ADC_MODE_INDEPENDENT               0x00000000							/* Independent ADC conversions mode */
#define ADC_DUALMODE_REGSIMULT_INJECSIMULT ADC_CCR_DUAL_0						/* Combined regular simultaneous + injected simultaneous mode */
#define ADC_DUALMODE_REGSIMULT_ALTERTRIG   ADC_CCR_DUAL_1						/* Combined regular simultaneous + alternate trigger mode     */
#define ADC_DUALMODE_REGINTERL_INJECSIMULT (ADC_CCR_DUAL_1 | ADC_CCR_DUAL_0)	/* Combined Interleaved mode + injected simultaneous mode     */
#define ADC_DUALMODE_INJECSIMULT           (ADC_CCR_DUAL_2 | ADC_CCR_DUAL_0)	/* Injected simultaneous mode only */
#define ADC_DUALMODE_REGSIMULT             (ADC_CCR_DUAL_2 | ADC_CCR_DUAL_1)	/* Regular simultaneous mode only */
#define ADC_DUALMODE_INTERL                (ADC_CCR_DUAL_2 | ADC_CCR_DUAL_1 | ADC_CCR_DUAL_0)	/* Interleaved mode only */
#define ADC_DUALMODE_ALTERTRIG             (ADC_CCR_DUAL_3 | ADC_CCR_DUAL_0)	/* Alternate trigger mode only */

/*
 *  ADCコンバージョングループ定義
 */
#define ADC_REGULAR_GROUP              (ADC_ISR_EOC | ADC_ISR_EOS)				/* ADC regular group selection */
#define ADC_INJECTED_GROUP             (ADC_ISR_JEOC | ADC_ISR_JEOS)			/* ADC injected group selection */
#define ADC_REGULAR_INJECTED_GROUP     (ADC_REGULAR_GROUP | ADC_INJECTED_GROUP)	/* ADC regular and injected groups selection */ 

/*
 *  ADCポート設定テーブル
 */
static const ADC_PortControlBlock adc_pcb[NUM_ADCPORT] = {
  {	TADR_ADC1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),  RCC_AHB2ENR_ADCEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA1EN,
	TADR_DMA1_CH1_BASE, DMA_REQUEST_0 },

  {	TADR_ADC1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),  RCC_AHB2ENR_ADCEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA1EN,
	TADR_DMA1_CH2_BASE, DMA_REQUEST_0 },

  {	TADR_ADC1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),  RCC_AHB2ENR_ADCEN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA1EN,
	TADR_DMA1_CH3_BASE, DMA_REQUEST_0 }
};

static ADC_Handle_t  AdcHandle[NUM_ADCPORT];

/*
 *  ADC DMA転送終了コールバック関数
 */
static void
adc_dmatrans_func(DMA_Handle_t *hdma)
{
	ADC_Handle_t* hadc = ( ADC_Handle_t* )((DMA_Handle_t* )hdma)->localdata;

	if(hadc->Init.ContinuousConvMode == ADC_CONTINUOUS_DISABLE)
		hadc->status = ADC_STATUS_EOC;
	else
		hadc->status = ADC_STATUS_BUSY_EOC;
	if(hadc->xfercallback != NULL)
		hadc->xfercallback(hadc);
}

/*
 *  ADC DMAハーフ転送コールバック関数
 */
static void
adc_dmahalftrans_func(DMA_Handle_t *hdma)
{
	ADC_Handle_t* hadc = ( ADC_Handle_t* )((DMA_Handle_t* )hdma)->localdata;

	if(hadc->xferhalfcallback != NULL)
		hadc->xferhalfcallback(hadc);
}

/*
 *  ADC DMAエラーコールバック関数
 */
static void
adc_dmaerror_func(DMA_Handle_t *hdma)
{
	ADC_Handle_t* hadc = ( ADC_Handle_t* )((DMA_Handle_t* )hdma)->localdata;

	hadc->status = ADC_STATUS_READY;
	hadc->errorcode |= ADC_ERROR_DMA;
	if(hadc->errorcallback != NULL)
		hadc->errorcallback(hadc);
}

/*
 *  ADCイネーブル判定
 */
static bool_t
is_adc_enable(uint32_t base)
{
	if((sil_rew_mem((uint32_t *)(base+TOFF_ADC_CR)) & (ADC_CR_ADEN | ADC_CR_ADDIS)) == ADC_CR_ADEN && 
			(sil_rew_mem((uint32_t *)(base+TOFF_ADC_ISR)) & ADC_ISR_ADRDY) != 0)
		return true;
	else
		return false;
}

/*
 *  ADC有効化
 */
static ER
adc_enable(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t tick = 0;

	if(!is_adc_enable(hadc->base)){
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADCAL | ADC_CR_JADSTP | ADC_CR_ADSTP | \
				ADC_CR_JADSTART | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN)) != 0){
			hadc->status = ADC_STATUS_ERROR;
			hadc->errorcode |= ADC_ERROR_INT;
			ercd = E_SYS;
		}
		else{
			/*
			 *  ADC ENABLE
			 */
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADEN);
			sil_dly_nse(ADC_STAB_DELAY_US * 1000);
			while((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR)) & ADC_ISR_ADRDY) == 0){
				if(tick > ADC_ENABLE_TIMEOUT){
					hadc->status |= ADC_STATUS_ERROR;
					hadc->errorcode = ADC_ERROR_INT;
					ercd = E_TMOUT;
					break;
				}
				tick++;
				sil_dly_nse(1000);
			}
		}
	}
	return ercd;
}

/*
 *  ADC無効化
 */
static ER
adc_disable(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t tick = 0;

	if(is_adc_enable(hadc->base)){
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_JADSTART | ADC_CR_ADSTART | ADC_CR_ADEN)) == ADC_CR_ADEN){
			/*
			 *  ADC DISABLE
			 */
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADDIS);
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_EOSMP | ADC_ISR_ADRDY));
		}
		else{
			hadc->status = ADC_STATUS_ERROR;
			hadc->errorcode |= ADC_ERROR_INT;
			return E_SYS;
		}
		while((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADEN) != 0){
			if(tick > ADC_DISABLE_TIMEOUT){
				hadc->errorcode = ADC_ERROR_INT;
				ercd = E_TMOUT;
			}
			tick++;
			sil_dly_nse(1000);
		}
	}
	return ercd;
}

/*
 * ADC DUAL REGULARコンバージョンモード
 */
static bool_t
adc_dual_regular_conversion(ADC_Handle_t* hadc)
{
	uint32_t adc_dual = sil_rew_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR)) & ADC_CCR_DUAL;
	if(hadc->base != TADR_ADC2_BASE)
		return true;
	if(adc_dual == ADC_MODE_INDEPENDENT || adc_dual == ADC_DUALMODE_INJECSIMULT || adc_dual == ADC_DUALMODE_ALTERTRIG)
		return true;
	else
		return false;
}

/*
 *  ADCコンバージョン停止
 */
static ER
adc_conversionstop(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t ConversionGroup = ADC_REGULAR_INJECTED_GROUP;
	uint32_t tick = 0;
	uint32_t check_cr;

	/*
	 *  ADCコンバージョンスタート状態ならば停止待ちを行う
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADSTART | ADC_CR_JADSTART)) != 0){
		/*
		 *  AUTO-INJECTIONモードは特別な停止待ち
		 */
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR)) & ADC_CFGR_JAUTO) != 0 &&
			hadc->Init.ContinuousConvMode == ADC_CONTINUOUS_ENABLE &&
				hadc->Init.LowPowerAutoWait == ADC_LOWAUTOWAIT_ENABLE){
			/*
			 *  レギュラーコンバージョングループに変更
			 */
			ConversionGroup = ADC_REGULAR_GROUP;

			/*
			 *  JEOS停止待ち
			 */
			while((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR)) & ADC_ISR_JEOS) == 0){
				if(tick >= (ADC_CONVERSION_TIME_MAX_CPU_CYCLES *4)){
					hadc->status = ADC_STATUS_ERROR;
					hadc->errorcode |= ADC_ERROR_INT;
					return E_SYS;
				}
				tick++;
			}

			/*
			 * JEOSクリア
			 */
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_JEOS);
		}

		/*
		 *  レギュラーコンバージョングループ停止
		 */
		if(ConversionGroup != ADC_INJECTED_GROUP){
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADSTART | ADC_CR_ADDIS)) == ADC_CR_ADSTART)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADSTP);
		}

		/*
		 *  INJJECTEDコンバージョングループ停止
		 */
		if(ConversionGroup != ADC_REGULAR_GROUP){
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_JADSTART | ADC_CR_ADDIS)) == ADC_CR_JADSTART)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_JADSTP);
		}

		switch(ConversionGroup){
		case ADC_REGULAR_INJECTED_GROUP:
			check_cr = (ADC_CR_ADSTART | ADC_CR_JADSTART);
			break;
		case ADC_INJECTED_GROUP:
			check_cr = ADC_CR_JADSTART;
			break;
		default:
			check_cr = ADC_CR_ADSTART;
			break;
		}

		/*
		 *  停止待ち
		 */
		tick = 0;
		while((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & check_cr) != 0){
			if(tick > ADC_STOP_CONVERSION_TIMEOUT){
				hadc->status = ADC_STATUS_ERROR;
				hadc->errorcode |= ADC_ERROR_INT;
				ercd = E_TMOUT;
				break;
			}
			tick++;
			sil_dly_nse(1000);
		}
	}
	return ercd;
}

/*
 *  ADC初期設定
 *  parameter1  port: ADCポート番号
 *  parameter2  pini: ADC初期設定構造体へのポインタ
 *  return ADCハンドラへのポインタ、NULLでエラー
 */
ADC_Handle_t *
adc_init(ID portid, ADC_Init_t* pini)
{
	static DMA_Handle_t  hdma_adc;
	ADC_Handle_t *hadc;
	const ADC_PortControlBlock *apcb;
	uint32_t     act_adc = 0, tmp, i;

	/*
	 *  設定パラメータチェック
	 */
	if(portid < 1 || portid > NUM_ADCPORT || pini == NULL)
		return NULL;

	if(pini->ScanConvMode != ADC_SCANMODE_DISABLE){
		if(pini->NumConversion < 1 || pini->NumConversion > 16)
			return NULL;
		if(pini->DiscontinuousConvMode == ADC_DISCONTINUOUS_ENABLE){
			if(pini->NumDiscConversion < 1 || pini->NumDiscConversion > 8)
				return NULL;
		}
	}
	if(pini->DiscontinuousConvMode == ADC_DISCONTINUOUS_ENABLE && pini->ContinuousConvMode == ADC_CONTINUOUS_ENABLE)
		return NULL;

	hadc = &AdcHandle[INDEX_ADC(portid)];
	memcpy(&hadc->Init, pini, sizeof(ADC_Init_t));

	apcb = &adc_pcb[INDEX_ADC(portid)];
	hadc->base = apcb->base;
	hadc->apcb = apcb;

	for(i = 0 ; i < NUM_ADCPORT ; i++){
		if(AdcHandle[i].status != ADC_STATUS_RESET)
			act_adc |= 1<<i;
	}

	/*
	 *  ADCクロック設定
	 */
	sil_orw_mem((uint32_t *)(apcb->adcclockbase), apcb->adcclockbit);
	sil_rew_mem((uint32_t *)(apcb->adcclockbase));

	/*
	 *  DMA対応があれば、DMA設定を行う
	 */
	if(apcb->dmaclockbase != 0 && pini->DMAContinuousRequests != ADC_DMACONTINUOUS_DISABLE){
		sil_orw_mem((uint32_t *)(apcb->dmaclockbase), apcb->dmaclockbit);
		sil_rew_mem((uint32_t *)(apcb->dmaclockbase));

		hdma_adc.cbase                    = apcb->dmarxchannel;
		hdma_adc.Init.Request             = apcb->dmarxrequest;
		hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_adc.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
		hdma_adc.Init.Mode                = DMA_CIRCULAR;
		hdma_adc.Init.Priority            = DMA_PRIORITY_MEDIUM;

		dma_init(&hdma_adc);

		hadc->hdmarx = &hdma_adc;
		hdma_adc.localdata = hadc;
	}
	else{
		hadc->hdmarx = NULL;
	}

	/*
	 *  ADC DEEP POWER設定解除
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_DEEPPWD) != 0)
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_DEEPPWD);

	/*
	 *  ADC VOLTAGE 設定
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADVREGEN) == 0){
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADVREGEN);
		sil_dly_nse(ADC_STAB_DELAY_US * 1000);
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADVREGEN) == 0)
			return NULL;
	}

	/*
	 *  ADCモードチェック
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return NULL;

	/*
	 *  ADCのプリスケーラ設定(コモン設定)
	 */
	if(act_adc == 0)
		sil_modw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), (ADC_CCR_PRESC|ADC_CCR_CKMODE), hadc->Init.ClockPrescaler);

	/*
	 *  ADC初期設定
	 */
	tmp = pini->ContinuousConvMode | pini->Overrun | pini->DataAlign | pini->Resolution | pini->DiscontinuousConvMode;
	if(pini->DiscontinuousConvMode == ADC_DISCONTINUOUS_ENABLE)
		tmp |= pini->NumDiscConversion << 17;
	if(pini->ExternalTrigConv != ADC_SOFTWARE_START && pini->ExternalTrigConvEdge != ADC_EXTERNALTRIGCONVEDGE_NONE)
		tmp |= ( pini->ExternalTrigConv | pini->ExternalTrigConvEdge);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), INIT_CFGR, tmp);

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADSTART | ADC_CR_JADSTART)) == 0){
		tmp = pini->LowPowerAutoWait | pini->DMAContinuousRequests;
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), INIT_CFGR_2, tmp);

		/*
		 *  ADCオーバーサンプリング設定
		 */
		if(pini->OversamplingMode == ADC_OVR_SAMPLING_ENABLE){
			if(pini->ExternalTrigConv == ADC_SOFTWARE_START || pini->ExternalTrigConvEdge == ADC_EXTERNALTRIGCONVEDGE_NONE){
				/*
				 *  外部トリガ設定エラー
				 */
				return NULL;
			}
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR2), INIT_CFGR2, (ADC_CFGR2_ROVSE | 
							pini->OversamplingRatio | pini->OversamplingRightBitShift | 
							pini->OversamplingTriggeredMode | pini->OversamplingStopReset));
		}
		else	/* オーバーサンプリング設定しない */
			sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR2), ADC_CFGR2_ROVSE);
	}

	if(pini->ScanConvMode == ADC_SCANMODE_ENABLE)
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), ADC_SQR1_L, (pini->NumConversion - 1));
	else
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), ADC_SQR1_L);

	/*
	 *  ADCを初期状態に
	 */
	hadc->errorcode = ADC_ERROR_NONE;
	hadc->status = ADC_STATUS_READY;
	return hadc;
}

/*
 *  ADC終了設定
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_deinit(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t  act_adc_cnt = 0, i;

	if(hadc == NULL)
		return E_PAR;

	ercd = adc_conversionstop(hadc);
	if(ercd == E_OK){
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), ADC_CFGR_JQM);
		/*
		 *  ADC無効化
		 */
		ercd = adc_disable(hadc);
	}

	if(ercd == E_OK){
		/*
		 *  リセットIER/ISR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), INIT_IER);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), INIT_ISR);

		/*
		 *  リセットCR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), (ADC_CR_ADVREGEN | ADC_CR_ADCALDIF));
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_DEEPPWD);
		/*
		 *  リセットCFGR/CGFR2
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), (INIT_CFGR | INIT_CFGR_2 | ADC_CFGR_AWD1CH | \
						ADC_CFGR_JAUTO | ADC_CFGR_JAWD1EN | ADC_CFGR_AWD1EN  | ADC_CFGR_AWD1SGL | \
						ADC_CFGR_JQM   | ADC_CFGR_JDISCEN | ADC_CFGR_DISCNUM | ADC_CFGR_DMAEN));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR2), (INIT_CFGR2 | ADC_CFGR2_JOVSE));

		/*
		 *  リセットSMPR
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR1), 0);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR2), 0);

		/*
		 *  リセットTR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR1), (ADC_TR1_LT1 | ADC_TR1_HT1));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR2), (ADC_TR2_LT2 | ADC_TR2_HT2));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR3), (ADC_TR3_LT3 | ADC_TR3_HT3));

		/*
		 *  リセットSQR
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), 0);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR2), 0);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR3), 0);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR4), 0);

		/*
		 *  リセットOFR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR1), (ADC_OFR1_OFFSET1_EN | ADC_OFR1_OFFSET1_CH | ADC_OFR1_OFFSET1));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR2), (ADC_OFR2_OFFSET2_EN | ADC_OFR2_OFFSET2_CH | ADC_OFR2_OFFSET2));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR3), (ADC_OFR3_OFFSET3_EN | ADC_OFR3_OFFSET3_CH | ADC_OFR3_OFFSET3));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR4), (ADC_OFR4_OFFSET4_EN | ADC_OFR4_OFFSET4_CH | ADC_OFR4_OFFSET4));

		/*
		 *  リセットAWD
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD2CR), ADC_AWD2CR_AWD2CH);
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD3CR), ADC_AWD3CR_AWD3CH);

		/*
		 *  リセットDIFSEL
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_DIFSEL), ADC_DIFSEL_DIFSEL);

		/*
		 *  リセットCALFACT
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CALFACT), (ADC_CALFACT_CALFACT_D | ADC_CALFACT_CALFACT_S));

		/*
		 *  リセットCOMMON ADC
		 */
		for(i = 0 ; i < NUM_ADCPORT ; i++){
			if(AdcHandle[i].status != ADC_STATUS_RESET)
				act_adc_cnt++;
		}
		if(act_adc_cnt <= 1){
			/*
			 *  リセットCCR
			 */
			sil_andw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), (ADC_CCR_CKMODE | ADC_CCR_PRESC | ADC_CCR_VBATEN | \
						ADC_CCR_TSEN | ADC_CCR_VREFEN | ADC_CCR_MDMA | ADC_CCR_DMACFG | ADC_CCR_DELAY | ADC_CCR_DUAL));
			/*
			 *  ADCクロック停止
			 */
			sil_andw_mem((uint32_t *)(hadc->apcb->adcclockbase), hadc->apcb->adcclockbit);
		}
	}

	/*
	 *  DMA停止
	 */
	if(hadc->hdmarx != NULL)
		dma_deinit(hadc->hdmarx);

	/*
	 *  ADCの状態初期化
	 */
	hadc->errorcode = ADC_ERROR_NONE;
	hadc->status = ADC_STATUS_RESET;
	return ercd;
}

/*
 *  ADC-割込みモード開始処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_start_int(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;

	if(hadc == NULL)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return E_OBJ;

    /*
	 *  ADC有効化
	 */
	ercd = adc_enable(hadc);

	if(ercd == E_OK){
		/*
		 *  実行状態に移行
		 */
		hadc->status = ADC_STATUS_BUSY;
		hadc->errorcode = ADC_ERROR_NONE;

		/*
		 *  割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR));

		/*
		 *  ADC割込み設定
		 */
		if(hadc->Init.EOCSelection == ADC_EOC_SEQ_CONV)
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_EOSIE);
		else
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_EOCIE);
		if(hadc->Init.Overrun == ADC_OVR_DATA_PRESERVED)
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_OVRIE);

		/*
		 *  ADCスタート
		 */
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR)) & ADC_CFGR_JAUTO) != 0)
			hadc->status = ADC_STATUS_BUSY_INJ;
		if(adc_dual_regular_conversion(hadc))
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADSTART);
	}
	return ercd;
}

/*
 *  ADC-割込みモード停止処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_end_int(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;

	if(hadc == NULL)
		return E_PAR;

	/*
	 *  ADCコンバージョンスタート状態ならば停止待ちを行う
	 */
	ercd = adc_conversionstop(hadc);

	/*
	 *  ADC停止処理
	 */
	if(ercd == E_OK){
		/*
		 *  転送、エラー割込み停止
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), (ADC_IER_EOCIE | ADC_IER_EOSIE | ADC_IER_OVRIE));

		/*
		 *  ADC無効化
		 */
		ercd = adc_disable(hadc);
	}
	if(ercd == E_OK)
		hadc->status = ADC_STATUS_READY;
	return ercd;
}

/*
 *  ADC-DMAモード開始処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  parameter2  pdata: 受信領域のポインタ
 *  parameter3  length: 受信データ長
 *  return ERコード
 */
ER
adc_start_dma(ADC_Handle_t* hadc, uint32_t* pdata, uint32_t length)
{
	ER ercd = E_OK;

	if(hadc == NULL)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return E_OBJ;

	if(!adc_dual_regular_conversion(hadc))
		return E_OBJ;

    /*
	 *  ADC有効化
	 */
	ercd = adc_enable(hadc);

    /*
	 *  ADC-DMAスタート処理
	 */
   	if(ercd == E_OK){
		/*
		 *  DMAコールバック関数設定
		 */
		hadc->hdmarx->xfercallback = adc_dmatrans_func;
		hadc->hdmarx->xferhalfcallback = adc_dmahalftrans_func;
		hadc->hdmarx->errorcallback = adc_dmaerror_func;

		/*
		 *  割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR));

		/*
		 *  ADCオーバーラン割込み設定
		 */
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_OVRIE);

		/*
		 *  ADC DMA設定
		 */
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), ADC_CFGR_DMAEN);

		/*
		 *  実行状態に移行
		 */
		hadc->status = ADC_STATUS_BUSY;
		hadc->errorcode = ADC_ERROR_NONE;

		/*
		 *  DMAスタート
		 */
		dma_start(hadc->hdmarx, hadc->base+TOFF_ADC_DR, (uint32_t)pdata, length);

		/*
		 *  ADCスタート
		 */
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADSTART);
	}
	else
		ercd = E_OBJ;
	return ercd;
}

/*
 *  ADC-DMAモード停止処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_end_dma(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;

	if(hadc == NULL)
		return E_PAR;

	/*
	 *  ADCコンバージョンスタート状態ならば停止待ちを行う
	 */
	ercd = adc_conversionstop(hadc);

	/*
	 *  ADC停止処理
	 */
	if(ercd == E_OK){
		/*
		 *  ADCのDMAモードをオフ
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), ADC_CFGR_DMAEN);

		/*
		 *  DMAを停止
		 */
		if(hadc->hdmarx != NULL)
			ercd = dma_end(hadc->hdmarx);
		if(ercd != E_OK)
			hadc->status |= ADC_STATUS_ERROR;

		/*
		 *  オーバーラン割込みを停止
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_OVRIE);

		/*
		 *  ADC無効化
		 */
		ercd = adc_disable(hadc);
	}
	if(ercd == E_OK)
		hadc->status = ADC_STATUS_READY;
	return ercd;
}

/*
 *  ADC 割込みハンドラ実行関数
 */
void
adc_handler(ADC_Handle_t* hadc)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR));
	uint32_t ier = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_IER));
	uint32_t cfg = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR));

	if(((isr & ier) & ADC_ISR_EOSMP) != 0){
		if(hadc->eosmpcallback != NULL)
			hadc->eosmpcallback(hadc);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_EOSMP);
	}

	/*
	 *  エンドコンバージョンまたはエンドシーケンス判定
	 */
	if(((isr & ier) & (ADC_ISR_EOC | ADC_ISR_EOS)) != 0){
	    /*
		 *  コンバージョン終了状態に移行
		 */
	    if(hadc->errorcode == ADC_ERROR_NONE)
			hadc->status = ADC_STATUS_EOC;

	    /*
		 *  ADC停止処理
		 */
	    if((cfg & ADC_CFGR_EXTEN) == 0){
			uint32_t tmp;
			if(adc_dual_regular_conversion(hadc))
				tmp = cfg;
			else
				tmp = sil_rew_mem((uint32_t *)(TADR_ADC1_BASE+TOFF_ADC_CFGR));
			if((tmp & ADC_CFGR_CONT) == 0){
			    /*
				 *  END OF SEQUENCEケース
				 */
				if((isr & ADC_ISR_EOS) != 0){
					/*
					 *  EOSEQ割込み用終了処理：ADSTARTの状態を確認
					 */
					if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) == 0){
						/*
						 *  割込みマスク
						 */
						sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), (ADC_IER_EOCIE | ADC_IER_EOSIE));
						if(hadc->status == ADC_STATUS_BUSY_INJ)
							hadc->status = ADC_STATUS_READY;
						else
							hadc->status = ADC_STATUS_BUSY_EOC;
					}
					else{	/* インターナルエラー */
						hadc->status = ADC_STATUS_ERROR;
						hadc->errorcode |= ADC_ERROR_INT;
					}
				}
			}
		}

		/*
		 *  終了コールバック関数の読み出し
		 */
		if(hadc->xfercallback != NULL)
			hadc->xfercallback(hadc);

	    /*
		 *  割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_EOC | ADC_ISR_EOS));
	}

	/*
	 *  INJECTEDエンドコンバージョン判定
	 */
	if(((isr & ier) & (ADC_ISR_JEOC | ADC_ISR_JEOS)) != 0){
	    /*
		 *  コンバージョン終了状態に移行
		 */
	    if(hadc->errorcode == ADC_ERROR_NONE)
			hadc->status = ADC_STATUS_EOC_INJ;

		if((isr & ADC_ISR_JEOS) != 0){
			uint32_t tmp;
			if(adc_dual_regular_conversion(hadc))
				tmp = cfg;
			else
				tmp = sil_rew_mem((uint32_t *)(TADR_ADC1_BASE+TOFF_ADC_CFGR));
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_JSQR)) & ADC_JSQR_JEXTEN) == 0 &&
				(tmp & ADC_CFGR_JQM) == 0 && (tmp & (ADC_CFGR_JAUTO|ADC_CFGR_CONT)) != (ADC_CFGR_JAUTO|ADC_CFGR_CONT) &&
					(cfg & ADC_CFGR_EXTEN) == 0){

				if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_JADSTART) == 0){
					sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), (ADC_IER_JEOCIE | ADC_IER_JEOSIE));
					if(hadc->status == ADC_STATUS_BUSY_INJ)
						hadc->status = ADC_STATUS_READY;
					else
						hadc->status = ADC_STATUS_BUSY_EOC;
				}
				else{	/* インターナルエラー */
					hadc->status = ADC_STATUS_ERROR;
					hadc->errorcode |= ADC_ERROR_INT;
				}
			}
		}

		if(hadc->injectedcallback != NULL)
			hadc->injectedcallback(hadc);

	    /*
		 *  割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_JEOC | ADC_ISR_JEOS));
	}

	/*
	 *  アナログウォッチドック判定
	 */
	if((isr & ADC_ISR_AWD1) != 0 && (ier & ADC_IER_AWD1IE) != 0){
		/*
		 *  ウォッチドック発生状態に変更
		 */
		hadc->status = ADC_STATUS_AWD1;

		/*
		 *  ウォッチドック割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD1);

		/*
		 *  ウィンドウ領域アウトのコールバック関数読み出し
		 */
		if(hadc->outofwincallback != NULL)
			hadc->outofwincallback(hadc);
	}
	if((isr & ADC_ISR_AWD2) != 0 && (ier & ADC_IER_AWD2IE) != 0){
		/*
		 *  ウォッチドック発生状態に変更
		 */
		hadc->status = ADC_STATUS_AWD2;

		/*
		 *  ウォッチドック割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD2);

		/*
		 *  ウィンドウ領域アウトのコールバック関数読み出し
		 */
		if(hadc->outofwincallback != NULL)
			hadc->outofwincallback(hadc);
	}
	if((isr & ADC_ISR_AWD3) != 0 && (ier & ADC_IER_AWD3IE) != 0){
		/*
		 *  ウォッチドック発生状態に変更
		 */
		hadc->status = ADC_STATUS_AWD3;

		/*
		 *  ウォッチドック割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD3);

		/*
		 *  ウィンドウ領域アウトのコールバック関数読み出し
		 */
		if(hadc->outofwincallback != NULL)
			hadc->outofwincallback(hadc);
	}

	/*
	 *  オーバーランエラー判定
	 */
	if((isr & ADC_ISR_OVR) != 0 && (ier & ADC_IER_OVRIE) != 0){
		/*
		 *  オーバーラン割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_OVR);

	    /*
		 * オーバーラン設定：ADC_OVR_DATA_PRESERVED　またはDMA設定でエラー処理を行う
		 */
	    if(hadc->Init.Overrun == ADC_OVR_DATA_PRESERVED ||
			(sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR)) & ADC_CFGR_DMAEN) != 0){

			/*
			 *  オーバーラン発生状態に変更
			 */
			hadc->status = ADC_STATUS_READY;
			hadc->errorcode |= ADC_ERROR_OVR;

			/*
			 *  エラーコールバック関数を読み出し
			 */
			if(hadc->errorcallback != NULL)
				hadc->errorcallback(hadc);
	    }
	}
}

/*
 *  ADC12割込みハンドラ
 */
void
adc_int12_handler(void)
{
	if(AdcHandle[0].status != ADC_STATUS_RESET)
		adc_handler(&AdcHandle[0]);
	if(AdcHandle[1].status != ADC_STATUS_RESET)
		adc_handler(&AdcHandle[1]);
}

/*
 *  ADC3割込みハンドラ
 */
void
adc_int3_handler(void)
{
	if(AdcHandle[2].status != ADC_STATUS_RESET)
		adc_handler(&AdcHandle[2]);
}


/*
 *  レギュラーレジスタの取り出し
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return レギュラーレジスタ値
 */
uint32_t
adc_getvalue(ADC_Handle_t* hadc)
{
	return sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_DR));
}

/*
 *  ADCチャネル設定
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  parameter2  sConfig: チャネル設定構造体へのポインタ
 *  return ERコード
 */
ER
adc_setupchannel(ADC_Handle_t* hadc, ADC_ChannelConf_t* sConfig)
{
	GPIO_Init_t GPIO_InitStruct;
	uint32_t clkpin, shift, ccr, tmp;

	if(hadc == NULL || sConfig == NULL)
		return E_PAR;

	clkpin = (sConfig->GpioBase - TADR_GPIOA_BASE) / 0x400;
	if(clkpin > 7 || sConfig->GpioPin >= 16)
		return E_PAR;

	if(sConfig->Rank == 0 || sConfig->Rank > 16)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return E_OBJ;

	/*
	 *  チャネル有効設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB2ENR), (1<<clkpin));
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB2ENR));

	GPIO_InitStruct.mode = GPIO_MODE_ANALOG_AD;
	GPIO_InitStruct.pull = GPIO_NOPULL;
	gpio_setup(sConfig->GpioBase, &GPIO_InitStruct, sConfig->GpioPin);

	/*
	 *  ランク設定
	 */
	if (sConfig->Rank < 5){	/* ランク1から4 */
		shift = 6 * (sConfig->Rank - 1) + 6;
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), (ADC_SQR2_SQ5 << shift), (sConfig->Channel << shift));
	}
	else if (sConfig->Rank < 10){	/* ランク5から9 */
		shift = 6 * (sConfig->Rank - 5);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR2), (ADC_SQR2_SQ5 << shift), (sConfig->Channel << shift));
	}
	else if (sConfig->Rank < 15){	/* ランク10から14 */
		shift = 6 * (sConfig->Rank - 10);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR3), (ADC_SQR2_SQ5 << shift), (sConfig->Channel << shift));
	}
	else{	/* ランク15から16 */
		shift = 6 * (sConfig->Rank - 15);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR4), (ADC_SQR2_SQ5 << shift), (sConfig->Channel << shift));
	}

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADSTART | ADC_CR_JADSTART)) == 0){
		/*
		 *  サンプリング時間設定
		 */
		if(sConfig->Channel > ADC_CHANNEL_9){	/* チャネル10から18 */
			shift = 3 * (sConfig->Channel - 10);
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR2), (ADC_SMPR2_SMP10 << shift), (sConfig->SamplingTime << shift));
		}
		else{ 	/* チャネル0から9 */
			shift = 3 * sConfig->Channel;
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR1), (ADC_SMPR1_SMP0 << shift), (sConfig->SamplingTime << shift));
		}
		/*
		 *  オフセット設定
		 */
		tmp = sConfig->Offset << ((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR)) & ADC_CFGR_RES) * 2);
		switch (sConfig->OffsetNumber){
		case ADC_OFFSET_1:
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR1), INIT_OFR, (ADC_OFR1_OFFSET1_EN | (sConfig->Channel << 26) | tmp));
			break;
		case ADC_OFFSET_2:
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR2), INIT_OFR, (ADC_OFR2_OFFSET2_EN | (sConfig->Channel << 26) | tmp));
			break;
		case ADC_OFFSET_3:
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR3), INIT_OFR, (ADC_OFR3_OFFSET3_EN | (sConfig->Channel << 26) | tmp));
			break;
		case ADC_OFFSET_4:
			sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR4), INIT_OFR, (ADC_OFR4_OFFSET4_EN | (sConfig->Channel << 26) | tmp));
			break;
		default :	/* OFFSETなしの場合 */
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR1)) & ADC_OFR1_OFFSET1_CH) == (sConfig->Channel << 26))
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR1), ADC_OFR1_OFFSET1_EN);
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR2)) & ADC_OFR2_OFFSET2_CH) == (sConfig->Channel << 26))
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR2), ADC_OFR2_OFFSET2_EN);
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR3)) & ADC_OFR3_OFFSET3_CH) == (sConfig->Channel << 26))
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR3), ADC_OFR3_OFFSET3_EN);
			if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR4)) & ADC_OFR4_OFFSET4_CH) == (sConfig->Channel << 26))
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_OFR4), ADC_OFR4_OFFSET4_EN);
			break;
		}
	}

	/*
	 *  ADCが有効でない場合、Differential input modeと個別のセンサー設定を行う
	 */
	if(!is_adc_enable(hadc->base)){
		if(sConfig->SingleDiff != ADC_DIFFERENTIAL_ENDED)
			sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_DIFSEL), (1 << sConfig->Channel));
		else{
			/* Enable differential mode */
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_DIFSEL), (1 << sConfig->Channel));

			/*
			 *  サンプリングタイミングをチャネル＋１に変更
			 */
			if(sConfig->Channel >= ADC_CHANNEL_9){
				shift = 3 * (sConfig->Channel - 9);
				sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR2), (ADC_SMPR2_SMP10 << shift), (sConfig->SamplingTime << shift));
			}
			else{	/* For channels 0 to 8, SMPR1 must be configured */
				shift = 3 * (sConfig->Channel + 1);
				sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR1), (ADC_SMPR1_SMP0 << shift), (sConfig->SamplingTime << shift));
			}
		}

		/*
		 * Vbat/VrefInt/TempSensorチャネル設定
		 */
		ccr = sil_rew_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR));
		if((sConfig->Channel == ADC_CHANNEL_TEMPSENSOR && (ccr & ADC_CCR_TSEN) == 0) ||
			(sConfig->Channel == ADC_CHANNEL_VBAT && (ccr & ADC_CCR_VBATEN) == 0) ||
				(sConfig->Channel == ADC_CHANNEL_VREFINT && (ccr & ADC_CCR_VREFEN) == 0)){
			/*
			 *  有効なADCがある場合、状態エラーとする
			 */
			if(!is_adc_enable(TADR_ADC1_BASE) && !is_adc_enable(TADR_ADC2_BASE) && !is_adc_enable(TADR_ADC3_BASE)){
				if(hadc->base != TADR_ADC2_BASE){
					if(sConfig->Channel == ADC_CHANNEL_TEMPSENSOR)
						sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_TSEN);
					else if(sConfig->Channel == ADC_CHANNEL_VBAT)
						sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VBATEN);
					else if(sConfig->Channel == ADC_CHANNEL_VREFINT)
						sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VREFEN);
				}
			}
			else{
				return E_OBJ;
			}
		}
	}
	return E_OK;
}

/*
 *  ADCアナログウォッチドック設定
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  parameter2  AnalogWDGConfig: アナログウォッチドック設定構造体へのポインタ
 *  return ERコード
 */
ER
adc_setupwatchdog(ADC_Handle_t* hadc, ADC_AnalogWDGConf_t* AnalogWDGConfig)
{
	uint32_t cfgr, tmp1, tmp2;

	if(hadc == NULL || AnalogWDGConfig == NULL)
		return E_PAR;

	if(AnalogWDGConfig->WatchdogNumber == 0 || AnalogWDGConfig->WatchdogNumber > 3)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADSTART | ADC_CR_JADSTART)) != 0)
		return E_OBJ;

	cfgr = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR));
	if(AnalogWDGConfig->WatchdogNumber == 1){
		/*
		 *  AWD1モード設定
		 */
		cfgr &= ~(ADC_CFGR_AWD1SGL | ADC_CFGR_JAWD1EN | ADC_CFGR_AWD1EN | ADC_CFGR_AWD1CH);
		cfgr |= AnalogWDGConfig->WatchdogMode | (AnalogWDGConfig->Channel << 26);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR), cfgr);

		/*
		 *  AWD1 HIGH/LOWのスレッシュホールド設定
		 */
		tmp1 = AnalogWDGConfig->HighThreshold << (((cfgr & ADC_CFGR_RES) >> 3)*2);
		tmp2 = AnalogWDGConfig->LowThreshold << (((cfgr & ADC_CFGR_RES) >> 3)*2);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR1), (tmp1 << 16) | tmp2);

		/*
		 *  AWD1 割込みクリアと割込み設定
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD1);
		if(AnalogWDGConfig->ITMode == ADC_ANALOGWATCHDOG_ITMODE_ENABLE)
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD1IE);
		else
			sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD1IE);
	}
	else{
		/*
		 *  AWD2,3 スレッシュ解像度計算
		 */
		if((cfgr & ADC_CFGR_RES) != (ADC_CFGR_RES_1 | ADC_CFGR_RES_0)){
			tmp1 = AnalogWDGConfig->HighThreshold >> (4 - (((cfgr & ADC_CFGR_RES) >> 3)*2));
			tmp2 = AnalogWDGConfig->LowThreshold >> (4 - (((cfgr & ADC_CFGR_RES) >> 3)*2));
		}
		else{
			tmp1 = AnalogWDGConfig->HighThreshold << 2;
			tmp2 = AnalogWDGConfig->LowThreshold << 2;
		}
		if(AnalogWDGConfig->WatchdogNumber == 2){
			/*
			 *  AWD2モード設定
			 */
			if(AnalogWDGConfig->WatchdogMode != ADC_ANALOGWATCHDOG_NONE)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD2CR), (1 << AnalogWDGConfig->Channel));
			else
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD2CR), (1 << AnalogWDGConfig->Channel));

			/*
			 *  AWD2 HIGH/LOWのスレッシュホールド設定
			 */
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR2), (tmp1 << 16) | tmp2);

			/*
			 *  AWD2 割込みクリアと割込み設定
			 */
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD2);
			if(AnalogWDGConfig->ITMode == ADC_ANALOGWATCHDOG_ITMODE_ENABLE)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD2IE);
			else
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD2IE);
		}
		else{
			/*
			 *  AWD3モード設定
			 */
			if(AnalogWDGConfig->WatchdogMode != ADC_ANALOGWATCHDOG_NONE)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD3CR), (1 << AnalogWDGConfig->Channel));
			else
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_AWD3CR), (1 << AnalogWDGConfig->Channel));

			/*
			 *  AWD3 HIGH/LOWのスレッシュホールド設定
			 */
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR3), (tmp1 << 16) | tmp2);

			/*
			 *  AWD3 HIGH/LOWのスレッシュホールド設定
			 */
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD3);
			if(AnalogWDGConfig->ITMode == ADC_ANALOGWATCHDOG_ITMODE_ENABLE)
				sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD3IE);
			else
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD3IE);
		}
	}
	return E_OK;
}


