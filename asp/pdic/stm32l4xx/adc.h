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
 *  @(#) $Id: adc.h 698 2017-07-31 14:26:01Z roi $
 */
/*
 * 
 *  STM32L4xx ADCデバイスドライバの外部宣言
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
#define ADC_CLOCK_SYNC_PCLK_DIV1        ADC_CCR_CKMODE_0	/* ADC synchronous clock derived from AHB clock not divided  */
#define ADC_CLOCK_SYNC_PCLK_DIV2        ADC_CCR_CKMODE_1	/* ADC synchronous clock derived from AHB clock divided by 2 */
#define ADC_CLOCK_SYNC_PCLK_DIV4        ADC_CCR_CKMODE		/* ADC synchronous clock derived from AHB clock divided by 4 */

#define ADC_CLOCK_ASYNC_DIV1            0x00000000							/* ADC asynchronous clock not divided    */
#define ADC_CLOCK_ASYNC_DIV2            ADC_CCR_PRESC_0						/* ADC asynchronous clock divided by 2   */
#define ADC_CLOCK_ASYNC_DIV4            ADC_CCR_PRESC_1						/* ADC asynchronous clock divided by 4   */
#define ADC_CLOCK_ASYNC_DIV6            (ADC_CCR_PRESC_1|ADC_CCR_PRESC_0)	/* ADC asynchronous clock divided by 6   */
#define ADC_CLOCK_ASYNC_DIV8            ADC_CCR_PRESC_2						/* ADC asynchronous clock divided by 8   */
#define ADC_CLOCK_ASYNC_DIV10           (ADC_CCR_PRESC_2|ADC_CCR_PRESC_0)	/* ADC asynchronous clock divided by 10  */
#define ADC_CLOCK_ASYNC_DIV12           (ADC_CCR_PRESC_2|ADC_CCR_PRESC_1)	/* ADC asynchronous clock divided by 12  */
#define ADC_CLOCK_ASYNC_DIV16           (ADC_CCR_PRESC_2|ADC_CCR_PRESC_1|ADC_CCR_PRESC_0)	/* ADC asynchronous clock divided by 16  */
#define ADC_CLOCK_ASYNC_DIV32           ADC_CCR_PRESC_3						/* ADC asynchronous clock divided by 32  */
#define ADC_CLOCK_ASYNC_DIV64           (ADC_CCR_PRESC_3|ADC_CCR_PRESC_0)	/* ADC asynchronous clock divided by 64  */
#define ADC_CLOCK_ASYNC_DIV128          (ADC_CCR_PRESC_3|ADC_CCR_PRESC_1)	/* ADC asynchronous clock divided by 128 */
#define ADC_CLOCK_ASYNC_DIV256          (ADC_CCR_PRESC_3|ADC_CCR_PRESC_1|ADC_CCR_PRESC_0)	/* ADC asynchronous clock divided by 256 */

/*
 *  ADCリゾリューション定義(Resolution)
 */
#define ADC_RESOLUTION_12B              0x00000000
#define ADC_RESOLUTION_10B              ADC_CFGR_RES_0
#define ADC_RESOLUTION_8B               ADC_CFGR_RES_1
#define ADC_RESOLUTION_6B               ADC_CFGR_RES

/*
 *  ADCデータアライン定義(DataAlign)
 */ 
#define ADC_DATAALIGN_RIGHT             0x00000000
#define ADC_DATAALIGN_LEFT              ADC_CFGR_ALIGN


/*
 *  ADCスキャンコンバージョンモード(ScanConvMode)
 */
#define ADC_SCANMODE_DISABLE            0x00000000
#define ADC_SCANMODE_ENABLE             0x00000001

/*
 *  ADC EOCシーケンス設定(EOCSelection)
 */
#define ADC_EOC_SEQ_DISABLE             0x00000000
#define ADC_EOC_SINGLE_CONV             ADC_ISR_EOC
#define ADC_EOC_SEQ_CONV                ADC_ISR_EOS

/*
 *  ADC継続モード設定(ContinuousConvMode)
 */
#define ADC_CONTINUOUS_DISABLE          0x00000000
#define ADC_CONTINUOUS_ENABLE           ADC_CFGR_CONT

/*
 *  ADC DMA設定定義(DMAContinuousRequests)
 */
#define ADC_DMACONTINUOUS_DISABLE       0x00000000
#define ADC_DMACONTINUOUS_ENABLE        ADC_CFGR_DMACFG

/*
 *  ADC 非継続モード設定(DiscontinuousConvMode)
 */
#define ADC_DISCONTINUOUS_DISABLE       0x00000000
#define ADC_DISCONTINUOUS_ENABLE        ADC_CFGR_DISCEN

/*
 *  ADC外部トリガソース設定(ExternalTrigConv)
 */
#define ADC_EXTERNALTRIGCONV_T1_CC1     0x00000000								/* Event 0 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T1_CC2     ADC_CFGR_EXTSEL_0						/* Event 1 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T1_CC3     ADC_CFGR_EXTSEL_1						/* Event 2 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T2_CC2     (ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)	/* Event 3 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T2_CC3     ADC_CFGR_EXTSEL_2						/* Event 4 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T4_CC4     (ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_0)	/* Event 5 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_Ext_IT11   (ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_1)	/* Event 6 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T8_TRGO    (ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)	/* Event 7 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T8_TRGO2   ADC_CFGR_EXTSEL_3						/* Event 8 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T1_TRGO	(ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_0)	/* Event 9 triggers regular group conversion start  */
#define ADC_EXTERNALTRIGCONV_T1_TRGO2   (ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1)	/* Event 10 triggers regular group conversion start */
#define ADC_EXTERNALTRIGCONV_T2_TRGO	(ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)	/* Event 11 triggers regular group conversion start */
#define ADC_EXTERNALTRIGCONV_T4_TRGO    (ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_2)	/* Event 12 triggers regular group conversion start */
#define ADC_EXTERNALTRIGCONV_T6_TRGO    (ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_0)	/* Event 13 triggers regular group conversion start */
#define ADC_EXTERNALTRIGCONV_T15_TRGO   (ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_1)	/* Event 14 triggers regular group conversion start */
#define ADC_EXTERNALTRIGCONV_T3_CC4     ADC_CFGR_EXTSEL							/* Event 15 triggers regular group conversion start */
#define ADC_SOFTWARE_START              0x00000001								/* Software triggers regular group conversion start */

/*
 *  ADC外部トリガエッジ設定()
 */
#define ADC_EXTERNALTRIGCONVEDGE_NONE           0x00000000
#define ADC_EXTERNALTRIGCONVEDGE_RISING         ADC_CFGR_EXTEN_0
#define ADC_EXTERNALTRIGCONVEDGE_FALLING        ADC_CFGR_EXTEN_1
#define ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING  ADC_CFGR_EXTEN


/*
 *  ADCチャネル定義
 */
#define ADC_CHANNEL_0                   0x00000000								/* ADC channel 0  */
#define ADC_CHANNEL_1                   ADC_SQR3_SQ10_0							/* ADC channel 1  */
#define ADC_CHANNEL_2                   ADC_SQR3_SQ10_1							/* ADC channel 2  */
#define ADC_CHANNEL_3                   (ADC_SQR3_SQ10_1 | ADC_SQR3_SQ10_0)		/* ADC channel 3  */
#define ADC_CHANNEL_4                   ADC_SQR3_SQ10_2							/* ADC channel 4  */
#define ADC_CHANNEL_5                   (ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_0)		/* ADC channel 5  */
#define ADC_CHANNEL_6                   (ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_1)		/* ADC channel 6  */
#define ADC_CHANNEL_7                   (ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_1 | ADC_SQR3_SQ10_0)	/* ADC channel 7  */
#define ADC_CHANNEL_8                   ADC_SQR3_SQ10_3							/* ADC channel 8  */
#define ADC_CHANNEL_9                   (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_0)		/* ADC channel 9  */
#define ADC_CHANNEL_10                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_1)		/* ADC channel 10 */
#define ADC_CHANNEL_11                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_1 | ADC_SQR3_SQ10_0)	/* ADC channel 11 */
#define ADC_CHANNEL_12                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_2)		/* ADC channel 12 */
#define ADC_CHANNEL_13                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_0)	/* ADC channel 13 */
#define ADC_CHANNEL_14                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_1)	/* ADC channel 14 */
#define ADC_CHANNEL_15                  (ADC_SQR3_SQ10_3 | ADC_SQR3_SQ10_2 | ADC_SQR3_SQ10_1 | ADC_SQR3_SQ10_0)	/* ADC channel 15 */
#define ADC_CHANNEL_16                  ADC_SQR3_SQ10_4							/* ADC channel 16 */
#define ADC_CHANNEL_17                  (ADC_SQR3_SQ10_4 | ADC_SQR3_SQ10_0)		/* ADC channel 17 */
#define ADC_CHANNEL_18                  (ADC_SQR3_SQ10_4 | ADC_SQR3_SQ10_1)		/* ADC channel 18 */

#define ADC_CHANNEL_TEMPSENSOR          ADC_CHANNEL_17
#define ADC_CHANNEL_VBAT                ADC_CHANNEL_18
#define ADC_CHANNEL_VREFINT             ADC_CHANNEL_0

/*
 *  ADCサンプリング時間設定
 */
#define ADC_SAMPLETIME_2CYCLES_5        0x00000000								/* Sampling time 2.5 ADC clock cycle    */
#define ADC_SAMPLETIME_6CYCLES_5        ADC_SMPR2_SMP10_0						/* Sampling time 6.5 ADC clock cycles   */
#define ADC_SAMPLETIME_12CYCLES_5       ADC_SMPR2_SMP10_1						/* Sampling time 12.5 ADC clock cycles  */
#define ADC_SAMPLETIME_24CYCLES_5       (ADC_SMPR2_SMP10_1 | ADC_SMPR2_SMP10_0)	/* Sampling time 24.5 ADC clock cycles  */
#define ADC_SAMPLETIME_47CYCLES_5       ADC_SMPR2_SMP10_2						/* Sampling time 47.5 ADC clock cycles  */
#define ADC_SAMPLETIME_92CYCLES_5       (ADC_SMPR2_SMP10_2 | ADC_SMPR2_SMP10_0)	/* Sampling time 92.5 ADC clock cycles  */
#define ADC_SAMPLETIME_247CYCLES_5      (ADC_SMPR2_SMP10_2 | ADC_SMPR2_SMP10_1)	/* Sampling time 247.5 ADC clock cycles */
#define ADC_SAMPLETIME_640CYCLES_5      ADC_SMPR2_SMP10							/* Sampling time 640.5 ADC clock cycles */

/*
 *  ADCローパワーオートウェイト
 */
#define ADC_LOWAUTOWAIT_DISABLE         0x00000000
#define ADC_LOWAUTOWAIT_ENABLE          ADC_CFGR_AUTDLY

/*
 *  ADCオーバーデータライティング
 */
#define ADC_OVR_DATA_PRESERVED          0x00000000
#define ADC_OVR_DATA_OVERWRITTEN        ADC_CFGR_OVRMOD

/*
 *  ADCオーバーサンプリングモード
 */
#define ADC_OVR_SAMPLING_DISABLE        0x00000000
#define ADC_OVR_SAMPLING_ENABLE         0x00000001

/*
 *  ADCオーバーサンプリングラティオ
 */
#define ADC_OVERSAMPLING_RATIO_2        0x00000000
#define ADC_OVERSAMPLING_RATIO_4        ADC_CFGR2_OVSR_0
#define ADC_OVERSAMPLING_RATIO_8        ADC_CFGR2_OVSR_1
#define ADC_OVERSAMPLING_RATIO_16       (ADC_CFGR2_OVSR_0 | ADC_CFGR2_OVSR_1)
#define ADC_OVERSAMPLING_RATIO_32       ADC_CFGR2_OVSR_2
#define ADC_OVERSAMPLING_RATIO_64       (ADC_CFGR2_OVSR_0 | ADC_CFGR2_OVSR_2)
#define ADC_OVERSAMPLING_RATIO_128      (ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_2)
#define ADC_OVERSAMPLING_RATIO_256      ADC_CFGR2_OVSR

/*
 *  ADCオーバーサンプリングライトビットシフト
 */
#define ADC_RIGHTBITSHIFT_NONE          0x00000000
#define ADC_RIGHTBITSHIFT_1             ADC_CFGR2_OVSS_0
#define ADC_RIGHTBITSHIFT_2             ADC_CFGR2_OVSS_1
#define ADC_RIGHTBITSHIFT_3             (ADC_CFGR2_OVSS_0 | ADC_CFGR2_OVSS_1)
#define ADC_RIGHTBITSHIFT_4             ADC_CFGR2_OVSS_2
#define ADC_RIGHTBITSHIFT_5             (ADC_CFGR2_OVSS_0 | ADC_CFGR2_OVSS_2)
#define ADC_RIGHTBITSHIFT_6             (ADC_CFGR2_OVSS_1 | ADC_CFGR2_OVSS_2)
#define ADC_RIGHTBITSHIFT_7             (ADC_CFGR2_OVSS_0 | ADC_CFGR2_OVSS_1 | ADC_CFGR2_OVSS_2)
#define ADC_RIGHTBITSHIFT_8             ADC_CFGR2_OVSS_3

/*
 *  ADCオーバーサンプリングトリガ
 */
#define ADC_TRIGGEREDMODE_SINGLE_TRIGGER    0x00000000
#define ADC_TRIGGEREDMODE_MULTI_TRIGGER     ADC_CFGR2_TROVS

/*
 *  ADCレギュラーオーバーサンプリングモード
 */
#define ADC_REGOVERSAMPLING_CONTINUED_MODE  0x00000000		/*  Oversampling buffer maintained during injection sequence */
#define ADC_REGOVERSAMPLING_RESUMED_MODE    ADC_CFGR2_ROVSM	/*  Oversampling buffer zeroed during injection sequence     */

/*
 *  ADC Extended Single-ended/Differential input mode
 */
#define ADC_SINGLE_ENDED                    0x00000000		/* ADC channel set in single-ended input mode */
#define ADC_DIFFERENTIAL_ENDED              ADC_CR_ADCALDIF	/* ADC channel set in differential mode       */

/*
 * ADC Extended Offset Number
 */
#define ADC_OFFSET_NONE                     0x0000		/* No offset correction */
#define ADC_OFFSET_1                        0x0001		/* Offset correction to apply to a first channel  */
#define ADC_OFFSET_2                        0x0002		/* Offset correction to apply to a second channel */
#define ADC_OFFSET_3                        0x0003		/* Offset correction to apply to a third channel  */
#define ADC_OFFSET_4                        0x0004		/* Offset correction to apply to a fourth channel */

/*
 *  ADCアナログウォッチドックモード設定
 */
#define ADC_ANALOGWATCHDOG_NONE             0x00000000								/* No analog watchdog selected */
#define ADC_ANALOGWATCHDOG_SINGLE_REG       (ADC_CFGR_AWD1SGL | ADC_CFGR_AWD1EN)	/* Analog watchdog applied to a regular group single channel */
#define ADC_ANALOGWATCHDOG_SINGLE_INJEC     (ADC_CFGR_AWD1SGL | ADC_CFGR_JAWD1EN)	/* Analog watchdog applied to an injected group single channel */
#define ADC_ANALOGWATCHDOG_SINGLE_REGINJEC  (ADC_CFGR_AWD1SGL | ADC_CFGR_AWD1EN | ADC_CFGR_JAWD1EN)	/* Analog watchdog applied to a regular and injected groups single channel */
#define ADC_ANALOGWATCHDOG_ALL_REG          ADC_CFGR_AWD1EN							/* Analog watchdog applied to regular group all channels */
#define ADC_ANALOGWATCHDOG_ALL_INJEC        ADC_CFGR_JAWD1EN						/* Analog watchdog applied to injected group all channels */
#define ADC_ANALOGWATCHDOG_ALL_REGINJEC     (ADC_CFGR_AWD1EN | ADC_CFGR_JAWD1EN)	/* Analog watchdog applied to regular and injected groups all channels */

/*
 *  ADCアナログウォッチドック割込みモード設定
 */
#define ADC_ANALOGWATCHDOG_ITMODE_DISABLE   0x00000000
#define ADC_ANALOGWATCHDOG_ITMODE_ENABLE    0x00000001

/*
 *  ADCハードウェア設定構造体
 */
typedef struct _ADC_PortControlBlock{
	uint32_t              base;
	uint32_t              adcclockbase;
	uint32_t              adcclockbit;
	uint32_t              dmaclockbase;
	uint32_t              dmaclockbit;
	uint32_t              dmarxchannel;
	uint32_t              dmarxrequest;
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

	uint32_t              LowPowerAutoWait;				/* ADCローパワーオートウェイト */
	uint32_t              Overrun;						/* ADCDRレジスタのオーバーラン書き込み設定 */
	uint32_t              OversamplingMode;				/* ADCオーバーランサンプリング設定 */
	uint32_t              OversamplingRatio;			/* ADCオーバーランサンプリング Ratio. */
	uint32_t              OversamplingRightBitShift;	/* ADCオーバーランサンプリング Configures the division coefficient */
	uint32_t              OversamplingTriggeredMode;	/* ADCオーバーランサンプリング triggered  mode. */
	uint32_t              OversamplingStopReset;		/* ADCオーバーランサンプリング regular oversampling mode. */
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
	const ADC_PortControlBlock  *apcb;					/* ADC PCB */
	void                  (*xfercallback)(ADC_Handle_t * hadc);		/* ADC 転送完了コールバック */
	void                  (*xferhalfcallback)(ADC_Handle_t * hadc);	/* ADC 半分転送コールバック */
	void                  (*eosmpcallback)(ADC_Handle_t * hadc);	/* ADC EOSサンプリングコールバック */
	void                  (*injectedcallback)(ADC_Handle_t * hadc);	/* ADC IJECTEDコールバック */
	void                  (*outofwincallback)(ADC_Handle_t * hadc);	/* ADC アナログウォッチドックコールバック */
	void                  (*errorcallback)(ADC_Handle_t * hadc);	/* ADC エラーコールバック */
	volatile uint32_t     status;						/* ADCステータス */
	volatile uint32_t     errorcode;					/* ADCエラー値 */
};

/*
 *  チャンネル設定構造体
 */
typedef struct
{
	uint32_t              Channel;						/* ADCチャンネル番号 */
	uint32_t              Rank;							/* ADCランク番号(1-16) */
	uint32_t              SamplingTime;					/* ADCサンプリング時間 */
	uint32_t              SingleDiff;					/* ADCsingle-ended or differential input */
	uint32_t              OffsetNumber;					/* ADCoffset number */
	uint32_t              Offset;						/* ADC offset */
	uint32_t              GpioBase;						/* ADC-GPIOベースアドレス */
	uint32_t              GpioPin;						/* ADC-GPIOピン番号 */
}ADC_ChannelConf_t;

/*
 *  ADCアナログウォッチドック設定構造体
 */
typedef struct
{
	uint32_t              WatchdogNumber;				/* ADCWatchdog番号(1-3) */
	uint32_t              WatchdogMode;					/* ADCアナログウォッチドッグモード */
	uint32_t              HighThreshold;				/* ADCスレッシュホールド上限値 */
	uint32_t              LowThreshold;					/* ADCスレッシュホールド下限値 */
	uint32_t              Channel;						/* ADC対応チャネル番号 */
	uint32_t              ITMode;						/* ADCアナログウォッチドック割込み設定 */
}ADC_AnalogWDGConf_t;

/*
 *  ADCのステータス定義
 */
#define ADC_STATUS_RESET        0x0000	/* ADC 未使用状態 */
#define ADC_STATUS_READY        0x0001	/* ADC レディ状態 */
#define ADC_STATUS_BUSY         0x0002	/* ADC ビジィ状態 */
#define ADC_STATUS_EOC          0x0010	/* ADC 終了状態 */
#define ADC_STATUS_EOC_INJ      0x0020	/* ADC INJ終了状態 */
#define ADC_STATUS_BUSY_EOC     0x0012	/* ADC ビジィ終了発生状態 */
#define ADC_STATUS_BUSY_INJ		0x0022	/* ADC ビジィINJ状態 */
#define ADC_STATUS_AWD1         0x0100	/* ADC アナログウォッチドック1発生状態 */
#define ADC_STATUS_AWD2         0x0200	/* ADC アナログウォッチドック1発生状態 */
#define ADC_STATUS_AWD3         0x0300	/* ADC アナログウォッチドック1発生状態 */
#define ADC_STATUS_ERROR		0x1000	/* ADC エラー状態 */

/*
 *  ADCエラー定義
 */
#define ADC_ERROR_NONE          0x00	/* No error             */
#define ADC_ERROR_OVR           0x01	/* OVR error            */
#define ADC_ERROR_DMA           0x02	/* DMA transfer error   */
#define ADC_ERROR_INT           0x04	/* INTERNAL error       */


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
extern void adc_int12_handler(void);
extern void adc_int3_handler(void);

#ifdef __cplusplus
}
#endif

#endif	/* _ADC_H_ */

