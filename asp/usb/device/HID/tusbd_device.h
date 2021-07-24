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
 *  @(#) $Id: tusbd_device.h 698 2017-10-05 16:49:00Z roi $
 */
/*
 *  TOPPERS USB DEVICE CLASS CONFIGURATION インクルードファイル
 */

#ifndef _TUSBD_DEVICE_H_
#define _TUSBD_DEVICE_H_

#define tusbdClassInit                tusbdHidInit
#define tusbdClassDeInit              tusbdHidDeInit
#define tusbdClassSuspend(a)
#define tusbdClassResume(a)
#define tusbdClassEp0RxReady(a)
#define tusbdClassEp0TxSent(a)
#define tusbdClassSetup               tusbdHidSetup
#define tusbdClassDataIn              tusbdHidDataIn
#define tusbdClassDataOut(a, b)
#define tusbdClassInIncomlete(a, b)
#define tusbdClassOutIncomlete(a, b)
#define USBD_HID_SendReport           tusbdHidSendData

#define TUSBD_HS_CONFIG_DESCRIPTOR    configurationHidDescriptor
#define TUSBD_HS_CONFIG_DESC_LENGTH   TOTAL_HID_DESCRIPTOR_LENGTH
#define TUSBD_FS_CONFIG_DESCRIPTOR    configurationHidDescriptor
#define TUSBD_FS_CONFIG_DESC_LENGTH   TOTAL_HID_DESCRIPTOR_LENGTH
#define TUSBD_OTR_CONFIG_DESCRIPTOR   configurationHidOtherDescriptor
#define TUSBD_OTR_CONFIG_DESC_LENGTH  TOTAL_HID_DESCRIPTOR_LENGTH

#include "tusbd_hid.h"


#endif /* _TUSBD_DEVICE_H_ */

