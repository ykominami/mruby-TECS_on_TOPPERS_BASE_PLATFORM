/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2016-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  $Id: usbddesc.c 2416 2017-05-28 11:02:20Z roi $
 */

/*
 *  USB DEVICE DESCRIPTORの設定
 */

#include "tusbd_base.h"

#define MANUFACTURE_LENGTH            64
#define PRODUCT_LENGTH                32
#define CONFIGURATION_LENGTH          32
#define INTERFACE_LENGTH              32

/* Private define STM defintion ------------------------------------------------------*/
#define USBD_VID                      0x0483
#if USB_DEVICE_ID == 0
#define USBD_PID                      0x5710
#else
#define USBD_PID                      0x5740
#endif
#define USBD_RELNO                    0x0200
#define USBD_MANUFACTURER_STRING      "STMicroelectronics"
#if USB_DEVICE_ID == 0
#define USBD_PRODUCT_STRING           "HID Mouse"
#define USBD_CONFIGURATION_HS_STRING  "HID Config"
#define USBD_INTERFACE_HS_STRING      "HID Interface"
#define USBD_CONFIGURATION_FS_STRING  "HID Config"
#define USBD_INTERFACE_FS_STRING      "HID Interface"
#else
#define USBD_PRODUCT_STRING           "STM32 Virtual ComPort"
#define USBD_CONFIGURATION_HS_STRING  "VCP Config"
#define USBD_INTERFACE_HS_STRING      "VCP Interface"
#define USBD_CONFIGURATION_FS_STRING  "VCP Config"
#define USBD_INTERFACE_FS_STRING      "VCP Interface"
#endif

/* USB Standard Device Descriptor */
uint8_t defaultDeviceDescriptor[] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	0x12,						/* 0:bLength */
	DEVICE_DESCRIPTOR,			/* 1:bDescriptorType */
	0x00,                       /* 2:bcdUSB */
	0x02,						/* 3:bcdUSB */
	0x00,						/* 4:bDeviceClass */
	0x00,						/* 5:bDeviceSubClass */
	0x00,						/* 6:bDeviceProtocol */
	USB_MAX_EP0_SIZE,			/* 7:bMaxPacketSize */
	(USBD_VID & 0xFF),			/* 8:idVendor */
	(USBD_VID >> 8),			/* 9:idVendor */
	(USBD_PID & 0xFF),			/* 10:idVendor */
	(USBD_PID >> 8),			/* 11:idVendor */
	(USBD_RELNO & 0XFF),		/* 12:bcdDevice */
	(USBD_RELNO >> 8),			/* 13:bcdDevice */
	0x01,						/* 14:iManufacturer */
	0x02,						/* 15:iProduct */
	0x03,						/* 16:iSerialNumber */
	TUSBD_MAX_NUM_CONFIGURATION  /* 17:bNumConfigurations */
}; /* USB_DeviceDescriptor */

/* USB Standard Device Descriptor */
uint8_t defaultDeviceQualifierDescriptor[] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	0x0A,
	DEVICE_QUALIFIER_DESCRIPTOR,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00
};

/* USB Standard Device Descriptor */
uint8_t defaultLanguageIDDescriptor[] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	0x04,
	STRING_DESCRIPTOR,
	0x09,
	0x04
};

uint8_t defaultManufacturerString[MANUFACTURE_LENGTH*2] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	MANUFACTURE_LENGTH*2,
	STRING_DESCRIPTOR,
	0x00
};

uint8_t defaultProductString[PRODUCT_LENGTH*2] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	PRODUCT_LENGTH*2,
	STRING_DESCRIPTOR,
	0x00
};

uint8_t defaultSerialString[] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	0x1A,
	STRING_DESCRIPTOR,
	'A', 0x00,
	'B', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'1', 0x00
};

uint8_t defaultConfigurationString[CONFIGURATION_LENGTH*2] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	CONFIGURATION_LENGTH*2,
	STRING_DESCRIPTOR,
	0x00
};

uint8_t defaultInterfaceString[INTERFACE_LENGTH*2] __attribute__ ((aligned(USB_DATA_ALIGN))) = {
	INTERFACE_LENGTH*2,
	STRING_DESCRIPTOR,
	0x00
};

/* ascii to unicode 変換 */
static uint8_t
ascii2unicode(const char *src, uint16_t *dst, uint16_t len)
{
	uint8_t i, size;
	uint8_t us, *p;

	for(i = 0 ; i < len ; i++, src++, dst++){
		us = *src;
		if(us == 0)
			break;
		p = (uint8_t *)dst;
		p[0] = us;
		p[1] = 0;
	}
	size = i;

	/* 0が見つかったら残りすべて0にする */
	for( ; i < len ; i++, dst++){
		*dst = 0;
	}
	return size;
}

static void
MakeDescriptor(uint8_t speed)
{
	uint8_t len;

	len = ascii2unicode(USBD_MANUFACTURER_STRING, (uint16_t *)&defaultManufacturerString[2], MANUFACTURE_LENGTH-2);
	defaultManufacturerString[0] = len * 2 + 2;
	syslog_2(LOG_NOTICE, "defaultManufacturerString len(%d) [%08x]", len, defaultManufacturerString);
	len = ascii2unicode(USBD_PRODUCT_STRING, (uint16_t *)&defaultProductString[2], PRODUCT_LENGTH-2);
	defaultProductString[0] = len * 2 + 2;
	syslog_2(LOG_NOTICE, "defaultProductString len(%d) [%08x]", len, defaultProductString);
	if(speed == USB_DEVICE_SPEED_HIGH)
		len = ascii2unicode(USBD_CONFIGURATION_HS_STRING, (uint16_t *)&defaultConfigurationString[2], CONFIGURATION_LENGTH-2);
	else
		len = ascii2unicode(USBD_CONFIGURATION_FS_STRING, (uint16_t *)&defaultConfigurationString[2], CONFIGURATION_LENGTH-2);
	defaultConfigurationString[0] = len * 2 + 2;
	syslog_2(LOG_NOTICE, "defaultConfigurationString len(%d) [%08x]", len, defaultConfigurationString);
	if(speed == USB_DEVICE_SPEED_HIGH)
		len = ascii2unicode(USBD_INTERFACE_HS_STRING, (uint16_t *)&defaultInterfaceString[2], INTERFACE_LENGTH-2);
	else
		len = ascii2unicode(USBD_INTERFACE_FS_STRING, (uint16_t *)&defaultInterfaceString[2], INTERFACE_LENGTH-2);
	defaultInterfaceString[0] = len * 2 + 2;
	syslog_2(LOG_NOTICE, "defaultInterfaceString len(%d) [%08x]", len, defaultInterfaceString);
}

void
MakeUsbDescriptor(TUSBD_Handle_t *pdev)
{
	pdev->makeDescriptor = MakeDescriptor;
}

