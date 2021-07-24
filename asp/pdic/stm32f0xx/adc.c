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
#define INDEX_ADC(adcid)        ((uint_t)((adcid) - 1))

#define INIT_CFGR1              (ADC_CFGR1_ALIGN | ADC_CFGR1_SCANDIR | ADC_CFGR1_EXTSEL | \
								 ADC_CFGR1_EXTEN | ADC_CFGR1_CONT    | ADC_CFGR1_DMACFG | \
								 ADC_CFGR1_OVRMOD | ADC_CFGR1_WAIT   | ADC_CFGR1_AUTOFF | ADC_CFGR1_DISCEN)

#define INIT_IER                (ADC_IER_ADRDYIE | ADC_IER_EOSMPIE | ADC_IER_EOCIE | \
								 ADC_IER_EOSIE   | ADC_IER_OVRIE   | ADC_IER_AWD1IE )

#define INIT_ISR                (ADC_ISR_ADRDY | ADC_ISR_EOSMP | ADC_ISR_EOC | \
								 ADC_ISR_EOS   | ADC_ISR_OVR   | ADC_ISR_AWD1 )


/*
 *  ADCタイムアウト時間
 */
#define ADC_ENABLE_TIMEOUT            (10*1000)	/* 10ms */
#define ADC_DISABLE_TIMEOUT           (10*1000)	/* 10ms */
#define ADC_STOP_CONVERSION_TIMEOUT   (10*1000)	/* 10ms */

/*
 *  ADC実行待ち時間(μsec)
 */
#define ADC_STAB_DELAY_US              3
/*
 *  ADC温度センサー実行待ち時間(μsec)
 */
#define ADC_TEMPSENSOR_DELAY_US        10

/*
 *  ADCポート設定テーブル
 */
static const ADC_PortControlBlock adc_pcb[NUM_ADCPORT] = {
  {	TADR_ADC1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR),  RCC_APB2ENR_ADCEN,
	(TADR_RCC_BASE+TOFF_RCC_AHBENR),   RCC_AHBENR_DMA1EN,
	TADR_DMA1_CH1_BASE, DMA_REQUEST_1 }
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
 *  ADC有効化
 */
static ER
adc_enable(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t tick = 0;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADEN | ADC_CR_ADDIS)) != ADC_CR_ADEN ||
			 ((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR)) & ADC_ISR_ADRDY) == 0 && 
				(sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1)) & ADC_CFGR1_AUTOFF) == 0)){
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADCAL | ADC_CR_ADSTP | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN)) != 0){
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
					hadc->status = ADC_STATUS_ERROR;
					hadc->errorcode |= ADC_ERROR_INT;
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

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & (ADC_CR_ADEN | ADC_CR_ADDIS)) == ADC_CR_ADEN && 
			((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR)) & ADC_ISR_ADRDY) != 0
				|| (sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1)) & ADC_CFGR1_AUTOFF) != 0)){
		/*
		 *  ADC DISABLE
		 */
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADDIS);
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_ADRDY | ADC_ISR_EOSMP));
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
 *  ADCコンバージョン停止
 */
static ER
adc_conversionstop(ADC_Handle_t* hadc)
{
	ER ercd = E_OK;
	uint32_t tick = 0;

	/*
	 *  ADCコンバージョンスタート状態ならば停止待ちを行う
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0){
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADDIS) == 0)
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR), ADC_CR_ADSTP);
		while((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0){
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

	/*
	 *  設定パラメータチェック
	 */
	if(portid < 1 || portid > NUM_ADCPORT || pini == NULL)
		return NULL;

	if(pini->DiscontinuousConvMode != 0 && pini->ContinuousConvMode != 0)
		return NULL;

	hadc = &AdcHandle[INDEX_ADC(portid)];
	memcpy(&hadc->Init, pini, sizeof(ADC_Init_t));

	apcb = &adc_pcb[INDEX_ADC(portid)];
	hadc->base = apcb->base;

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
		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
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
	 *  ADCのプリスケーラ設定(コモン設定)
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR2), ADC_CFGR2_CKMODE, hadc->Init.ClockPrescaler);

	/*
	 *  ADC初期設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), ADC_CFGR1_RES, hadc->Init.Resolution);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), INIT_CFGR1, (hadc->Init.DataAlign |
							hadc->Init.ScanConvMode | hadc->Init.ContinuousConvMode |
							hadc->Init.DiscontinuousConvMode | hadc->Init.DMAContinuousRequests |
							hadc->Init.Overrun | hadc->Init.LowPowerAutoWait | hadc->Init.LowPowerAutoPowerOff));

	/*
	 *  ADC外部トリガ設定
	 */
	if(hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START){
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), (ADC_CFGR1_EXTSEL | ADC_CFGR1_EXTEN),
			(hadc->Init.ExternalTrigConv | hadc->Init.ExternalTrigConvEdge));
	}

	/*
	 *  サンプリング時間設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR), ADC_SMPR_SMP, hadc->Init.SamplingTime);

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

	if(hadc == NULL)
		return E_PAR;

	ercd = adc_conversionstop(hadc);
	if(ercd == E_OK){
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
		 *  リセットCFGR1/CGFR2
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), (INIT_CFGR1 | ADC_CFGR1_AWD1CH  | ADC_CFGR1_AWD1EN |
															ADC_CFGR1_RES | ADC_CFGR1_DMAEN));
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR2), ADC_CFGR2_CKMODE);

		/*
		 *  リセットSMPR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR), ADC_SMPR_SMP);

		/*
		 *  リセットTR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR), (ADC_TR1_LT1 | ADC_TR1_HT1));

		/*
		 *  リセットCHSELR
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CHSELR), ADC_CHSELR_CHSEL);
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
	if(hadc->Init.LowPowerAutoPowerOff != ADC_LOWAUTOPOWEROFF_ENABLE)
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
			sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_EOCIE);
		else
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_EOCIE);
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), (ADC_IER_EOSIE | ADC_IER_OVRIE));

		/*
		 *  ADCスタート
		 */
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
adc_start_dma(ADC_Handle_t* hadc, uint16_t* pdata, uint32_t length)
{
	ER ercd = E_OK;

	if(hadc == NULL)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return E_OBJ;

    /*
	 *  ADC有効化
	 */
	if(hadc->Init.LowPowerAutoPowerOff != ADC_LOWAUTOPOWEROFF_ENABLE)
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
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), ADC_CFGR1_DMAEN);

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
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), ADC_CFGR1_DMAEN);

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
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1));

	if(((isr & ier) & (ADC_ISR_EOC | ADC_ISR_EOS)) != 0){
	    /*
		 *  コンバージョン終了状態に移行
		 */
	    if(hadc->errorcode == ADC_ERROR_NONE)
			hadc->status = ADC_STATUS_EOC;

	    /*
		 *  ADC停止処理
		 */
	    if((cr1 & ADC_CFGR1_EXTEN) == 0 && hadc->Init.ContinuousConvMode == ADC_CONTINUOUS_DISABLE){
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
					hadc->status = ADC_STATUS_BUSY_EOC;
				}
				else{	/* インターナルエラー */
					hadc->status |= ADC_STATUS_ERROR;
					hadc->errorcode = ADC_ERROR_INT;
				 }
			}
		}

		/*
		 *  終了コールバック関数の読み出し
		 */
		if(hadc->xfercallback != NULL)
			hadc->xfercallback(hadc);

	    /*
		 *  ローパワー有効モードでなければ割込みクリア
		 */
	    if(hadc->Init.LowPowerAutoWait != ADC_LOWAUTOWAIT_ENABLE){
			sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), (ADC_ISR_EOC | ADC_ISR_EOS));
		}
	}

	/*
	 *  アナログウォッチドック判定
	 */
	if((isr & ADC_ISR_AWD1) != 0 && (cr1 & ADC_CFGR1_AWD1EN) != 0){
		/*
		 *  ウォッチドック発生状態に変更
		 */
		hadc->status = ADC_STATUS_AWD;

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
			(sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1)) & ADC_CFGR1_DMAEN) != 0){

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
 *  ADC割込みハンドラ
 */
void
adc_int_handler(void)
{
	uint32_t i;
	ADC_Handle_t* hadc = &AdcHandle[0];

	for(i = 0 ; i < NUM_ADCPORT ; i++, hadc++){
		if(hadc->status > ADC_STATUS_RESET)
			adc_handler(hadc);
	}
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
	uint32_t clkpin;

	if(hadc == NULL || sConfig == NULL)
		return E_PAR;

	clkpin = (sConfig->GpioBase - TADR_GPIOA_BASE) / 0x400;
	if(clkpin >= 7 || sConfig->GpioPin >= 16)
		return E_PAR;

	/*
	 *  チャネル有効設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHBENR), 1<<(clkpin+17));
	sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHBENR));

	GPIO_InitStruct.mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.pull = GPIO_NOPULL;
	gpio_setup(sConfig->GpioBase, &GPIO_InitStruct, sConfig->GpioPin);

	/*
	 *  ランク設定
	 */
	if(sConfig->Rank != ADC_RANK_NONE){
		/*
		 *  チャネル設定
		 */
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CHSELR), (sConfig->Channel & ADC_CHSELR_CHSEL));

		/*
		 *  温度センサー設定
		 */
	    if(sConfig->Channel == ADC_CHANNEL_TEMPSENSOR){
			sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_TSEN);
			sil_dly_nse(ADC_TEMPSENSOR_DELAY_US*1000);
		}

		/*
		 *  VREFINTチャネル設定
		 */
		if(sConfig->Channel == ADC_CHANNEL_VREFINT){
			sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VREFEN);
		}

		/*
		 *  VBATチャネル設定
		 */
		if(sConfig->Channel == ADC_CHANNEL_VBAT){
			sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VBATEN);
		}
	}
	else{
		/*
		 *  チャネル設定リセット
		 */
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CHSELR), (sConfig->Channel & ADC_CHSELR_CHSEL));

		/*
		 *  温度センサー設定解除
		 */
		if(sConfig->Channel == ADC_CHANNEL_TEMPSENSOR){
			sil_andw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_TSEN);
		}

		/*
		 *  VREFINTチャネル設定解除
		 */
		if(sConfig->Channel == ADC_CHANNEL_VREFINT){
			sil_andw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VREFEN);
		}

		/*
		 *  VBATチャネル設定解除
		 */
		if(sConfig->Channel == ADC_CHANNEL_VBAT){
			sil_andw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VBATEN);
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
	uint32_t shift;

	if(hadc == NULL || AnalogWDGConfig == NULL)
		return E_PAR;

	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR)) & ADC_CR_ADSTART) != 0)
		return E_OBJ;

	/*
	 *  割込み設定
	 */
	sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_ISR), ADC_ISR_AWD1);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_IER), ADC_IER_AWD1IE, AnalogWDGConfig->ITMode);

	/*
	 *  モード設定
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), (ADC_CFGR1_AWD1SGL | ADC_CFGR1_AWD1EN | ADC_CFGR1_AWD1CH));

	/* Set the analog watchdog enable mode */
	sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CFGR1), (AnalogWDGConfig->WatchdogMode |
									(AnalogWDGConfig->Channel & ADC_CFGR1_AWD1CH)));

	/*
	 *  HIGH/LOWのスレッシュホールド設定
	 */
	shift = ((sil_rew_mem((uint32_t *)(hadc->base+ADC_CFGR1_RES)) & ADC_CFGR1_RES) >> 3) * 2;
	sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_TR), ((AnalogWDGConfig->HighThreshold << (shift+16)) | (AnalogWDGConfig->LowThreshold<<shift)));
	return E_OK;
}


