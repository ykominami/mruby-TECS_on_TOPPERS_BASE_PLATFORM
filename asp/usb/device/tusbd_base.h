/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2017 by TOPPERS PROJECT
 *                             Educational Working Group.
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
 *  @(#) $Id: tusbd_base.h 698 2017-10-05 21:31:22Z roi $
 */
/*
 *  USB Device Middleware BASE部定義
 */

#ifndef _TUSBD_BASE_H_
#define _TUSBD_BASE_H_

#include "tusb_rtos.h"
#include "tusb_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef TUSBD_NUM_ENDPOINT
#define TUSBD_NUM_ENDPOINT                    15
#endif

#define USB_REQUEST_TYPE_MASK                 0x60

#define TUSBD_CONFIG_REMOTE_WAKEUP            0x02

#define USB_FEATURE_EP_HALT                   0
#define USB_FEATURE_REMOTE_WAKEUP             1
#define USB_FEATURE_TEST_MODE                 2



#define USB_HS_MAX_PACKET_SIZE                512
#define USB_FS_MAX_PACKET_SIZE                64
#define USB_MAX_EP0_SIZE                      USB_FS_MAX_PACKET_SIZE

/*
 *  デバイス状態定義
 */
#define TUSBD_STATE_RESET                     0		/* 未実行状態 */
#define TUSBD_STATE_INIT                      1		/* 初期化状態 */
#define TUSBD_STATE_ADDRESSED                 2		/* アドレス設定済み状態 */
#define TUSBD_STATE_CONFIGURED                3		/* エナミュレーション終了状態 */
#define TUSBD_STATE_SUSPENDED                 4		/* サスペンド状態 */

/*
 *  エンドポイント#0状態
 */
#define TUSBD_EP0_IDLE                        0		/* アイドル状態 */
#define TUSBD_EP0_SETUP                       1		/* SETUP状態 */
#define TUSBD_EP0_DATAIN                      2		/* DATAIN状態 */
#define TUSBD_EP0_DATAOUT                     3		/* DATAOUT状態 */
#define TUSBD_EP0_STATUSIN                    4		/* STATUSIN状態 */
#define TUSBD_EP0_STATUSOUT                   5		/* STATUSOUT状態 */

/* bmAttributes in configuration descriptor */
/* C_RESERVED must always be set */
#define C_RESERVED                            (1<<7)
#define C_SELF_POWERED                        (1<<6)
#define C_REMOTE_WAKEUP                       (1<<5)

/* bMaxPower in configuration descriptor */
#define C_POWER(mA)                           ((mA)/2)

/*
 *  エンドポイント管理型
 */
typedef struct
{
	uint16_t                   status;
	uint16_t                   maxpacket;
	uint32_t                   xfersize;
	uint32_t                   remlength;
} TUSBD_Endpoint_t;

/*
 *  USBデバイスハンドラ
 */
typedef struct
{
	uint8_t                    devData[32];
	uint8_t                    id;
	uint8_t                    dev_speed;
	uint8_t                    dev_address;
	uint8_t                    dev_config;
	uint8_t                    dev_test_mode;
	uint8_t                    dev_remote_wakeup;
	uint8_t                    ep0_state;
	uint16_t                   ep0_data_len;
	uint8_t                    dev_state;
	uint8_t                    dev_old_state;
	TUSBD_Endpoint_t           ep_in[TUSBD_NUM_ENDPOINT];
	TUSBD_Endpoint_t           ep_out[TUSBD_NUM_ENDPOINT];

	UsbSetupRequest            request;
	void                       *pClassData;
	void                       *pSysData;
	void                       *pUsrData;
	void                       (*makeDescriptor)(uint8_t speed);
} TUSBD_Handle_t;

#include "tusbd_conf.h"

#ifndef MAX_PACKET_SIZE_HS_EPBULK
#define MAX_PACKET_SIZE_HS_EPBULK             512
#endif
#ifndef MAX_PACKET_SIZE_FS_EPBULK
#define MAX_PACKET_SIZE_FS_EPBULK             64
#endif


/*
 *  関数プロトタイプ宣言
 */
TUSBD_ERCODE tusbdInit(TUSBD_Handle_t *pdevice, uint8_t id);
TUSBD_ERCODE tusbdDeInit(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdStart(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdStop(TUSBD_Handle_t *pdevice);

TUSBD_ERCODE tusbdSetupStage(TUSBD_Handle_t *pdevice, uint8_t *psetup);
TUSBD_ERCODE tusbdDataOutStage(TUSBD_Handle_t *pdevice, uint8_t epnum, uint8_t *pdata);
TUSBD_ERCODE tusbdDataInStage(TUSBD_Handle_t *pdevice, uint8_t epnum, uint8_t *pdata);

TUSBD_ERCODE tusbdReset(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdSetSpeed(TUSBD_Handle_t *pdevice, uint8_t speed);
TUSBD_ERCODE tusbdSuspend(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdResume(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdIsoInIncomplete(TUSBD_Handle_t *pdevice, uint8_t epnum);
TUSBD_ERCODE tusbdIsoOutIncomplete(TUSBD_Handle_t *pdevice, uint8_t epnum);

TUSBD_ERCODE tusbdDeviceConnected(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdDeviceDisconnected(TUSBD_Handle_t *pdevice);

TUSBD_ERCODE tusbdControlSendData(TUSBD_Handle_t *pdevice, uint8_t *buf, uint16_t len);
TUSBD_ERCODE tusbdControlReceiveStart(TUSBD_Handle_t *pdevice, uint8_t *pbuf, uint16_t len);
TUSBD_ERCODE tusbdControlSendStatus(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdControlReceiveStatus(TUSBD_Handle_t *pdevice);
void         tusbdControlSendStall(TUSBD_Handle_t *pdevice, UsbSetupRequest *req);

#define tusbdControlContinueSendData(d, b, l)   tusbdDriverStartTransmit((d), 0x00, (b), (l))
#define tusbdControlContinueReceive(d, b, l) tusbdDriverSetupReceive((d), 0x00, (b), (l))

uint8_t      *tusbdFindDescriptor(TUSBD_Handle_t *pdevice, uint8_t type);

#ifdef __cplusplus
}
#endif

#endif /* _TUSBD_BASE_H_ */

