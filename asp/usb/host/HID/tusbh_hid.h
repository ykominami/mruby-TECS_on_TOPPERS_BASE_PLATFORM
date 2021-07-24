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
 *  @(#) $Id: tusbh_hid.h 698 2017-09-17 16:23:27Z roi $
 */
/*
 *  USB Host Middleware HID部定義
 */

#ifndef _HUSBH_HID_H_
#define _HUSBH_HID_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define HID_MIN_POLL            10
#define HID_MAX_REPORT_LENGTH   8

#define HID_MOUSE_TYPE          1
#define HID_KEYBOARD_TYPE       2
#define HID_UNKNOWN_TYPE        0xFF

/*
 *  HID BOOT Interface Descriptor field values
 */
#define HID_BOOT_CODE           0x01
#define HID_KEYBRD_BOOT_CODE    0x01
#define HID_MOUSE_BOOT_CODE     0x02


/*
 *  HIDクラス要求ID
 */
#define USB_HID_GET_REPORT      0x01
#define USB_HID_GET_IDLE        0x02
#define USB_HID_GET_PROTOCOL    0x03
#define USB_HID_SET_REPORT      0x09
#define USB_HID_SET_IDLE        0x0A
#define USB_HID_SET_PROTOCOL    0x0B

#define HID_SET_TYPE            (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)
#define HID_GET_TYPE            (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE | USB_RECIPIENT_ENDPOINT)

/*
 *  HID状態定義
 */
enum {
	THID_INIT = 0,
	THID_IDLE,
	THID_GETREPORT_WAIT,
	THID_SETREPORT,
	THID_SETREPORT_WAIT,
	THID_GET_DATA,
	THID_POLL,
	THID_CLEARFEATURE_WAIT,
	THID_ERROR
};


/*
 *  HIDハンドラ定義
 */
typedef struct
{
	uint8_t                   buffer[32];
	uint8_t                   type;
	uint8_t                   InPipe;
	uint8_t                   state;
	uint8_t                   ep_addr;
	uint8_t                   DataReady;
	uint8_t                   ReqReport;
	uint16_t                  length;
	uint16_t                  poll;
	uint32_t                  timer;
	uint16_t                  ReportDescLength;
	uint8_t                   ReportDesc[66];
	uint8_t                   ReportType;
	uint8_t                   ReportId;
	uint16_t                  ReportLength;
	uint8_t                   Report[32];
} HID_Handle_t;


void    tusbhLinkHID(TUSBH_Handle_t *phost);

TUSBH_ERCODE tubhHidSetReport(HID_Handle_t *hhid, uint8_t type, uint8_t id, uint8_t* pbuf, uint8_t len);
uint8_t tusbhHidGetType(TUSBH_Device_t *pdevice);
uint8_t tusbhHidGetPollInterval(TUSBH_Device_t *hdevice);
void    tusbhSetHidCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *, uint8_t, uint8_t *));



#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_HID_H_ */

