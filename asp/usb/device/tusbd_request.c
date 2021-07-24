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
 *  @(#) $Id: tusbd_request.c 698 2017-10-06 21:56:09Z roi $
 */
/*
 *  USB Device Middleware REQUEST部
 */

#include "tusbd_base.h"


/*
 *  USB DEVICE コントロール通信 送信開始
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 pbuf:     送信バッファ
 *  parameter3 len:      送信データ長
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdControlSendData(TUSBD_Handle_t *pdevice, uint8_t *pbuf, uint16_t len)
{
	pdevice->ep0_state          = TUSBD_EP0_DATAIN;
	pdevice->ep_in[0].xfersize  = len;
	pdevice->ep_in[0].remlength = len;
	return tusbdDriverStartTransmit(pdevice, 0x00, pbuf, len);
}

/*
 *  USB DEVICE コントロール通信 受信開始
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 pbuf:     受信バッファ
 *  parameter3 len:      受信データ長
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdControlReceiveStart(TUSBD_Handle_t *pdevice, uint8_t *pbuf, uint16_t len)
{
	pdevice->ep0_state = TUSBD_EP0_DATAOUT; 
	pdevice->ep_out[0].xfersize  = len;
	pdevice->ep_out[0].remlength = len;
	return tusbdDriverSetupReceive(pdevice, 0, pbuf, len);
}

/*
 *  USB DEVICE コントロール通信 ステータス送信
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdControlSendStatus(TUSBD_Handle_t *pdevice)
{
	pdevice->ep0_state = TUSBD_EP0_STATUSIN;
	return tusbdDriverStartTransmit(pdevice, 0x00, NULL, 0);
}

/*
 *  USB DEVICE コントロール通信 ステータス受信
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdControlReceiveStatus(TUSBD_Handle_t *pdevice)
{
	pdevice->ep0_state = TUSBD_EP0_STATUSOUT;
	return tusbdDriverSetupReceive(pdevice, 0, NULL, 0);
}

/*
 *  USB DEVICE コントロール通信 STALL送信
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 req:      セットアップリクエスト
 *  return     TUSBD_ERCODE
 */

void
tusbdControlSendStall(TUSBD_Handle_t *pdevice, UsbSetupRequest *req)
{
	pdevice->ep0_state = TUSBD_EP0_IDLE;
	tusbdDriverStallEp(pdevice, req->bmRequest & 0x80);
}


