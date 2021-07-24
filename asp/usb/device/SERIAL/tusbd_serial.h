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
 *  @(#) $Id: tusbd_serial.h 698 2017-10-15 15:13:47Z roi $
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
 *  USB Device CDC SERIAL CLASSS部定義
 */

#ifndef _TUSBD_SERIAL_H_
#define _TUSBD_SERIAL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include  "tusbd_base.h"


#define CDC_EPBULKIN_ADDR                           0x81
#define CDC_EPBULKOUT_ADDR                          0x01
#define CDC_EPINT_ADDR                              0x82

#define TOTAL_CDC_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (2 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (5 + 5 + 5 + 4) \
                               + (3 * ENDPOINT_DESCRIPTOR_LENGTH))

#define CDC_PACKET_SIZE_EPINT                       8
#define CDC_LINE_CODE_SIZE                          7

/*
 *  CDC要求定義
 */
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23

#define CDC_RECEIVED                                0x40


typedef struct
{
	uint8_t  rxdata[MAX_PACKET_SIZE_HS_EPBULK];
	uint8_t  cmddata[64];
	uint8_t  CmdCode;
	uint8_t  CmdLength;
	uint16_t packetsize;
	uint32_t RxLength;
	uint8_t  *TxBuffer;
	uint32_t TxLength;
	volatile uint32_t TxState;
} TUSBD_CDC_Handle_t;

extern uint8_t configurationCdcHsDescriptor[];
extern uint8_t configurationCdcFsDescriptor[];
extern uint8_t configurationCdcOtrDescriptor[];


TUSBD_ERCODE tusbdCdcSetReceivePacket(TUSBD_Handle_t *pdevice);
TUSBD_ERCODE tusbdCdcStartTransmit(TUSBD_Handle_t *pdevice, uint8_t *pbuff, uint32_t length);


TUSBD_ERCODE tusbdCdcInit(TUSBD_Handle_t *pdevice, uint8_t idx);
TUSBD_ERCODE tusbdCdcDeInit(TUSBD_Handle_t *pdevice, uint8_t idx);
void         tusbdCdcEP0RxReady(TUSBD_Handle_t *pdevice);
void         tusbdCdcSetup(TUSBD_Handle_t *pdevice, UsbSetupRequest *req);
void         tusbdCdcDataIn(TUSBD_Handle_t *pdevice, uint8_t epnum);
void         tusbdCdcDataOut(TUSBD_Handle_t *pdevice, uint8_t epnum);


#ifdef __cplusplus
}
#endif

#endif /* _TUSBD_SERIAL_H_ */

