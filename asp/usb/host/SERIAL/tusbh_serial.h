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
 *  @(#) $Id: tusbh_serial.h 698 2017-09-20 18:44:27Z roi $
 */
/*
 *  USB Host Middleware CDC:SERIAL部定義
 */

#ifndef _HUSBH_SERIAL_H_
#define _HUSBH_SERIAL_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif


/*
 *  COMMUNCATION SUB CLASS CODE定義
 */
#define COMMUNICATIONS_INTERFACE_CLASS  0x00
#define DIRECT_LINE_CONTROL_MODEL       0x01
#define ABSTRACT_CONTROL_MODEL          0x02
#define TELEPHONE_CONTROL_MODEL         0x03
#define MULTICHANNEL_CONTROL_MODEL      0x04
#define CAPI_CONTROL_MODEL              0x05
#define ETHERNET_NETWORKING_CONTROL_MODEL 0x06
#define ATM_NETWORKING_CONTROL_MODEL    0x07
#define WIRELESS_HANDSET_CONTROL_MODEL  0x08
#define MOBILE_DIRECT_LINE_MODEL        0x0A
#define ETHERNET_EMULATION_MODEL        0x0C
#define NETWORK_CONTROL_MODEL           0x0D


/*
 *  COMMUNICATION INTERFCAE CLASS 制御プロトコルコード定義
 */
#define CDC_NO_SPECIFIC_PROTOCOL_CODE   0x00
#define CDC_ITU_T_V_250                 0x01
#define CDC_VENDOR_SPECIFIC             0xFF

/*
 *  DATA INTERFCAE CLASS 制御プロトコルコード定義
 */
#define DIC_NO_SPECIFIC_PROTOCOL_CODE   0x00
#define DIC_USBNCM_1_0                  0x01


#define CDC_SET_LINE_CODING             0x20
#define CDC_GET_LINE_CODING             0x21
#define CDC_SET_CONTROL_LINE_STATE      0x22
#define CDC_SEND_BREAK                  0x23

#define CDC_REQUEST_GETLINE_TYPE        (USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)
#define CDC_REQUEST_SETLINE_TYPE        (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)

#define LINE_CODING_STRUCTURE_SIZE      0x07

/*
 *  CDCコールバックコード定義
 */
#define CDC_LINE_STATUS_CHANGED         1
#define CDC_LINE_STATUS_SENDED          2
#define CDC_LINE_STATUS_RECEIVED        3

/*
 *  CDCプロセスの状態定義
 */
enum
{
	CDC_PROCESS_INIT = 0,
	CDC_PROCESS_INIT_WAIT,
	CDC_PROCESS_IDLE,
	CDC_PROCESS_SET_LINE_CODING,
	CDC_PROCESS_SET_LINE_CODING_WAIT,
	CDC_PROCESS_GET_LINE_CODING,
	CDC_PROCESS_GET_LINE_CODING_WAIT,
	CDC_PROCESS_SEND,
	CDC_PROCESS_RECEIVE,
	CDC_PROCESS_TRANSFER_WAIT,
	CDC_PROCESS_ERROR,
	CDC_PROCESS_ERROR_WAIT
};

/*
 *  CDCハンドラ定義
 */
typedef struct
{
	TUSBH_Device_t            *pDev;
	uint8_t                   state;
	uint8_t                   pre_state;
	uint16_t                  TrnEpSize;
	uint16_t                  TrnPktSize;

	uint8_t                   CommInPipe;
	uint8_t                   CommInEp;
	uint16_t                  CommInEpSize;
	uint8_t                   InPipe;
	uint8_t                   OutPipe;
	uint8_t                   OutEp;
	uint8_t                   InEp;
	uint16_t                  OutEpSize;
	uint16_t                  InEpSize;
	uint32_t                  TrnLength;
	uint32_t                  TrnSize;
	uint8_t                   *pTrnBuff;
	uint32_t                  timer;
	uint32_t                  urb_wait;
	ID                        rw_taskid;
	uint32_t                  rw_status;
	Line_Coding_t             oLineCoding;
	Line_Coding_t             uLineCoding;
} CDC_Handle_t;


void         tusbhLinkSERIAL(TUSBH_Handle_t *phost);
TUSBH_ERCODE tusbhCdcSetLineCoding(TUSBH_Handle_t *phost, uint8_t unit, Line_Coding_t *linecod);
TUSBH_ERCODE tusbhCdcGetLineCoding(TUSBH_Handle_t *phost, uint8_t unit, Line_Coding_t *linecod);
TUSBH_ERCODE tusbhCdcSend(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint32_t length);
TUSBH_ERCODE tusbhCdcReceive(TUSBH_Handle_t *phost, uint8_t unit, uint8_t *pbuf, uint32_t *length, uint32_t timeout);
void tusbhSetCdcCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *p, uint8_t));


#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_SERIAL_H_ */

