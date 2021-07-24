/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の (1)〜(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
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
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: sdcard.h,v 1.0 2016/01/17 21:31:56 roi Exp $
 */
/*
 *  STM32F746用 SDMMCドライバインクルードファイル
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#ifdef __cplusplus
 extern "C" {
#endif


/*
 *  SDカード設定
 */
#ifdef TOPPERS_STM32F769_DISCOVERY
#define INHNO_SDMMC     IRQ_VECTOR_SDMMC2	/* 割込みハンドラ番号 */
#define INTNO_SDMMC     IRQ_VECTOR_SDMMC2	/* 割込み番号 */
#define INTPRI_SDMMC    -5          /* 割込み優先度 */
#define INTATR_SDMMC    TA_EDGE     /* 割込み属性 */

#define INHNO_DMARX     IRQ_VECTOR_DMA2_STREAM0	/* 割込みハンドラ番号 */
#define INTNO_DMARX     IRQ_VECTOR_DMA2_STREAM0	/* 割込み番号 */
#define INTPRI_DMARX    -6          /* 割込み優先度 */
#define INTATR_DMARX    TA_EDGE     /* 割込み属性 */
#define DMARX_SID       DMA2STM0_SID

#define INHNO_DMATX     IRQ_VECTOR_DMA2_STREAM5	/* 割込みハンドラ番号 */
#define INTNO_DMATX     IRQ_VECTOR_DMA2_STREAM5	/* 割込み番号 */
#define INTPRI_DMATX    -6          /* 割込み優先度 */
#define INTATR_DMATX    TA_EDGE     /* 割込み属性 */
#define DMATX_SID       DMA2STM5_SID
#else
#define INHNO_SDMMC     IRQ_VECTOR_SDMMC	/* 割込みハンドラ番号 */
#define INTNO_SDMMC     IRQ_VECTOR_SDMMC	/* 割込み番号 */
#define INTPRI_SDMMC    -5          /* 割込み優先度 */
#define INTATR_SDMMC    TA_EDGE     /* 割込み属性 */

#define INHNO_DMARX     IRQ_VECTOR_DMA2_STREAM3	/* 割込みハンドラ番号 */
#define INTNO_DMARX     IRQ_VECTOR_DMA2_STREAM3	/* 割込み番号 */
#define INTPRI_DMARX    -6          /* 割込み優先度 */
#define INTATR_DMARX    TA_EDGE     /* 割込み属性 */
#define DMARX_SID       DMA2STM3_SID

#define INHNO_DMATX     IRQ_VECTOR_DMA2_STREAM6	/* 割込みハンドラ番号 */
#define INTNO_DMATX     IRQ_VECTOR_DMA2_STREAM6	/* 割込み番号 */
#define INTPRI_DMATX    -6          /* 割込み優先度 */
#define INTATR_DMATX    TA_EDGE     /* 割込み属性 */
#define DMATX_SID       DMA2STM6_SID
#endif


#define CMD_SHORTRESP       (1<<6)	/* short response */
#define CMD_LONGRESP        (1<<7)	/* long response */
#define CMD_SLONGRESP       (CMD_SHORTRESP|CMD_LONGRESP)
#define CMD_TIMEOUT         (1<<8)	/* timeout request bit */
#define CMD_SETLAST         (1<<10)
#define CMD_MASK            0xFFFF
#define CMD_IGNOREIRES      (1<<16)
#define CMD_INDEX           0x003F

/*
 *  SD/MMC コマンドリスト
 */
#define MCI_CMD0			(0x0000)								/* GO_IDLE_STATE(MMC) or RESET(SD) */
#define MCI_CMD1			(0x0001|CMD_SHORTRESP|CMD_IGNOREIRES)	/* SEND_OP_COND(MMC) or CMD1(SD) */
#define MCI_CMD2			(0x0002|CMD_SLONGRESP|CMD_IGNOREIRES)	/* ALL SEND_CID */
#define MCI_CMD3			(0x0003|CMD_SHORTRESP)					/* SET_RELATE_ADDR */
#define MCI_CMD6			(0x0006|CMD_SHORTRESP)
#define MCI_CMD7			(0x0007|CMD_SHORTRESP)					/* SELECT/DESELECT_CARD */
#define MCI_CMD8            (0x0008|CMD_SHORTRESP)					/* HS_SEND_EXT_CSD */
#define MCI_CMD9			(0x0009|CMD_SLONGRESP|CMD_IGNOREIRES)	/* SEND_CSD */
#define MCI_CMD12			(0x000C|CMD_SHORTRESP)					/* Stop either READ or WRITE operation */
#define MCI_CMD13			(0x000D|CMD_SHORTRESP)					/* SEND_STATUS */
#define MCI_CMD16			(0x0010|CMD_SHORTRESP)					/* SET_BLOCK_LEN */
#define MCI_CMD17			(0x0011|CMD_SHORTRESP)					/* READ_SINGLE_BLOCK */
#define MCI_CMD18			(0x0012|CMD_SHORTRESP)					/* READ_MULTI_BLOCK */
#define MCI_CMD24			(0x0018|CMD_SHORTRESP)					/* WRITE_SINGLE_BLOCK */
#define MCI_CMD25			(0x0019|CMD_SHORTRESP)					/* WRITE_MULTI_BLOCK */
#define MCI_CMD32			(0x0020|CMD_SHORTRESP)					/* ERASE_GRP_START */
#define MCI_CMD33			(0x0021|CMD_SHORTRESP)					/* ERASE_GRP_END */
#define MCI_CMD38			(0x0026|CMD_SHORTRESP)					/* ERASE */
#define MCI_CMD55			(0x0037|CMD_SHORTRESP)					/* APP_CMD, the following will a ACMD */

#define MCI_ACMD6			(0x0006|CMD_SHORTRESP)					/* ACMD6 for SD card BUSWIDTH */
#define MCI_ACMD13			(0x000D|CMD_SHORTRESP)					/* ACMD23 for SD card status */
#define MCI_ACMD41			(0x0029|CMD_SHORTRESP|CMD_IGNOREIRES)	/* ACMD41 for SD card */
#define MCI_ACMD51			(0x0033|CMD_SHORTRESP)					/* ACMD41 for SD SCR */

#define RES_CMD1            (MCI_CMD1 & CMD_INDEX)
#define RES_CMD12           (MCI_CMD12 & CMD_INDEX)
#define RES_ACMD41          (MCI_ACMD41 & CMD_INDEX)

#define OCR_INDEX			0x00FF8000

#define CARD_STATUS_ACMD_ENABLE		(1<<5)
#define CARD_STATUS_RDY_DATA		(1<<8)
#define CARD_STATUS_CURRENT_STATE	(0x0F<<9)
#define CARD_STATUS_ERASE_RESET		(1<<13)

#define SLOW_RATE			1
#define NORMAL_RATE			2

#define SD_1_BIT 			0
#define SD_4_BIT			1

#define SD_CARD_V11         0
#define SD_CARD_V20         1
#define SD_CARD_HC          2
#define MMC_CARD			3
#define SD_IO_CARD          4
#define HS_MM_CARD          5
#define SD_IO_COMBO_CARD    6
#define MMC_CARD_HC         7


#define MCI_NOTEND          0
#define MCI_END             1
#define MCI_ERREND          2

#define SDMODE_PROTECT      (1<<5)

/*
 *  SDMMC拡張エラー定義
 */
#define E_SDCOM             (-80)	/* コマンドエラー */
#define E_SDCRC             (-81)	/* CRCエラー */
#define E_SDECMD            (-82)	/* コマンドインデックスエラー */
#define E_SDVOL             (-83)	/* 電圧エラー */
#define E_SDTRS             (-84)	/* 通信エラー */

/*
 *  カード転送状態定義
 */
#define SD_TRANSFER_OK      0	/* 転送成功 */
#define SD_TRANSFER_BUSY    1	/* 転送中 */
#define SD_TRANSFER_ERROR   2	/* 転送エラー */


/*
 *  バス幅定義
 */
#define SDMMC_BUS_WIDE_1B   0x00000000
#define SDMMC_BUS_WIDE_4B   SDMMC_CLKCR_WIDBUS_0
#define SDMMC_BUS_WIDE_8B   SDMMC_CLKCR_WIDBUS_1


/*
 *  SDMMC用データブロックサイズ定義
 */
#define SDMMC_DBSIZE_1B     0x00000000
#define SDMMC_DBSIZE_2B     SDMMC_DCTRL_DBLOCKSIZE_0
#define SDMMC_DBSIZE_4B     SDMMC_DCTRL_DBLOCKSIZE_1
#define SDMMC_DBSIZE_8B     (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1)
#define SDMMC_DBSIZE_16B    SDMMC_DCTRL_DBLOCKSIZE_2
#define SDMMC_DBSIZE_32B    (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DBSIZE_64B    (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DBSIZE_128B   (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DBSIZE_256B   SDMMC_DCTRL_DBLOCKSIZE_3
#define SDMMC_DBSIZE_512B   (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DBSIZE_1024B  (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DBSIZE_2048B  (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_3) 
#define SDMMC_DBSIZE_4096B  (SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DBSIZE_8192B  (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DBSIZE_16384B (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)

/*
 *  SDMMC初期設定周波数
 */
#define SDMMC_INIT_CLK_DIV  0x76

/*
 *  SDMMCデータ転送用周波数
 */
#define SDMMC_TRANSFER_CLK_DIV 0x0


/*
 *  SDMMCハンドラ構造体定義
 */

typedef struct
{
	uint32_t                base;		/* SDMMCレジスタベースアドレス */
	uint32_t                ClockMode;	/* 指定クロックモード */
	uint32_t                BusWide;	/* 指定バス幅 */
	uint32_t                RetryCount;	/* リトライ回数 */
	uint32_t                cardtype;	/* カードタイプ */
	uint32_t                RCA;		/* RCA値 */
	uint32_t                CSD[4];		/* CSD値 */
	uint32_t                CID[4];		/* CID値 */
	volatile uint32_t       status;		/* 転送状態 */
	volatile uint32_t       SdCmd;		/* 転送指定コマンド */
	DMA_Handle_t            *hdmarx;	/* 受信用DMAハンドラ */
	DMA_Handle_t            *hdmatx;	/* 送信用DMAハンドラ */
}SDMMC_Handle_t;

/*
 *  SDMMCカード情報構造体
 */
typedef struct
{
	uint64_t                capacity;	/* 容量(バイト) */
	uint32_t                blocksize;	/* ブロックサイズ */
	uint32_t                maxsector;	/* 最大セクタ数 */
	uint16_t                RCA;		/* SD RCA */
	uint8_t                 cardtype;	/* カード種類 */
	uint8_t                 status;		/* ステータス */

}SDMMC_CardInfo_t;


/*
 *  CMD13カード状態
 */
#define SD_CARD_READY           0x00000001	/* カードレディ状態 */
#define SD_CARD_IDENTIFICATION	0x00000002	/* カードアイデンティフィケーション状態 */
#define SD_CARD_STANDBY         0x00000003	/* カードスタンバィ状態 */
#define SD_CARD_TRANSFER        0x00000004	/* カード転送状態 */
#define SD_CARD_SENDING         0x00000005	/* カード送信状態 */
#define SD_CARD_RECEIVING       0x00000006	/* カード受信状態 */
#define SD_CARD_PROGRAMMING     0x00000007	/* カードプログラミング状態 */
#define SD_CARD_DISCONNECTED    0x00000008	/* カードディスコネクト状態 */
#define SD_CARD_ERROR           0x000000FF	/* カードエラー状態 */


extern void sdmmc_init(intptr_t exinf);
extern bool_t sdmmc_sense(int id);
extern SDMMC_Handle_t *sdmmc_open(int id);
extern ER sdmmc_close(SDMMC_Handle_t *hsd);
extern ER sdmmc_erase(SDMMC_Handle_t *hsd, uint64_t startaddr, uint64_t endaddr);
extern ER sdmmc_blockread(SDMMC_Handle_t *hsd, uint32_t *pbuf, uint64_t ReadAddr, uint32_t blocksize, uint32_t num);
extern ER sdmmc_blockwrite(SDMMC_Handle_t *hsd, uint32_t *pbuf, uint64_t WriteAddr, uint32_t blocksize, uint32_t num);
extern ER sdmmc_wait_transfar(SDMMC_Handle_t *hsd, uint32_t Timeout);
extern void sdmmc_checkint(SDMMC_Handle_t *hsd);

extern ER sdmmc_getcardinfo(SDMMC_Handle_t *hsd, SDMMC_CardInfo_t *pCardInfo);
extern ER sdmmc_checkCID(SDMMC_Handle_t *hsd);
extern ER sdmmc_setaddress(SDMMC_Handle_t *hsd);
extern ER sdmmc_sendCSD(SDMMC_Handle_t *hsd);
extern ER sdmmc_select_card(SDMMC_Handle_t *hsd, uint64_t addr);
extern ER sdmmc_configuration(SDMMC_Handle_t *hsd);
extern ER sdmmc_set_widebus(SDMMC_Handle_t *hsd);
extern uint32_t sdmmc_getstatus(SDMMC_Handle_t *hsd);


#ifdef __cplusplus
}
#endif


#endif /* _SDMMC_H_ */

