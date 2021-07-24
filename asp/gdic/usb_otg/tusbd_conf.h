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
 *  @(#) $Id: tusbd_conf.h 698 2017-10-05 21:46:02Z roi $
 */
/*
 *  TOPPERS USB DEVICE MODULE CONFIGURATION インクルードファイル
 */

#ifndef _TUSBD_CONF_H_
#define _TUSBD_CONF_H_

#include "dwc2_device_driver.h"

#define TUSBD_MAX_NUM_INTERFACES              1
#define TUSBD_MAX_NUM_CONFIGURATION           1
#define TUSBD_NUM_ENDPOINT                    15
#define TUSBD_CONFIG_SELF_POWERED             0x01

#define USBD_StatusTypeDef   TUSBD_ERCODE
#define USBD_HandleTypeDef   TUSBD_Handle_t
#define USBD_Init            tusbdInit
#define USBD_Start           tusbdStart

extern uint8_t defaultDeviceDescriptor[];
extern uint8_t defaultDeviceQualifierDescriptor[];
extern uint8_t defaultLanguageIDDescriptor[];
extern uint8_t defaultManufacturerString[];
extern uint8_t defaultProductString[];
extern uint8_t defaultSerialString[];
extern uint8_t defaultConfigurationString[];
extern uint8_t defaultInterfaceString[];

#define TUSBD_DEVICE_DESCRIPTOR               defaultDeviceDescriptor
#define TUSBD_DEVICE_QUALIFIER_DESCRIPTOR     defaultDeviceQualifierDescriptor
#define TUSBD_LANGUAGEID_DESCRIPTOR           defaultLanguageIDDescriptor
#define TUSBD_MANUFACTURER_STRING             defaultManufacturerString
#define TUSBD_PRODUCT_STRING                  defaultProductString
#define TUSBD_SERIAL_STRING                   defaultSerialString
#define TUSBD_CONFIGURATION_STRING            defaultConfigurationString
#define TUSBD_INTERFACE_STRING                defaultInterfaceString
#define TUSBD_USER_STRING                     NULL

#define tusbdDriverSetUSBAddress(d, a)  usbo_setDevAddress(((d)->pSysData), (a))
#define tusbdDriverGetRxDataSize        dwc2_device_getrxdatasize

#define tusbdDriverInit                 dwc2_device_init
#define tusbdDriverDeInit               dwc2_device_deinit
#define tusbdDriverStart                dwc2_device_start
#define tusbdDriverStop                 dwc2_device_stop
#define tusbdDriverOpenEp               dwc2_device_openep
#define tusbdDriverCloseEp              dwc2_device_closeep
#define tusbdDriverFlushEp              dwc2_device_flushep
#define tusbdDriverStallEp              dwc2_device_stallep
#define tusbdDriverClearStallEp         dwc2_device_clearep
#define tusbdDriverStallConditionEp     dwc2_device_getstallcondition
#define tusbdDriverStartTransmit        dwc2_device_transmitdata
#define tusbdDriverSetupReceive         dwc2_device_startreceive
#define tusbdDriverTestMode             dwc2_device_testmode

#endif /* _TUSBD_CONF_H_ */

