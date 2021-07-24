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
 *  @(#) $Id: tusbd_hid.h 698 2017-10-10 18:37:30Z roi $
 */
/* Copyright (c) 2010-2011 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 *  USB Device HID CLASSS部定義
 */

#ifndef _TUSBD_HID_H_
#define _TUSBD_HID_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include  "tusbd_base.h"


#define EPIN_ADDR                 0x81
#define EPIN_SIZE                 0x04

#define TOTAL_HID_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (1 * ENDPOINT_DESCRIPTOR_LENGTH))

#define NUM_MAX_OUTREPORT         128

#define USB_GET_REPORT            0x1
#define USB_GET_IDLE              0x2
#define USB_GET_PROTOCOL          0x3
#define USB_SET_REPORT            0x9
#define USB_SET_IDLE              0xA
#define USB_SET_PROTOCOL          0xB


/* Main items */
#define INPUT(size)               (0x80 | size)
#define OUTPUT(size)              (0x90 | size)
#define FEATURE(size)             (0xb0 | size)
#define COLLECTION(size)          (0xa0 | size)
#define END_COLLECTION(size)      (0xc0 | size)

/* Global items */
#define USAGE_PAGE(size)          (0x04 | size)
#define LOGICAL_MINIMUM(size)     (0x14 | size)
#define LOGICAL_MAXIMUM(size)     (0x24 | size)
#define PHYSICAL_MINIMUM(size)    (0x34 | size)
#define PHYSICAL_MAXIMUM(size)    (0x44 | size)
#define UNIT_EXPONENT(size)       (0x54 | size)
#define UNIT(size)                (0x64 | size)
#define REPORT_SIZE(size)         (0x74 | size)
#define REPORT_ID(size)           (0x84 | size)
#define REPORT_COUNT(size)        (0x94 | size)
#define PUSH(size)                (0xa4 | size)
#define POP(size)                 (0xb4 | size)

/* Local items */
#define USAGE(size)               (0x08 | size)
#define USAGE_MINIMUM(size)       (0x18 | size)
#define USAGE_MAXIMUM(size)       (0x28 | size)
#define DESIGNATOR_INDEX(size)    (0x38 | size)
#define DESIGNATOR_MINIMUM(size)  (0x48 | size)
#define DESIGNATOR_MAXIMUM(size)  (0x58 | size)
#define STRING_INDEX(size)        (0x78 | size)
#define STRING_MINIMUM(size)      (0x88 | size)
#define STRING_MAXIMUM(size)      (0x98 | size)
#define DELIMITER(size)           (0xa8 | size)

/*
 *  HIDデータ状態定義
 */
#define HID_DATA_IDLE             0
#define HID_DATA_BUSY             1



typedef struct
{
	uint8_t             hidData[32];
	uint8_t             devReport[NUM_MAX_OUTREPORT];
	uint16_t            devReportLength;
	uint8_t             Protocol;
	uint8_t             IdleState;
	uint8_t             AltSetting;
	uint8_t             datastate;
} TUSBD_HID_Handle_t; 


extern uint8_t configurationHidDescriptor[];
extern uint8_t configurationHidOtherDescriptor[];


TUSBD_ERCODE tusbdHidSendData(TUSBD_Handle_t *pdevice, uint8_t *pdata, uint16_t len);
TUSBD_ERCODE tusbdHidSetupReport(TUSBD_Handle_t *pdevice, uint8_t *report, uint16_t len);

TUSBD_ERCODE tusbdHidInit(TUSBD_Handle_t *pdevice, uint8_t idx);
TUSBD_ERCODE tusbdHidDeInit(TUSBD_Handle_t *pdevice, uint8_t idx);
void         tusbdHidSetup(TUSBD_Handle_t *pdevice, UsbSetupRequest *req);
void         tusbdHidDataIn(TUSBD_Handle_t *pdevice, uint8_t epnum);


#ifdef __cplusplus
}
#endif

#endif /* _TUSBD_HID_H_ */

