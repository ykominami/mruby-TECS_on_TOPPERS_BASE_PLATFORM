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
 *  @(#) $Id: adc.h 698 2016-02-24 21:08:38Z roi $
 */
/*
 * 
 *  STM32F4xx ADCデバイスドライバの外部宣言
 *
 */

#ifndef _ADC_H_
#define _ADC_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  ADCポート定義
 */
#define ADC1_PORTID             1
#define ADC2_PORTID             2
#define ADC3_PORTID             3
#define NUM_ADCPORT             3

/*
 *  ADCクロックプリスケーラ定義(ClockPrescaler)
 */ 
#define ADC_CLOCK_SYNC_PCLK_DIV2        0x00000000
#define ADC_CLOCK_SYNC_PCLK_DIV4        ADC_CCR_ADCPRE_0
#define ADC_CLOCK_SYNC_PCLK_DIV6        ADC_CCR_ADCPRE_1
#define ADC_CLOCK_SYNC_PCLK_DIV8        ADC_CCR_ADCPRE

/*
 *  ADCリゾリューション定義(Resolution)
 */
#define ADC_RESOLUTION_12B              0x00000000
#define ADC_RESOLUTION_10B              ADC_CR1_RES_0
#define ADC_RESOLUTION_8B               ADC_CR1_RES_1
#define ADC_RESOLUTION_6B               ADC_CR1_RES

/*
 *  ADCデータアライン定義(DataAlign)
 */ 
#define ADC_DATAALIGN_RIGHT             0x00000000
#define ADC_DATAALIGN_LEFT              ADC_CR2_ALIGN


/*
 *  ADCスキャンコンバージョンモード(ScanConvMode)
 */
#define ADC_SCANMODE_DISABLE            0x00000000
#define ADC_SCANMODE_ENABLE             ADC_CR1_SCAN

/*
 *  ADC EOCシーケンス設定(EOCSelection)
 */
#define ADC_EOC_SEQ_DISABLE             0x00000000
#define ADC_EOC_SEQ_ENABLE              ADC_CR2_EOCS

/*
 *  ADC継続モード設定(ContinuousConvMode)
 */
#define ADC_CONTINUOUS_DISABLE          0x00000000
#define ADC_CONTINUOUS_ENABLE           ADC_CR2_CONT

/*
 *  ADC DMA設定定義(DMAContinuousRequests)
 */
#define ADC_DMACONTINUOUS_DISABLE       0x00000000
#define ADC_DMANONECIRCULAR             0x80000000
#define ADC_DMACONTINUOUS_ENABLE        ADC_CR2_DDS
#define ADC_DMACONTINUOUS_ENABLE2       (ADC_CR2_DDS | ADC_DMANONECIRCULAR)

/*
 *  ADC 非継続モード設定(DiscontinuousConvMode)
 */
#define ADC_DISCONTINUOUS_DISABLE       0x00000000
#define ADC_DISCONTINUOUS_ENABLE        ADC_CR1_DISCEN

/*
 *  ADC外部トリガソース設定(ExternalTrigConv)
 */
#define ADC_EXTERNALTRIGCONV_T1_CC1     0x00000000
#define ADC_EXTERNALTRIGCONV_T1_CC2     ADC_CR2_EXTSEL_0
#define ADC_EXTERNALTRIGCONV_T1_CC3     ADC_CR2_EXTSEL_1
#define ADC_EXTERNALTRIGCONV_T2_CC2     (ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T2_CC3     ADC_CR2_EXTSEL_2
#define ADC_EXTERNALTRIGCONV_T2_CC4     (ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T2_TRGO    (ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1)
#define ADC_EXTERNALTRIGCONV_T3_CC1     (ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T3_TRGO    ADC_CR2_EXTSEL_3
#define ADC_EXTERNALTRIGCONV_T4_CC4     (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T5_CC1     (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_1)
#define ADC_EXTERNALTRIGCONV_T5_CC2     (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T5_CC3     (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_2)
#define ADC_EXTERNALTRIGCONV_T8_CC1     (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_0)
#define ADC_EXTERNALTRIGCONV_T8_TRGO    (ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1)
#define ADC_EXTERNALTRIGCONV_Ext_IT11   ADC_CR2_EXTSEL
#define ADC_SOFTWARE_START              (ADC_CR2_EXTSEL + 1)

/*
 *  ADC外部トリガエッジ設定()
 */
#define ADC_EXTERNALTRIGCONVEDGE_NONE           0x00000000
#define ADC_EXTERNALTRIGCONVEDGE_RISING         ADC_CR2_EXTEN_0
#define ADC_EXTERNALTRIGCONVEDGE_FALLING        ADC_CR2_EXTEN_1
#define ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING  ADC_CR2_EXTEN


/*
 *  ADCチャネル定義
 */
#define ADC_CHANNEL_0                   0x00000000
#define ADC_CHANNEL_1                   ADC_CR1_AWDCH_0
#define ADC_CHANNEL_2                   ADC_CR1_AWDCH_1
#define ADC_CHANNEL_3                   (ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_4                   ADC_CR1_AWDCH_2
#define ADC_CHANNEL_5                   (ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_6                   (ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1)
#define ADC_CHANNEL_7                   (ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_8                   ADC_CR1_AWDCH_3
#define ADC_CHANNEL_9                   (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_10                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1)
#define ADC_CHANNEL_11                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_12                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2)
#define ADC_CHANNEL_13                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_14                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1)
#define ADC_CHANNEL_15                  (ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_16                  ADC_CR1_AWDCH_4
#define ADC_CHANNEL_17                  (ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_18                  (ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_1)

#define ADC_CHANNEL_VREFINT             ADC_CHANNEL_17
#define ADC_CHANNEL_VBAT                ADC_CHANNEL_18

/*
 *  ADCサンプリング時間設定
 */
#define ADC_SAMPLETIME_3CYCLES          0x00000000
#define ADC_SAMPLETIME_15CYCLES         ADC_SMPR1_SMP10_0
#define ADC_SAMPLETIME_28CYCLES         ADC_SMPR1_SMP10_1
#define ADC_SAMPLETIME_56CYCLES         (ADC_SMPR1_SMP10_1 | ADC_SMPR1_SMP10_0)
#define ADC_SAMPLETIME_84CYCLES         ADC_SMPR1_SMP10_2
#define ADC_SAMPLETIME_112CYCLES        (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_0)
#define ADC_SAMPLETIME_144CYCLES        (ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_1)
#define ADC_SAMPLETIME_480CYCLES        ADC_SMPR1_SMP10


/*
 *  ADCアナログウォッチドックモード設定
 */
#define ADC_ANALOGWATCHDOG_NONE               0x00000000
#define ADC_ANALOGWATCHDOG_SINGLE_REG         (ADC_CR1_AWDSGL | ADC_CR1_AWDEN)
#define ADC_ANALOGWATCHDOG_SINGLE_INJEC       (ADC_CR1_AWDSGL | ADC_CR1_JAWDEN)
#define ADC_ANALOGWATCHDOG_SINGLE_REGINJEC    (ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_JAWDEN)
#define ADC_ANALOGWATCHDOG_ALL_REG            ADC_CR1_AWDEN
#define ADC_ANALOGWATCHDOG_ALL_INJEC          ADC_CR1_JAWDEN
#define ADC_ANALOGWATCHDOG_ALL_REGINJEC       (ADC_CR1_AWDEN | ADC_CR1_JAWDEN)

/*
 *  ADCアナログウォッチドック割込みモード設定
 */
#define ADC_ANALOGWATCHDOG_ITMODE_DISABLE     0x00000000
#define ADC_ANALOGWATCHDOG_ITMODE_ENABLE      ADC_CR1_AWDIE

/*
 *  ADCハードウェア設定構造体
 */
typedef struct _ADC_PortControlBlock{
	uint32_t              base;
	uint32_t              adcclockbase;
	uint32_t              adcclockbit;
	uint32_t              dmaclockbase;
	uint32_t              dmaclockbit;
	uint32_t              dmarxbase;
	uint32_t              dmarxchannel;
} ADC_PortControlBlock;

/*
 *  ADCの初期化構造体
 */
typedef struct
{
	uint32_t              ClockPrescaler;				/* ADCクロックプリスケーラ値(各ADCにて共通値) */
	uint32_t              Resolution;					/* ADCのリゾリューション */
	uint32_t              DataAlign;					/* ADC結果データアライン設定 */
	uint32_t              ScanConvMode;					/* ADCスキャンコンバージョンモード */
	uint32_t              EOCSelection;					/* ADC EOC設定 */
	uint32_t              ContinuousConvMode;			/* ADC継続モード */
	uint32_t              DMAContinuousRequests;		/* ADC-DMAモード */
	uint32_t              NumConversion;				/* ADCコンバージョン数 */
	uint32_t              DiscontinuousConvMode;		/* ADC非継続モード */
	uint32_t              NumDiscConversion;			/* ADC非継続変換数 */
	uint32_t              ExternalTrigConv;				/* ADC外部トリガ設定 */
	uint32_t              ExternalTrigConvEdge;			/* ADC外部トリガエッジ設定 */
}ADC_Init_t;

/*
 *  ADCハンドラ構造体
 */ 
typedef struct __ADC_Handle_t ADC_Handle_t;
struct __ADC_Handle_t
{
	uint32_t              base;							/* ADCベースアドレス */
	ADC_Init_t            Init;							/* ADC初期化データ */
	volatile uint32_t     NumCurrentConversionRank;		/* ADC現在のコンバージョンランク値 */
	DMA_Handle_t          *hdmarx;						/* ADC用DMAハンドラへのポインタ */
	void                  (*xfercallback)(ADC_Handle_t * hadc);		/* ADC 転送完了コールバック */
	void                  (*xferhalfcallback)(ADC_Handle_t * hadc);	/* ADC 半分転送コールバック */
	void                  (*outofwincallback)(ADC_Handle_t * hadc);	/* ADC アナログウォッチドックコールバック */
	void                  (*errorcallback)(ADC_Handle_t * hadc);	/* ADC エラーコールバック */
	volatile uint32_t     status;						/* ADCステータス */
	volatile uint32_t     ErrorCode;					/* ADCエラー値 */
};

/*
 *  チャンネル設定構造体
 */
typedef struct
{
	uint32_t              Channel;						/* ADCチャンネル番号 */
	uint32_t              Rank;							/* ADCランク番号(1-16) */
	uint32_t              SamplingTime;					/* ADCサンプリング時間 */
	uint32_t              GpioBase;						/* ADC-GPIOベースアドレス */
	uint32_t              GpioPin;						/* ADC-GPIOピン番号 */
}ADC_ChannelConf_t;

/*
 *  ADCアナログウォッチドック設定構造体
 */
typedef struct
{
	uint32_t              WatchdogMode;					/* ADCアナログウォッチドッグモード */
	uint32_t              HighThreshold;				/* ADCスレッシュホールド上限値 */
	uint32_t              LowThreshold;					/* ADCスレッシュホールド下限値 */
	uint32_t              Channel;						/* ADC対応チャネル番号 */
	uint32_t              ITMode;						/* ADCアナログウォッチドック割込み設定 */
}ADC_AnalogWDGConf_t;

/*
 *  ADCのステータス定義
 */
#define ADC_STATUS_RESET        0x00    /* ADC 未使用状態 */
#define ADC_STATUS_READY        0x01    /* ADC レディ状態 */
#define ADC_STATUS_BUSY         0x02    /* ADC ビジィ状態 */
#define ADC_STATUS_EOC          0x10    /* ADC 終了状態 */
#define ADC_STATUS_BUSY_EOC     0x12    /* ADC ビジィ終了発生状態 */ 
#define ADC_STATUS_AWD          0x20    /* ADC アナログウォッチドック発生状態 */

/*
 *  ADCエラー定義
 */
#define ADC_ERROR_NONE          0x00	/* No error             */
#define ADC_ERROR_OVR           0x01	/* OVR error            */
#define ADC_ERROR_DMA           0x02	/* DMA transfer error   */


extern ADC_Handle_t *adc_init(ID portid, ADC_Init_t *pini);
extern ER adc_deinit(ADC_Handle_t *hadc);
extern ER adc_start_int(ADC_Handle_t* hadc);
extern ER adc_end_int(ADC_Handle_t* hadc);
extern ER adc_start_dma(ADC_Handle_t* hadc, uint32_t* pdata, uint32_t length);
extern ER adc_end_dma(ADC_Handle_t* hadc);
extern ER adc_setupchannel(ADC_Handle_t* hadc, ADC_ChannelConf_t* sConfig);
extern ER adc_setupwatchdog(ADC_Handle_t* hadc, ADC_AnalogWDGConf_t* AnalogWDGConfig);
extern uint32_t adc_getvalue(ADC_Handle_t* hadc);

extern void adc_handler(ADC_Handle_t* hadc);
extern void adc_int_handler(void);

#ifdef __cplusplus
}
#endif

#endif	/* _ADC_H_ */

