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
 *  @(#) $Id: tusbh_hub.h 698 2017-09-12 18:44:27Z roi $
 */
/*
 *  USB Host Middleware HUB部定義
 */

#ifndef _HUSBH_HUB_H_
#define _HUSBH_HUB_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif



#define PORT_CONNECTION_FEATURE         (0x00)
#define PORT_ENABLEFEATURE              (0x01)
#define PORT_SUSPEND                    (0x02)
#define PORT_OVER_CURRENT_FEATURE       (0x03)
#define PORT_RESETFEATURE               (0x04)
#define PORT_POWERFEATURE               (0x08)
#define PORT_LOW_SPEED                  (0x09)

#define PORT_CLEAR_CONNECTION_FEATURE   (0x10)
#define PORT_CLEAR_ENABLE_FEATURE       (0x11)
#define PORT_CLEAR_RESET_FEATURE        (0x14)

#define PORT_SETFEATURE_TYPE            (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE | USB_RECIPIENT_ENDPOINT)
#define PORT_CLEARFEATURE_TYPE          (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE | USB_RECIPIENT_ENDPOINT)
#define PORT_GETSTATUS_TYPE             (USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE | USB_RECIPIENT_ENDPOINT)

#define PORT_STATUS_CONNECTION          (1<<PORT_CONNECTION_FEATURE)
#define PORT_STATUS_ENABLE              (1<<PORT_ENABLEFEATURE)
#define PORT_STATUS_SUSPEND             (1<<PORT_SUSPEND)
#define PORT_STATUS_OVER_CURRENT        (1<<PORT_OVER_CURRENT_FEATURE)
#define PORT_STATUS_RESET               (1<<PORT_RESETFEATURE)
#define PORT_STATUS_POWER               (1<<PORT_POWERFEATURE)
#define PORT_STATUS_LOW_SPEED           (1<<PORT_LOW_SPEED)

#define PORT_CHANGE_CONNECTION          (1<<16)
#define PORT_CHANGE_ENABLE              (1<<17)
#define PORT_CHANGE_SUSPEND             (1<<18)
#define PORT_CHANGE_OVER_CURRENT        (1<<19)
#define PORT_CHANGE_RESET               (1<<20)

/*
 * HUB状態定義
 */
enum
{
	THUB_IDLE = 0,
	THUB_SYNC,
	THUB_GET_DATA,
	THUB_POLL,
	THUB_GET_STATUS_WAIT,
	THUB_CLEAR_FEATURE_WAIT,
	THUB_PORT_RESET_WAIT,
	THUB_CONNECT_WAIT,
	THUB_PORT_ERROR,
	THUB_PORT_ERROR_WAIT
};

/*
 *  HUB CONNECT状態定義
 */
enum
{
	THUB_CONNCET_IDEL = 0,
	THUB_CONNECT_RESET,
	THUB_CONNECT_ENABLE,
	THUB_CONNECT_ACTIVE
};

/*
 *  HUBハンドラ定義
 */
typedef struct
{
	TUSBH_Device_t            *pDev;
	uint8_t                   state;
	uint8_t                   hubid;
	uint8_t                   numport;
	uint8_t                   characteristics;
	uint32_t                  timer;
	uint32_t                  starttime;

	uint8_t                   InPipe;
	uint8_t                   ep_addr;
	uint8_t                   portno;
	uint8_t                   cstate;
	uint16_t                  length;
	uint16_t                  poll; 
	uint32_t                  reset_portid;
	uint32_t                  dummy2;
	uint8_t                   buffer[64];
} HUB_Handle_t;


void  tusbhLinkHUB(TUSBH_Handle_t *phost);


#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_HUB_H_ */

