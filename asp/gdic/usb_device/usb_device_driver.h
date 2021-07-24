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
 *  @(#) $Id: usb_device_driver.h 698 2017-11-20 18:37:28Z roi $
 */
/*
 *  STM USB DEVICE HIGH DRIVER インクルードファイル
 */


#ifndef _USB_DEVICE_DRIVER_H_
#define _USB_DEVICE_DRIVER_H_

#include "tusb_rtos.h"
#include "usb_device.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define usb_device_getrxdatasize(d, a)  (((USB_DEV_Handle_t *)((d)->pSysData))->OUT_ep[(a) & 0x7F].xfer_count)

/*
 *  STM USB DEVICE HIGH DRIVER関数プロトタイプ宣言
 */
extern TUSBD_ERCODE usb_device_init(TUSBD_Handle_t *pdevice);
extern TUSBD_ERCODE usb_device_deinit(TUSBD_Handle_t *pdevice);
extern TUSBD_ERCODE usb_device_start(TUSBD_Handle_t *pdevice);
extern TUSBD_ERCODE usb_device_stop(TUSBD_Handle_t *pdevice);
extern TUSBD_ERCODE usb_device_openep(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t type, uint16_t mps);
extern TUSBD_ERCODE usb_device_closeep(TUSBD_Handle_t *pdevice, uint8_t addr);
extern TUSBD_ERCODE usb_device_flushep(TUSBD_Handle_t *pdevice, uint8_t addr);
extern TUSBD_ERCODE usb_device_stallep(TUSBD_Handle_t *pdevice, uint8_t addr);
extern TUSBD_ERCODE usb_device_clearep(TUSBD_Handle_t *pdevice, uint8_t addr);
extern uint8_t      usb_device_getstallcondition(TUSBD_Handle_t *pdevice, uint8_t addr);
extern TUSBD_ERCODE usb_device_transmitdata(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t *pbuf, uint16_t size);
extern TUSBD_ERCODE usb_device_startreceive(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t *pbuf, uint16_t size);
extern TUSBD_ERCODE usb_device_testmode(TUSBD_Handle_t *pdevice);


#ifdef __cplusplus
}
#endif

#endif /* _USB_DEVICE_DRIVER_H_ */

