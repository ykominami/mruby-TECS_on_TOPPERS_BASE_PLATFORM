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
 *  @(#) $Id: adc.c 698 2016-02-25 13:22:51Z roi $
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
 *  I2CポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_ADC(adcid)        ((uint_t)((adcid) - 1))

#define ADC_CHANNEL_TEMPSENSOR  ((uint32_t)ADC_CHANNEL_16)

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
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR),  RCC_APB2ENR_ADC1EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA2EN,
	TADR_DMA2_STM0_BASE, DMA_CHANNEL_0 },
  {	TADR_ADC2_BASE,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR),  RCC_APB2ENR_ADC2EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA2EN,
	TADR_DMA2_STM2_BASE, DMA_CHANNEL_1 },
  {	TADR_ADC3_BASE,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR),  RCC_APB2ENR_ADC3EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_DMA2EN,
	TADR_DMA2_STM0_BASE, DMA_CHANNEL_2 }
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
	hadc->ErrorCode |= ADC_ERROR_DMA;
	if(hadc->errorcallback != NULL)
		hadc->errorcallback(hadc);
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
	if(portid < 1 || portid > NUM_ADCPORT)
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

		hdma_adc.base = apcb->dmarxbase;
		hdma_adc.Init.Channel  = apcb->dmarxchannel;
		hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		if((pini->DMAContinuousRequests & ADC_DMANONECIRCULAR) != 0)
			hdma_adc.Init.Mode = DMA_NORMAL;
		else
			hdma_adc.Init.Mode = DMA_CIRCULAR;
		hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
		hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
		hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

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
	sil_modw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_ADCPRE, hadc->Init.ClockPrescaler);

	/*
	 *  ADC初期設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_SCAN, hadc->Init.ScanConvMode);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_RES, hadc->Init.Resolution);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_ALIGN, hadc->Init.DataAlign);

	/*
	 *  ADC外部トリガ設定
	 */
	if(hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START){
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_EXTSEL, hadc->Init.ExternalTrigConv);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_EXTEN, hadc->Init.ExternalTrigConvEdge);
	}
	else{
		sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), (ADC_CR2_EXTSEL | ADC_CR2_EXTEN));
	}

	/*
	 *  ADC継続モード設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_CONT, hadc->Init.ContinuousConvMode);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_DISCEN, hadc->Init.DiscontinuousConvMode);
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_DISCNUM, hadc->Init.NumDiscConversion << 13);

	/*
	 *  コンバージョン数設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), ADC_SQR1_L, (hadc->Init.NumConversion - 1) << 20);

	/*
	 *  ADC-DMAモード設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_DDS, hadc->Init.DMAContinuousRequests);

	/*
	 *  ADC EOC設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_EOCS, hadc->Init.EOCSelection);

	/*
	 *  ADCを初期状態に
	 */
	hadc->ErrorCode = ADC_ERROR_NONE;
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
	if(hadc == NULL)
		return E_PAR;

	/*
	 *  ADCリセット(コモン設定)
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_ADCRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2RSTR), RCC_APB2RSTR_ADCRST);

	/*
	 *  DMA停止
	 */
	if(hadc->hdmarx != NULL)
		dma_deinit(hadc->hdmarx);

	/*
	 *  ADCの状態初期化
	 */
	hadc->ErrorCode = ADC_ERROR_NONE;
	hadc->status = ADC_STATUS_RESET;
	return E_OK;
}

/*
 *  ADC-割込みモード開始処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_start_int(ADC_Handle_t* hadc)
{
	if(hadc == NULL)
		return E_PAR;

	/*
	 *  実行状態に移行
	 */
	hadc->status = ADC_STATUS_BUSY;
	hadc->ErrorCode = ADC_ERROR_NONE;

	/*
	 *  ADCが有効でなければ、ADCを設定し実行待ちを行う
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2)) & ADC_CR2_ADON) == 0){
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_ADON);
		sil_dly_nse(ADC_STAB_DELAY_US*1000);
	}

	/*
	 *  エラー転送割込み設定
	 */
	sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), (ADC_CR1_OVRIE | ADC_CR1_EOCIE));

	/*
	 *  マルチモード判定
	 */
	if((sil_rew_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR)) & ADC_CCR_MULTI) == 0){
		/*
		 *  非マルチモード：外部トリガでなければソフトスタート
		 */
		if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2)) & ADC_CR2_EXTEN) == 0){
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_SWSTART);
		}
	}
	else{
		/*
		 *  マルチモード：ADC1で外部トリガでなければソフトスタート
		 */
		if((hadc->base == TADR_ADC1_BASE) && ((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2)) & ADC_CR2_EXTEN) == 0)){
			sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_SWSTART);
		}
	}
	return E_OK;
}

/*
 *  ADC-割込みモード停止処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_end_int(ADC_Handle_t* hadc)
{
	if(hadc == NULL)
		return E_PAR;

	/*
	 *  転送、エラー割込み停止
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), (ADC_CR1_EOCIE | ADC_CR1_JEOCIE | ADC_CR1_OVRIE));

	/*
	 *  ADCモジュール停止
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_ADON);

	hadc->status = ADC_STATUS_READY;
	return E_OK;
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
	if(hadc == NULL)
		return E_PAR;

	/*
	 *  ADC DMA設定
	 */
	sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_DMA);

	/*
	 *  ADCオーバーラン割込み設定
	 */
	sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_OVRIE);

	/*
	 *  DMAコールバック関数設定
	 */
	hadc->hdmarx->xfercallback = adc_dmatrans_func;
	hadc->hdmarx->xferhalfcallback = adc_dmahalftrans_func;
	hadc->hdmarx->errorcallback = adc_dmaerror_func;

	/*
	 *  実行状態に移行
	 */
	hadc->status = ADC_STATUS_BUSY;
	hadc->ErrorCode = ADC_ERROR_NONE;
	hadc->NumCurrentConversionRank = hadc->Init.NumConversion;

	/*
	 *  DMAスタート
	 */
	dma_start(hadc->hdmarx, hadc->base+TOFF_ADC_DR, (uint32_t)pdata, length);

	/*
	 *  ADCが有効でなければ、ADCを設定し実行待ちを行う
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2)) & ADC_CR2_ADON) == 0){
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_ADON);
		sil_dly_nse(ADC_STAB_DELAY_US*1000);
	}

	/*
	 *  外部トリガ設定がなければ、ソフトウェアコンバージョンスタート設定を行う
	 */
	if((sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2)) & ADC_CR2_EXTEN) == 0){
		sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_SWSTART);
	}
	return E_OK;
}

/*
 *  ADC-DMAモード停止処理
 *  parameter1  hadc: ADCハンドラへのポインタ
 *  return ERコード
 */
ER
adc_end_dma(ADC_Handle_t* hadc)
{
	if(hadc == NULL)
		return E_PAR;

	/*
	 *  ADCモジュール停止
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_ADON);

	/*
	 *  オーバーラン割込みを停止
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_OVRIE);

	/*
	 *  ADCのDMAモードをオフ
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2), ADC_CR2_DMA);

	/*
	 *  DMAを停止
	 */
	if(hadc->hdmarx != NULL)
		dma_end(hadc->hdmarx);

	hadc->status = ADC_STATUS_READY;
	return E_OK;
}

/*
 *  ADC 割込みハンドラ実行関数
 */
void
adc_handler(ADC_Handle_t* hadc)
{
	uint32_t sr  = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_SR));
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1));
	uint32_t cr2 = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_CR2));

	/*
	 *  レギュラーレジスタの終了判定
	 */
	if((sr & ADC_SR_EOC) != 0 && (cr1 & ADC_CR1_EOCIE) != 0){
		if((cr2 & ADC_CR2_CONT) == 0)		/* ステータスの変更 */
			hadc->status = ADC_STATUS_EOC;
		else
			hadc->status = ADC_STATUS_BUSY_EOC;

		if((hadc->Init.ContinuousConvMode == ADC_CONTINUOUS_DISABLE) && ((cr2 & ADC_CR2_EXTEN) == 0)){
			if(hadc->Init.EOCSelection == ADC_EOC_SEQ_DISABLE){
				/*
				 *  転送とエラー割込みを停止
				 */
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_EOCIE);
				sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_OVRIE);
			}
			else{
				if (hadc->NumCurrentConversionRank != 0){
					hadc->NumCurrentConversionRank--;
				}

				/*
				 *  RANK終了判定
				 */
				if(hadc->NumCurrentConversionRank == 0){
					/*
					 *  転送とエラー割込みを停止
					 */
					sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_EOCIE);
					sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_OVRIE);
				}
			}
		}

		/*
		 *  終了コールバック関数の読み出し
		 */
		if(hadc->xfercallback != NULL)
			hadc->xfercallback(hadc);

		/*
		 * レギュラー転送の割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SR), ~ADC_SR_EOC);
	}

	/*
	 *  インジェクテェットチャネルの終了判定
	 */
	sr = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_SR));
	if((sr & ADC_SR_JEOC) != 0 && (cr1 & ADC_CR1_JEOCIE) != 0){
		if((cr2 & ADC_CR2_CONT) == 0)		/* ステータスの変更 */
			hadc->status = ADC_STATUS_EOC;
		else
			hadc->status = ADC_STATUS_BUSY_EOC;

		if(((hadc->Init.ContinuousConvMode == ADC_CONTINUOUS_DISABLE) || (cr1 & ADC_CR1_JAUTO) == 0) && (cr2 & ADC_CR2_JEXTEN) == 0){
			/*
			 *  インジェクテェットチャネル転送を停止
			 */
			sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_JEOCIE);
		}

		/*
		 *  インジェクテェットチャネル割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SR), ~ADC_SR_JEOC);
	}

	/*
	 *  アナログウォッチドック判定
	 */
	sr = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_SR));
	if((sr & ADC_SR_AWD) != 0 && (cr1 & ADC_CR1_AWDIE) != 0){
		/*
		 *  ウォッチドック発生状態に変更
		 */
		hadc->status = ADC_STATUS_AWD;

		/*
		 *  ウォッチドック割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SR), ~ADC_SR_AWD);

		/*
		 *  ウィンドウ領域アウトのコールバック関数読み出し
		 */
		if(hadc->outofwincallback != NULL)
			hadc->outofwincallback(hadc);
	}

	/*
	 *  オーバーランエラー判定
	 */
	sr = sil_rew_mem((uint32_t *)(hadc->base+TOFF_ADC_SR));
	if((sr & ADC_SR_OVR) != 0 && (cr1 & ADC_CR1_OVRIE) != 0){
		/*
		 *  オーバーラン発生状態に変更
		 */
		hadc->status = ADC_STATUS_READY;
		hadc->ErrorCode |= ADC_ERROR_OVR;

		/*
		 *  オーバーラン割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_SR), ~ADC_SR_OVR);

		/*
		 *  エラーコールバック関数を読み出し
		 */
		if(hadc->errorcallback != NULL)
			hadc->errorcallback(hadc);
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
		if(hadc->status > ADC_STATUS_READY)
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
	uint32_t shift, clkpin;

	if(hadc == NULL || sConfig == NULL)
		return E_PAR;

	clkpin = (sConfig->GpioBase - TADR_GPIOA_BASE) / 0x400;
	if(clkpin >= 12 || sConfig->GpioPin >= 16)
		return E_PAR;

	/*
	 *  チャネル有効設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), 1<<clkpin);
	sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));

	GPIO_InitStruct.mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.pull = GPIO_NOPULL;
	gpio_setup(sConfig->GpioBase, &GPIO_InitStruct, sConfig->GpioPin);

	/*
	 *  サンプリング時間設定
	 */
	if (sConfig->Channel > ADC_CHANNEL_9){	/* チャネル10から18 */
		shift = 3 * (sConfig->Channel - 10);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR1), (ADC_SMPR1_SMP10 << shift), (sConfig->SamplingTime << shift));
	}
	else{ 	/* チャネル0から9 */
		shift = 3 * sConfig->Channel;
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SMPR2), (ADC_SMPR2_SMP0 << shift), (sConfig->SamplingTime << shift));
	}

	/*
	 *  ランク設定
	 */
	if (sConfig->Rank < 7){	/* ランク1から6 */
		shift = 5 * (sConfig->Rank - 1);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR3), (ADC_SQR3_SQ1 << shift), (sConfig->Channel << shift));
	}
	else if (sConfig->Rank < 13){	/* ランク7から12 */
		shift = 5 * (sConfig->Rank - 7);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR2), (ADC_SQR2_SQ7 << shift), (sConfig->Channel << shift));
	}
	else{	/* ランク13から16 */
		shift = 5 * (sConfig->Rank - 13);
		sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_SQR1), (ADC_SQR1_SQ13 << shift), (sConfig->Channel << shift));
	}

	/*
	 *  電圧管理用チャネル設定
	 */
	if ((hadc->base == TADR_ADC1_BASE) && (sConfig->Channel == ADC_CHANNEL_VBAT)){
		sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_VBATE);
	}

	/*
	 *  温度センサー専用チャネル設定
	 */
	if ((hadc->base == TADR_ADC1_BASE) && ((sConfig->Channel == ADC_CHANNEL_TEMPSENSOR) || (sConfig->Channel == ADC_CHANNEL_VREFINT))){
		sil_orw_mem((uint32_t *)(TADR_ADC_BASE+TOFF_ADC_CCR), ADC_CCR_TSVREFE);
		if((sConfig->Channel == ADC_CHANNEL_TEMPSENSOR)){
			sil_dly_nse(ADC_TEMPSENSOR_DELAY_US*1000);
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
	if(hadc == NULL || AnalogWDGConfig == NULL)
		return E_PAR;

	/*
	 *  割込み設定
	 */
	sil_modw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_AWDIE, AnalogWDGConfig->ITMode);

	/*
	 *  モード設定
	 */
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), (ADC_CR1_AWDSGL | ADC_CR1_JAWDEN | ADC_CR1_AWDEN));

	/* Set the analog watchdog enable mode */
	sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), AnalogWDGConfig->WatchdogMode);
	sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_HTR), AnalogWDGConfig->HighThreshold);
  
	/*
	 *  HIGH/LOWのスレッシュホールド設定
	 */
	sil_wrw_mem((uint32_t *)(hadc->base+TOFF_ADC_LTR), AnalogWDGConfig->LowThreshold);
	sil_andw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), ADC_CR1_AWDCH);

	/*
	 *  対応チャネル設定
	 */
    sil_orw_mem((uint32_t *)(hadc->base+TOFF_ADC_CR1), AnalogWDGConfig->Channel);
	return E_OK;
}


