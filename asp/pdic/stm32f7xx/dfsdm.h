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
 *  @(#) $Id: dfsdm.h 698 2017-03-19 18:40:55Z roi $
 */
/*
 * 
 *  STM32F7xx DFSDMデバイスドライバの外部宣言
 *
 */

#ifndef _DFSDM_H_
#define _DFSDM_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  DFSDM CHANNEL状態定義
 */
#define DFSDM_CHANNEL_STATE_RESET   0x00000000	/* DFSDM CHANNELリセット状態 */
#define DFSDM_CHANNEL_STATE_READY   0x00000001	/* DFSDM CHANNELレディ状態 */
#define DFSDM_CHANNEL_STATE_ERROR   0x00080000	/* DFSDM CHANNELエラー状態 */

/*
 *  DFSDM CHANNEL初期化定義
 */
typedef struct
{
	uint32_t                OutClockActivation;		/* アウトプットクロック有効無効 */
	uint32_t                OutClockSelection;		/* アウトプットクロック選択 */
	uint32_t                OutClockDivider;		/* アウトプットクロックデバイダー */
	uint32_t                InputMultiplexer;		/* 入力：外部シリアルまたは内部レジスタ */
	uint32_t                InputDataPacking;		/* 入力：データピッキング */
	uint32_t                InputPins;				/* 入力：ピン */
	uint32_t                SerialType;				/* シリアル種別：SPI or Manchester modes. */
	uint32_t                SerialSpiClock;			/* SPIクロック選択：(external or internal with different sampling point) */
	uint32_t                AwdFilterOrder;			/* アナログウオッチドッグ シンクフィルター順序 */
	uint32_t                AwdOversampling;		/* アナログウオッチドッグ oversampliオーバーサンプリング RATIO */
	int32_t                 Offset;					/* DFSDMチャネルオフセット */
	uint32_t                RightBitShift;			/* DFSDMチャネル右ビットシフト */
}DFSDM_Channel_Init_t;

/*
 *  DFSDM CHANNELハンドラ定義
 */
typedef struct DFSDM_Channel_struct DFSDM_Channel_Handle_t;
struct DFSDM_Channel_struct
{
	uint32_t                base;					/* DFSDMチャネルベースアドレス */
	DFSDM_Channel_Init_t    Init;					/* DFSDM初期化データ */
	void                    (*ckabcallback)(DFSDM_Channel_Handle_t * hdfsc);	/* Channel Ckabコールバック関数 */
	void                    (*scdcallback)(DFSDM_Channel_Handle_t * hdfsc);		/* Channel Scdコールバック関数 */
	volatile uint32_t       state;					/* DFSDMチャネル状態 */
};

/*
 *  DFSDMフィルター状態定義
 */
#define DFSDM_FILTER_STATE_RESET    0x00000000	/* DFSDMフィルターリセット状態 */
#define DFSDM_FILTER_STATE_READY    0x00000001	/* DFSDMフィルターレディ状態 */
#define DFSDM_FILTER_STATE_REG      0x00000002	/* DFSDMフィルターレギュラー変換状態 */
#define DFSDM_FILTER_STATE_INJ      0x00000003	/* DFSDMフィルターインジェクト変換状態 */
#define DFSDM_FILTER_STATE_REG_INJ  0x00000004	/* DFSDMフィルターレギュラー・インジェクト変換状態 */
#define DFSDM_FILTER_STATE_ERROR    0x00080000	/* DFSDMフィルターエラー状態 */

/*
 *  DFSDMフィルターエラー定義e
 */
#define DFSDM_FILTER_ERROR_NONE             0x00000000	/* エラーなし */
#define DFSDM_FILTER_ERROR_REGULAR_OVERRUN  0x00000001	/* レギュラー変換オーバランエラー */
#define DFSDM_FILTER_ERROR_INJECTED_OVERRUN 0x00000002	/* インジェクト変換オーバーランエラー */
#define DFSDM_FILTER_ERROR_DMA              0x00000003	/* DMAエラー */

/*
 *  DFSDMフィルター初期化定義
 */
typedef struct
{
	uint32_t                RegTrigger;				/* レギュラー変換：software or synchronous. */
	uint32_t                RegFastMode;			/* レギュラー変換：fast mode Enable/disable fast mode */
	uint32_t                RegDmaMode;				/* レギュラー変換：DMA Enable/disable */
	uint32_t                InjTrigger;				/* インジェクト変換：Trigger: software, external or synchronous. */
	uint32_t                InjScanMode;			/* インジェクト変換：scanning mode Enable/disable */
	uint32_t                InjDmaMode;				/* インジェクト変換：DMA Enable/disable */
	uint32_t                InjExtTrigger;			/* インジェクト変換：External trigger */
	uint32_t                InjExtTriggerEdge;		/* インジェクト変換：External trigger edge: rising, falling or both. */
	uint32_t                FilterSincOrder;		/* Sinc filter order. */
	uint32_t                FilterOversampling;		/* Filter oversampling ratio. */
	uint32_t                FilterIntOversampling;	/* Integrator oversampling ratio. */
}DFSDM_Filter_Init_t;

/*
 *  DFSDMフィルターハンドラ定義
 */
typedef struct DFSDM_Filter_struct DFSDM_Filter_Handle_t;
struct DFSDM_Filter_struct
{
	uint32_t                base;					/* DFSDMフィルターベースアドレス */
	DFSDM_Filter_Init_t     Init;					/* DFSDMフィルター初期化データ */
	DMA_Handle_t            *hdmaReg;				/* レギュラー変換用DMAハンドラへのポインタ */
	DMA_Handle_t            *hdmaInj;				/* インジェクト変換用DMAハンドラへのポインタ */
	uint32_t                RegularContMode;		/* レギュラー変換：continuous mode */
	uint32_t                InjectedChannelsNbr;	/* インジェクト変換：Number of channels */
	uint32_t                InjConvRemaining;		/* インジェクト変換：remaining */
	void                    (*regconvcallback)(DFSDM_Filter_Handle_t * hdfsf);		/* RegConvコールバック関数 */
	void                    (*regconvhalfcallback)(DFSDM_Filter_Handle_t * hdfsf);	/* RegConv halfコールバック関数 */
	void                    (*injconvcallback)(DFSDM_Filter_Handle_t * hdfsf);		/* InjConvコールバック関数 */
	void                    (*injconvhalfcallback)(DFSDM_Filter_Handle_t * hdfsf);	/* InjConv halfコールバック関数 */
	void                    (*awdconvcallback)(DFSDM_Filter_Handle_t * hdfsf);		/* Analog Wdコールバック関数 */
	void                    (*errorcallback)(DFSDM_Filter_Handle_t * hdfsf);		/* エラーコールバック関数 */
	volatile uint32_t       state;					/* DFSDMフィルター状態 */
	volatile uint32_t       errorcode;				/* DFSDMフィルターエラー */
};

/*
 *  DFSDMフィルター analog watchdog parameters定義
 */
typedef struct
{
	uint32_t                DataSource;				/* データソース：digital filter or from channel watchdog filter. */
	uint32_t                Channel;				/* チャネル選択. */
	int32_t                 HighThreshold;			/* ハイスレッド設定 */
	int32_t                 LowThreshold;			/* ロースレッド設定 */
	uint32_t                HighBreakSignal;		/* ハイスレッド・ブレークシグナル */
	uint32_t                LowBreakSignal;			/* ロースレッド・ブレークシグナル */
} DFSDM_Filter_AwdParamTypeDef;


/*
 *  DFSDMイネーブル・ディーブル定義
 */
#define DFSDM_DISABLE               0x00000000
#define DFSDM_ENABLE                0x00000001
#define DFSDM_MASK                  (~DFSDM_ENABLE)

/*
 *  DFSDMチャネル OutClockSelection定義
 */
#define DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM   0x00000000				/* ソース：ouput clock is system clock */
#define DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO    DFSDM_CHCFGR1_CKOUTSRC  /* ソース：ouput clock is audio clock */

/*
 *  DFSDMチャネル InputMultiplexer定義
 */
#define DFSDM_CHANNEL_EXTERNAL_INPUTS       0x00000000				/* 外部インプット */
#define DFSDM_CHANNEL_INTERNAL_REGISTER     DFSDM_CHCFGR1_DATMPX_1	/* 内部レジスタ */

/*
 *  DFSDMチャネル InputDataPacking定義
 */
#define DFSDM_CHANNEL_STANDARD_MODE         0x00000000				/* Standard data packing mode */
#define DFSDM_CHANNEL_INTERLEAVED_MODE      DFSDM_CHCFGR1_DATPACK_0	/* Interleaved data packing mode */
#define DFSDM_CHANNEL_DUAL_MODE             DFSDM_CHCFGR1_DATPACK_1	/* Dual data packing mode */

/*
 *  DFSDMチャネル InputPins定義
 */
#define DFSDM_CHANNEL_SAME_CHANNEL_PINS     0x00000000				/* インプットピン：pins on same channel */
#define DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS DFSDM_CHCFGR1_CHINSEL	/* インプットピン：following channel */

/*
 *  DFSDMチャネル SerialType定義
 */
#define DFSDM_CHANNEL_SPI_RISING            0x00000000				/* SPI with rising edge */
#define DFSDM_CHANNEL_SPI_FALLING           DFSDM_CHCFGR1_SITP_0	/* SPI with falling edge */
#define DFSDM_CHANNEL_MANCHESTER_RISING     DFSDM_CHCFGR1_SITP_1	/* Manchester with rising edge */
#define DFSDM_CHANNEL_MANCHESTER_FALLING    DFSDM_CHCFGR1_SITP		/* Manchester with falling edge */

/*
 *  DFSDMチャネル SerialSpi clock selection定義
 */
#define DFSDM_CHANNEL_SPI_CLOCK_EXTERNAL    0x00000000				/* 外部SPIクロック */
#define DFSDM_CHANNEL_SPI_CLOCK_INTERNAL    DFSDM_CHCFGR1_SPICKSEL_0			/* 内部SPIクロック */
#define DFSDM_CHANNEL_SPI_CLOCK_INTERNAL_DIV2_FALLING DFSDM_CHCFGR1_SPICKSEL_1	/* 内部SPIクロック divided by 2, falling edge */
#define DFSDM_CHANNEL_SPI_CLOCK_INTERNAL_DIV2_RISING  DFSDM_CHCFGR1_SPICKSEL	/* 内部SPIクロック divided by 2, rising edge */

/*
 *  DFSDMチャネル analog watchdog filter order定義
 */
#define DFSDM_CHANNEL_FASTSINC_ORDER        0x00000000				/* FastSinc filter type */
#define DFSDM_CHANNEL_SINC1_ORDER           DFSDM_CHAWSCDR_AWFORD_0	/* Sinc 1 filter type */
#define DFSDM_CHANNEL_SINC2_ORDER           DFSDM_CHAWSCDR_AWFORD_1	/* Sinc 2 filter type */
#define DFSDM_CHANNEL_SINC3_ORDER           DFSDM_CHAWSCDR_AWFORD	/* Sinc 3 filter type */


/*
 *  DFSDMフィルター Regular/Injected conversion trigger定義
 */
#define DFSDM_FILTER_SW_TRIGGER             0x00000000			/* Software trigger */
#define DFSDM_FILTER_SYNC_TRIGGER           0x00000001			/* Synchronous with DFSDM_FLT0 */
#define DFSDM_FILTER_EXT_TRIGGER            0x00000002			/* External trigger (only for injected conversion) */

/*
 *  DFSDMフィルター external trigger定義(DFSDM filter 0, 1, 2 and 3)
 */
#define DFSDM_FILTER_EXT_TRIG_TIM1_TRGO     0x00000000
#define DFSDM_FILTER_EXT_TRIG_TIM1_TRGO2    DFSDM_FLTCR1_JEXTSEL_0
#define DFSDM_FILTER_EXT_TRIG_TIM8_TRGO     DFSDM_FLTCR1_JEXTSEL_1
#define DFSDM_FILTER_EXT_TRIG_TIM8_TRGO2    (DFSDM_FLTCR1_JEXTSEL_0 | DFSDM_FLTCR1_JEXTSEL_1)
#define DFSDM_FILTER_EXT_TRIG_TIM3_TRGO     DFSDM_FLTCR1_JEXTSEL_2
#define DFSDM_FILTER_EXT_TRIG_TIM4_TRGO     (DFSDM_FLTCR1_JEXTSEL_0 | DFSDM_FLTCR1_JEXTSEL_2)
#define DFSDM_FILTER_EXT_TRIG_TIM10_OC1     (DFSDM_FLTCR1_JEXTSEL_1 | DFSDM_FLTCR1_JEXTSEL_2)
#define DFSDM_FILTER_EXT_TRIG_TIM6_TRGO     (DFSDM_FLTCR1_JEXTSEL_0 | DFSDM_FLTCR1_JEXTSEL_1 | DFSDM_FLTCR1_JEXTSEL_2)
#define DFSDM_FILTER_EXT_TRIG_TIM7_TRGO     DFSDM_FLTCR1_JEXTSEL_3
#define DFSDM_FILTER_EXT_TRIG_EXTI11        (DFSDM_FLTCR1_JEXTSEL_3 | DFSDM_FLTCR1_JEXTSEL_4)
#define DFSDM_FILTER_EXT_TRIG_EXTI15        (DFSDM_FLTCR1_JEXTSEL_0 | DFSDM_FLTCR1_JEXTSEL_3 | DFSDM_FLTCR1_JEXTSEL_4)							/* For DFSDM filter 0, 1, 2 and 3 */
#define DFSDM_FILTER_EXT_TRIG_LPTIM1_OUT    (DFSDM_FLTCR1_JEXTSEL_1 | DFSDM_FLTCR1_JEXTSEL_3 | DFSDM_FLTCR1_JEXTSEL_4)

/*
 *  DFSDMフィルター external trigger edge定義
 */
#define DFSDM_FILTER_EXT_TRIG_RISING_EDGE   DFSDM_FLTCR1_JEXTEN_0	/* 外部 立上がりエッジ */
#define DFSDM_FILTER_EXT_TRIG_FALLING_EDGE  DFSDM_FLTCR1_JEXTEN_1	/* 外部 立下りエッジ */
#define DFSDM_FILTER_EXT_TRIG_BOTH_EDGES    DFSDM_FLTCR1_JEXTEN		/* 外部 両エッジ */

/*
 *  DFSDMフィルター sinc order定義
 */
#define DFSDM_FILTER_FASTSINC_ORDER         0x00000000							/* FastSinc filter type */
#define DFSDM_FILTER_SINC1_ORDER            DFSDM_FLTFCR_FORD_0					/* Sinc 1 filter type */
#define DFSDM_FILTER_SINC2_ORDER            DFSDM_FLTFCR_FORD_1					/* Sinc 2 filter type */
#define DFSDM_FILTER_SINC3_ORDER            (DFSDM_FLTFCR_FORD_0 | DFSDM_FLTFCR_FORD_1)	/* Sinc 3 filter type */
#define DFSDM_FILTER_SINC4_ORDER            DFSDM_FLTFCR_FORD_2					/* Sinc 4 filter type */
#define DFSDM_FILTER_SINC5_ORDER            (DFSDM_FLTFCR_FORD_0 | DFSDM_FLTFCR_FORD_2)	/* Sinc 5 filter type */

/*
 *  DFSDMフィルター AwdDataSource DataSource定義
 */
#define DFSDM_FILTER_AWD_FILTER_DATA        0x00000000			/* デジタルフィルターから入力 */
#define DFSDM_FILTER_AWD_CHANNEL_DATA       DFSDM_FLTCR1_AWFSEL	/* アナログウオッチドッグチャネルから入力 */

/*
 *  DFSDMチャネル定義
 */
#define DFSDM_CHANNEL_0                     0x00000001
#define DFSDM_CHANNEL_1                     0x00010002
#define DFSDM_CHANNEL_2                     0x00020004
#define DFSDM_CHANNEL_3                     0x00030008
#define DFSDM_CHANNEL_4                     0x00040010
#define DFSDM_CHANNEL_5                     0x00050020
#define DFSDM_CHANNEL_6                     0x00060040
#define DFSDM_CHANNEL_7                     0x00070080

/*
 *  DFSDMフィルター Continuous Mode定義
 */
#define DFSDM_CONTINUOUS_CONV_OFF           0x00000000	/* 変換を継続しない */
#define DFSDM_CONTINUOUS_CONV_ON            0x00000001	/* 変換を継続する */

/*
 *  DFSDMフィルター AwdThreshold定義
 */
#define DFSDM_AWD_HIGH_THRESHOLD            0x00000000	/* アナログウオッチドッグ ハイスレシュホールド */
#define DFSDM_AWD_LOW_THRESHOLD             0x00000001	/* アナログウオッチドッグ ロースレシュホールド */


extern void dfsdm_clockconfig(ID port, uint32_t AudioFreq);
extern ER dfsdm_channel_init(DFSDM_Channel_Handle_t *hdfsc);
extern ER dfsdm_channel_deinit(DFSDM_Channel_Handle_t *hdfsc);

extern ER dfsdm_channelCkabStart(DFSDM_Channel_Handle_t *hdfsc);
extern ER dfsdm_channelCkabStop(DFSDM_Channel_Handle_t *hdfsc);
extern ER dfsdm_channelScdStart(DFSDM_Channel_Handle_t *hdfsc, uint32_t Threshold, uint32_t BreakSignal);
extern ER dfsdm_channelScdStop(DFSDM_Channel_Handle_t *hdfsc);
extern int16_t dfsdm_channelGetAwdValue(DFSDM_Channel_Handle_t *hdfsc);
extern ER dfsdm_channelModifyOffset(DFSDM_Channel_Handle_t *hdfsc, int32_t Offset);


extern ER dfsdm_filter_init(DFSDM_Filter_Handle_t *hdfsf);
extern ER dfsdm_filter_deinit(DFSDM_Filter_Handle_t *hdfsf);
extern ER dfsdm_filter_config_reg(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel, uint32_t ContinuousMode);
extern ER dfsdm_filter_config_inj(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel);

extern ER dfsdm_filterRegularStart(DFSDM_Filter_Handle_t *hdfsf, int32_t *pData, uint32_t Length);
extern ER dfsdm_filterRegularMsbStart(DFSDM_Filter_Handle_t *hdfsf, int16_t *pData, uint32_t Length);
extern ER dfsdm_filterRegularStop(DFSDM_Filter_Handle_t *hdfsf);
extern ER dfsdm_filterInjectedStart(DFSDM_Filter_Handle_t *hdfsf, int32_t *pData, uint32_t Length);
extern ER dfsdm_filterInjectedMsbStart(DFSDM_Filter_Handle_t *hdfsf, int16_t *pData, uint32_t Length);
extern ER dfsdm_filterInjectedStop(DFSDM_Filter_Handle_t *hdfsf);
extern ER dfsdm_filterAwdStart(DFSDM_Filter_Handle_t *hdfsf, DFSDM_Filter_AwdParamTypeDef* awdParam);
extern ER dfsdm_filterAwdStop(DFSDM_Filter_Handle_t *hdfsf);
extern ER dfsdm_filterExdStart(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel);
extern ER dfsdm_filterExdStop(DFSDM_Filter_Handle_t *hdfsf);

extern int32_t  dfsdm_filterGetRegularValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t* Channel);
extern int32_t  dfsdm_filterGetInjectedValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t* Channel);
extern int32_t  dfsdm_filterGetExdMaxValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t* Channel);
extern int32_t  dfsdm_filterGetExdMinValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t* Channel);
extern uint32_t dfsdm_filterGetConvTimeValue(DFSDM_Filter_Handle_t *hdfsf);

extern void dfsdm_irqhandler(DFSDM_Filter_Handle_t *hdfsf);


#ifdef __cplusplus
}
#endif

#endif	/* _DFSDM_H_ */

