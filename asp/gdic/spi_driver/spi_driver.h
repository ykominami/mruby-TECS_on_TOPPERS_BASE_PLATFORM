/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: spi_driver.h 2416 2016-02-12 10:32:30Z roi $
 */

/*
 *	SPIドライバのヘッダファイル
 */

#ifndef _SPI_DRIVER_H_
#define _SPI_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "device.h"
#include "spi.h"
#include "pinmode.h"

/*
 *  SDカード種別定義
 */
#define SD_CARD_V11         0
#define SD_CARD_V20         1
#define SD_CARD_HC          2
#define MMC_CARD			3
#define SD_IO_CARD          4
#define HS_MM_CARD          5
#define SD_IO_COMBO_CARD    6
#define MMC_CARD_HC         7

#define PORT_HIGH           1
#define PORT_LOW            0

#define SDMODE_PROTECT      (1<<5)
#define BUFFERSIZE          140


#ifndef TOPPERS_MACRO_ONLY

/*
 *  SPI SETUP構造体定義
 */
typedef struct {
	int                     port;
	uint32_t                baud;
	ID                      spi_sem;
	uint8_t                 cs_pin;
	uint8_t                 rs_pin;
	uint8_t                 rst_pin;
	uint8_t                 cs2_pin;
	uint32_t                otype;
	uint32_t                pullmode;
} SPI_Setup_t;

/*
 *  SDCARDハンドラ構造体定義
 */
typedef struct {
	SPI_Handle_t            *hspi;		/* spiハンドラ */
	uint32_t                RetryCount;	/* リトライ回数 */
	uint32_t                cardtype;	/* カードタイプ */
	uint32_t                RCA;		/* RCA値 */
	uint8_t                 CSD[16];	/* CSD値 */
	uint8_t                 CID[16];	/* CID値 */
	ID                      spi_lock;
	volatile uint32_t       status;		/* 転送状態 */
	uint32_t                read_count;
	uint32_t                write_count;
	uint32_t                read_retry_count;
	uint32_t                write_retry_count;
	uint32_t                read_error;
	uint32_t                write_error;
}SDCARD_Handler_t;

/*
 *  SDCARD情報構造体
 */
typedef struct {
	uint64_t                capacity;	/* 容量(バイト) */
	uint32_t                blocksize;	/* ブロックサイズ */
	uint32_t                maxsector;	/* 最大セクタ数 */
	uint16_t                RCA;		/* SD RCA */
	uint8_t                 cardtype;	/* カード種類 */
	uint8_t                 status;		/* ステータス */
}SDCARD_CardInfo_t;


/*
 *  関数のプロトタイプ宣言
 */

extern SPI_Handle_t *spi_start(SPI_Setup_t *psps);
extern void spi_end(SPI_Handle_t *hspi);

extern void sdcard_setspi(ID portid, SPI_Handle_t *hspi);
extern bool_t sdcard_init(ID portid);
extern SDCARD_Handler_t *sdcard_open(int id);
extern ER sdcard_close(SDCARD_Handler_t *hsd);
extern ER sdcard_getcardinfo(SDCARD_Handler_t *hsd, SDCARD_CardInfo_t *pCardInfo);
extern ER sdcard_checkCID(SDCARD_Handler_t *hsd);
extern ER sdcard_sendCSD(SDCARD_Handler_t *hsd);
extern ER sdcard_select_card(SDCARD_Handler_t *hsd, uint64_t addr);
extern ER sdcard_configuration(SDCARD_Handler_t *hsd);
extern ER sdcard_blockread(SDCARD_Handler_t *hsd, uint32_t *pbuf, uint64_t ReadAddr, uint32_t blocksize, uint32_t num);
extern ER sdcard_blockwrite(SDCARD_Handler_t *hsd, uint32_t *pbuf, uint64_t WriteAddr, uint32_t blocksize, uint32_t num);
extern ER sdcard_wait_transfar(SDCARD_Handler_t *hsd, uint32_t Timeout);
extern ER sdcard_dummy(SDCARD_Handler_t *hsd);

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif

#endif	/* _SPI_DRIVER_H_ */

