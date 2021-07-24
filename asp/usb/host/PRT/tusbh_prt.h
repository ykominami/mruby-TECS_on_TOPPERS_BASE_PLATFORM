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
 *  @(#) $Id: tusbh_prt.h 698 2017-09-21 10:12:27Z roi $
 */
/*
 *  USB Host Middleware PRT部定義
 */

#ifndef _HUSBH_PRT_H_
#define _HUSBH_PRT_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define PRT_VERNDER_REQUEST_EOJ     0
#define PRT_REQUEST_DEVICEID        0
#define PRT_REQUEST_PORTSTATUS      1
#define PRT_REQUEST_SOFTRESET       2

#define PRT_REQUEST_STATUS_TYPE         (USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)
#define PRT_VENDER_SEND_TYPE            (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_VENDOR | USB_RECIPIENT_INTERFACE)

/*
 *  PRTコールバックコード定義
 */
#define PRT_LINE_STATUS_CHANGED         1
#define PRT_LINE_STATUS_SENDED          2
#define PRT_LINE_STATUS_RECEIVED        3

/*
 *  PRTプロセスの状態定義
 */
enum
{
	PRT_PROCESS_INIT = 0,
	PRT_PROCESS_IDLE,
	PRT_PROCESS_CONTROL,
	PRT_PROCESS_CONTROL_WAIT,
	PRT_PROCESS_SEND,
	PRT_PROCESS_RECEIVE,
	PRT_PROCESS_TRANSFER_WAIT,
	PRT_PROCESS_ERROR,
	PRT_PROCESS_ERROR_WAIT
};

/*
 *  PRTハンドラ定義
 */
typedef struct
{
	TUSBH_Device_t            *pDev;
	uint8_t                   state;
	uint8_t                   pre_state;
	uint8_t                   cmd;
	uint8_t                   dummy;
	uint16_t                  TrnEpSize;
	uint16_t                  TrnPktSize;

	uint8_t                   InPipe;
	uint8_t                   OutPipe;
	uint8_t                   OutEp;
	uint8_t                   InEp;
	uint16_t                  OutEpSize;
	uint16_t                  InEpSize;
	uint8_t                   controlType;
	uint8_t                   controlRequest;
	uint16_t                  controlLength;
	uint32_t                  TrnLength;
	uint32_t                  TrnSize;
	uint8_t                   *pTrnBuff;
	uint32_t                  timer;
	uint32_t                  urb_wait;
	ID                        rw_taskid;
	uint32_t                  rw_status;
	uint8_t                   buffer[64];
} PRT_Handle_t;


void         tusbhLinkPRT(TUSBH_Handle_t *phost);
TUSBH_ERCODE tusbhPrtEOJ(TUSBH_Handle_t *phost, uint8_t unit, uint8_t cmd, uint8_t prm1, uint8_t prm2);
TUSBH_ERCODE tusbhPrtPortID(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *buf);
TUSBH_ERCODE tusbhPrtSend(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint32_t length);
TUSBH_ERCODE tusbhPrtReceive(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint32_t *length, uint32_t timeout);
void tusbhSetPrtCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *p, uint8_t));


#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_PRT_H_ */

