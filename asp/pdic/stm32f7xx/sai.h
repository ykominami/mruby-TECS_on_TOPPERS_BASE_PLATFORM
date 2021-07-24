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
 *  @(#) $Id: sai.h 698 2016-03-26 16:37:57Z roi $
 */
/*
 * 
 *  STM32F7xx SAIデバイスドライバの外部宣言
 *
 */

#ifndef _SAI_H_
#define _SAI_H_

#ifdef __cplusplus
 extern "C" {
#endif


/*
 *  AUDIOポート定義
 */
#define AUDIO1_PORTID           1
#define AUDIO2_PORTID           2
#define NUM_AUDIOPORT           2

/*
 *  AUDIOハードウェア設定構造体
 */
typedef struct _AUDIO_PortControlBlock{
	uint32_t                base;
	uint32_t                outgioclockbase;
	uint32_t                outgioclockbit;
	uint32_t                ingioclockbase;
	uint32_t                ingioclockbit;
	uint32_t                intgioclockbase;
	uint32_t                intgioclockbit;
	uint32_t                outdmaclockbase;
	uint32_t                outdmaclockbit;
	uint32_t                indmaclockbase;
	uint32_t                indmaclockbit;
	uint32_t                audioclockbase;
	uint32_t                audioclockbit;

	uint32_t                giofsobase;
	uint32_t                giomcobase;
	uint32_t                giosdibase;
	uint32_t                giointbase;
	uint32_t                outdmabase;
	uint32_t                outdmachannel;
	uint32_t                indmabase;
	uint32_t                indmachannel;
	uint8_t                 outmsckpin;
	uint8_t                 outfspin;
	uint8_t                 outsckin;
	uint8_t                 outsdpin;
	uint8_t                 insdpin;
	uint8_t                 inintpin;
	uint8_t                 outaf;
	uint8_t                 inaf;
} AUDIO_PortControlBlock;

/*
 *  AUDIO初期化構造体 
 */
typedef struct {
	uint32_t                AudioMode;			/* AUDIOモード */
	uint32_t                Synchro;			/* SAIブロックシンクロ値 */
	uint32_t                SynchroExt;			/* SAIブロックシンクロ拡張値 */
	uint32_t                OutputDrive;		/* 出力デバイス */
    uint32_t                NoDivider;			/* DIVIDER有効/無効 */
	uint32_t                FIFOThreshold;		/* FIFOスレッシュホールド設定 */
	uint32_t                AudioFrequency;		/* AUDIO サンプリング周波数 */
	uint32_t                Mckdiv;				/* マスタークロックDIVIDER値 */
	uint32_t                MonoStereoMode;		/* モノクロ/ステレオモード */
	uint32_t                CompandingMode;		/* コンパウンディングモード */
	uint32_t                TriState;           /* TRIStateマネージメント設定 */
	uint32_t                Protocol;			/* SAI BLOCKプロトコル */
	uint32_t                DataSize;			/* SAI BLOCKデータサイズ */
	uint32_t                FirstBit;			/* MSB/LSB設定 */
	uint32_t                ClockStrobing;		/* クロック・ストロービング設定 */

	/*
	 *  フレームデータ
	 */
	uint32_t                FrameLength;		/* フレーム長 */
	uint32_t                ActiveFrameLength;	/* アクティブ・フレーム長 */
	uint32_t                FSDefinition;		/* フレーム・シンクロナス定義 */
	uint32_t                FSPolarity;         /* FS POLARITY設定 */
	uint32_t                FSOffset;           /* FS OFFSET設定 */

	/*
	 *  スロット定義
	 */
	uint32_t                FirstBitOffset;		/* スロット中の最初のデータビット位置 */
	uint32_t                SlotSize;			/* スロットサイズ */
	uint32_t                SlotNumber;			/* スロット番号 */
	uint32_t                SlotActive;			/* SLOTアクティブ設定 */
}SAI_Init_t;

/*
 *  AUDIOハンドラ定義
 */
typedef struct _AUDIO_HandleStruct AUDIO_Handle_t;
struct _AUDIO_HandleStruct
{
	uint32_t                base;			/* SAIデバイスのベースアドレス */
	const AUDIO_PortControlBlock *pcb;		/* ハードウエア定義構造体へのポインタ */
	SAI_Init_t              OutInit;		/* 出力ブロック初期設定 */
	SAI_Init_t              InInit;			/* 入力ブロック初期設定 */
	uint8_t                 *pBuffPtr;		/* データ領域へのポインタ */
	uint16_t                XferSize;		/* データサイズ */
	uint16_t                XferCount;		/* 送受信カウント */
	DMA_Handle_t            *hdmatx;		/* 送信DMAハンドラへのポインタ */
	DMA_Handle_t            *hdmarx;		/* 受信DMAハンドラへのポインタ */
    void        (*audiomutecallback)(AUDIO_Handle_t *haudio, uint32_t mode); 	/* MUTEコールバック関数 */
    void        (*audioerrorcallback)(AUDIO_Handle_t *haudio, uint32_t mode); 	/* 割込みエラーコールバック関数 */
	void        (*transcallback)(AUDIO_Handle_t *haudio);		/* 送信終了コールバック関数 */
	void        (*transhalfcallback)(AUDIO_Handle_t *haudio);	/* 送信終了コールバック関数 */
	void        (*recevcallback)(AUDIO_Handle_t *haudio);		/* 受信終了コールバック関数 */
	void        (*recevhalfcallback)(AUDIO_Handle_t *haudio);	/* 受信終了コールバック関数 */
	void        (*errorcallback)(AUDIO_Handle_t *haudio);		/* エラーコールバック関数 */
    volatile uint32_t       status[2];		/* AUDIOステータス */
    volatile uint32_t       ErrorCode;		/* エラーコード */
};


/*
 *  AUDIO設定モード定義
 */
#define AUDIO_OUT_BLOCK         0
#define AUDIO_IN_BLOCK          1

/*
 *  AUDIOステータス定義
 */
#define AUDIO_STATUS_RESET      0x00		/* AUDIOデバイスリセット状態 */
#define AUDIO_STATUS_READY      0x01		/* AUDIOデバイスレディ状態 */
#define AUDIO_STATUS_BUSY       0x02		/* AUDIOデバイスビジィー状態 */
#define AUDIO_STATUS_BUSY_TX    0x12		/* AUDIOデバイス送信ビジィー状態 */
#define AUDIO_STATUS_BUSY_RX    0x22		/* AUDIOデバイス受信ビジィー状態 */
#define AUDIO_STATUS_TIMEOUT    0x03		/* AUDIOデバイスタイムアウト状態 */
#define AUDIO_STATUS_ERROR      0x04		/* AUDIOデバイスエラー状態 */                                                                        

/*
 *  AUDIOエラー定義
 */
#define AUDIO_ERROR_NONE        0x00000000		/* エラーなし */
#define AUDIO_ERROR_OVRUDR      0x00000001		/* オーバーランまたはアンダーラン */
#define AUDIO_ERROR_AFSDET      0x00000004		/* Anticipated Frame synchronisation detection */
#define AUDIO_ERROR_LFSDET      0x00000008		/* Late Frame synchronisation detection        */
#define AUDIO_ERROR_WCKCFG      0x00000010		/* Wrong clock configuration */ 
#define AUDIO_ERROR_TIMEOUT     0x00000040		/* タイムアウト */

/*
 *  SAIブロックシンクロ拡張値(SynchroExt)
 */
#define SAI_SYNCEXT_DISABLE             0x00000000
#define SAI_SYNCEXT_IN_ENABLE           0x00000001
#define SAI_SYNCEXT_OUTBLOCKA_ENABLE    0x00000002
#define SAI_SYNCEXT_OUTBLOCKB_ENABLE    0x00000004

/*
 *  AUDIOモード(AudioMode)
 */
#define SAI_MODEMASTER_TX               0x00000000
#define SAI_MODEMASTER_RX               SAI_xCR1_MODE_0
#define SAI_MODESLAVE_TX                SAI_xCR1_MODE_1
#define SAI_MODESLAVE_RX                (SAI_xCR1_MODE_1 | SAI_xCR1_MODE_0)

/*
 *  SAI BLOCKプロトコル(Protocol)
 */
#define SAI_FREE_PROTOCOL               0x00000000
#define SAI_SPDIF_PROTOCOL              SAI_xCR1_PRTCFG_0
#define SAI_AC97_PROTOCOL               SAI_xCR1_PRTCFG_1

/*
 *  SAI BLOCKデータサイズ(DataSize)
 */
#define SAI_DATASIZE_8                  SAI_xCR1_DS_1
#define SAI_DATASIZE_10                 (SAI_xCR1_DS_1 | SAI_xCR1_DS_0)
#define SAI_DATASIZE_16                 SAI_xCR1_DS_2
#define SAI_DATASIZE_20                 (SAI_xCR1_DS_2 | SAI_xCR1_DS_0)
#define SAI_DATASIZE_24                 (SAI_xCR1_DS_2 | SAI_xCR1_DS_1)
#define SAI_DATASIZE_32                 (SAI_xCR1_DS_2 | SAI_xCR1_DS_1 | SAI_xCR1_DS_0)

/*
 *  MSB/LSB設定(FirstBit)
 */
#define SAI_FIRSTBIT_MSB                0x00000000
#define SAI_FIRSTBIT_LSB                SAI_xCR1_LSBFIRST

/*
 *  クロック・ストロービング設定(ClockStrobing)
 */
#define SAI_CLOCKSTROBING_FALLINGEDGE     ((uint32_t)0x00000000)
#define SAI_CLOCKSTROBING_RISINGEDGE      ((uint32_t)SAI_xCR1_CKSTR)

/*
 *  SAIブロックシンクロ値(Synchro)
 */
#define SAI_ASYNCHRONOUS                0x00000000
#define SAI_SYNCHRONOUS                 SAI_xCR1_SYNCEN_0
#define SAI_SYNCHRONOUS_EXT             SAI_xCR1_SYNCEN_1

/*
 *  出力デバイス(OutputDrive)
 */
#define SAI_OUTPUTDRIVE_DISABLE         0x00000000
#define SAI_OUTPUTDRIVE_ENABLE          SAI_xCR1_OUTDRIV

/*
 *  DIVIDER有効/無効(NoDivider)
 */
#define SAI_MASTERDIVIDER_ENABLE        0x00000000
#define SAI_MASTERDIVIDER_DISABLE       SAI_xCR1_NODIV

/*
 *  フレーム・シンクロナス定義(FSDefinition)
 */
#define SAI_FS_STARTFRAME               0x00000000
#define SAI_FS_CHANNEL_IDENTIFICATION   SAI_xFRCR_FSDEF

/*
 *  FS POLARITY設定(FSPolarity)
 */
#define SAI_FS_ACTIVE_LOW               0x00000000
#define SAI_FS_ACTIVE_HIGH              SAI_xFRCR_FSPO

/*
 *  FS OFFSET設定(FSOffset)
 */
#define SAI_FS_FIRSTBIT                 0x00000000
#define SAI_FS_BEFOREFIRSTBIT           SAI_xFRCR_FSOFF

/*
 *  SLOTサイズ設定(SlotSize)
 */
#define SAI_SLOTSIZE_DATASIZE           0x00000000
#define SAI_SLOTSIZE_16B                SAI_xSLOTR_SLOTSZ_0
#define SAI_SLOTSIZE_32B                SAI_xSLOTR_SLOTSZ_1

/*
 *  SLOTアクティブ設定(SlotActive)
 */
#define SAI_SLOT_NOTACTIVE              0x00000000
#define SAI_SLOTACTIVE_0                0x00010000
#define SAI_SLOTACTIVE_1                0x00020000
#define SAI_SLOTACTIVE_2                0x00040000
#define SAI_SLOTACTIVE_3                0x00080000
#define SAI_SLOTACTIVE_4                0x00100000
#define SAI_SLOTACTIVE_5                0x00200000
#define SAI_SLOTACTIVE_6                0x00400000
#define SAI_SLOTACTIVE_7                0x00800000
#define SAI_SLOTACTIVE_8                0x01000000
#define SAI_SLOTACTIVE_9                0x02000000
#define SAI_SLOTACTIVE_10               0x04000000
#define SAI_SLOTACTIVE_11               0x08000000
#define SAI_SLOTACTIVE_12               0x10000000
#define SAI_SLOTACTIVE_13               0x20000000
#define SAI_SLOTACTIVE_14               0x40000000
#define SAI_SLOTACTIVE_15               0x80000000
#define SAI_SLOTACTIVE_ALL              0xFFFF0000

/*
 *  モノクロ/ステレオモード(MonoStereoMode)
 */
#define SAI_STEREOMODE                  0x00000000
#define SAI_MONOMODE                    SAI_xCR1_MONO

/*
 *  TRIStateマネージメント設定(TriState)
 */
#define SAI_OUTPUT_NOTRELEASED          0x00000000
#define SAI_OUTPUT_RELEASED             SAI_xCR2_TRIS

/*
 *  FIFOスレシュホールド設定(FIFOThreshold)
 */
#define SAI_FIFOTHRESHOLD_EMPTY         0x00000000
#define SAI_FIFOTHRESHOLD_1QF           SAI_xCR2_FTH_0
#define SAI_FIFOTHRESHOLD_HF            SAI_xCR2_FTH_1
#define SAI_FIFOTHRESHOLD_3QF           (SAI_xCR2_FTH_1 | SAI_xCR2_FTH_0)
#define SAI_FIFOTHRESHOLD_FULL          SAI_xCR2_FTH_2

/*
 *  コンパウンディングモード設定(CompandingMode)
 */
#define SAI_NOCOMPANDING                0x00000000
#define SAI_ULAW_1CPL_COMPANDING        SAI_xCR2_COMP_1
#define SAI_ALAW_1CPL_COMPANDING        (SAI_xCR2_COMP_1 | SAI_xCR2_COMP_0)
#define SAI_ULAW_2CPL_COMPANDING        (SAI_xCR2_COMP_1 | SAI_xCR2_CPL)
#define SAI_ALAW_2CPL_COMPANDING        (SAI_xCR2_COMP_1 | SAI_xCR2_COMP_0 | SAI_xCR2_CPL)


extern AUDIO_Handle_t *audio_init(ID id);
extern void audio_deinit(AUDIO_Handle_t *haudio, uint32_t mode);
extern void audio_clockconfig(AUDIO_Handle_t *haudio, uint32_t AudioFreq, void *Params);

extern ER audio_start(AUDIO_Handle_t *haudio, uint32_t mode);
extern ER audio_end(AUDIO_Handle_t *haudio, uint32_t mode);
extern ER audio_transmit(AUDIO_Handle_t *haudio, uint8_t *pData, uint16_t Size);
extern ER audio_receive(AUDIO_Handle_t *haudio, uint8_t *pData, uint16_t Size);

extern ER audio_dmapause(AUDIO_Handle_t *haudio, uint32_t mode);
extern ER audio_dmaresume(AUDIO_Handle_t *haudio, uint32_t mode);
extern ER audio_dmastop(AUDIO_Handle_t *haudio, uint32_t mode);
extern void audio_enable(AUDIO_Handle_t *haudio, uint32_t mode);
extern ER audio_disable(AUDIO_Handle_t *haudio, uint32_t mode);
extern AUDIO_Handle_t *audio_gethandle(ID id);
extern uint32_t audio_status(AUDIO_Handle_t *haudio, uint32_t mode);
extern void audio_irqhandler(AUDIO_Handle_t *haudio);

#ifdef __cplusplus
}
#endif

#endif	/* _SAI_H_ */

