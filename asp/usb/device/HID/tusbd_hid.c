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
 *  @(#) $Id: tusbd_hid.c 698 2017-10-10 18:41:11Z roi $
 */
/*
 *  USB Device HID CLASS部
 */

#include "tusbd_hid.h"



#define HID_VERSION_1_11    (0x0111)

#define HID_INTERVAL_TIME    10

#ifndef HID_INTERFCAE_PROTOCOL
#define HID_INTERFCAE_PROTOCOL    0x02		/* 2:mouse 1:keyboard */
#endif



/*
 *  REPORT DESCRIPTOR
 */
static uint8_t reportDescriptor[]  __attribute__ ((aligned (USB_DATA_ALIGN))) = {

	USAGE_PAGE(1),   0x01,
	USAGE(1),   0x02,

	COLLECTION(1),   0x01,
	USAGE(1),   0x01,

	COLLECTION(1),   0x00,
	USAGE_PAGE(1),   0x09,
	USAGE_MINIMUM(1),   0x01,
	USAGE_MAXIMUM(1),   0x03,
	LOGICAL_MINIMUM(1),   0x00,
	LOGICAL_MAXIMUM(1),   0x01,
	REPORT_COUNT(1),   0x03,
	REPORT_SIZE(1),   0x01,
	INPUT(1),   0x02,
	REPORT_COUNT(1),   0x01,
	REPORT_SIZE(1),   0x05,
	INPUT(1),   0x01,
	USAGE_PAGE(1),   0x01,
	USAGE(1),   0x30,
	USAGE(1),   0x31,
	USAGE(1),   0x38,

	LOGICAL_MINIMUM(1),   0x81,
	LOGICAL_MAXIMUM(1),   0x7F,
	REPORT_SIZE(1),  0x08,
	REPORT_COUNT(1), 0x03,

	INPUT(1),   0x06,
	END_COLLECTION(0),

	USAGE(1), 0x3c,
	USAGE_PAGE(1), 0xff,
	USAGE(1), 0x01,
	LOGICAL_MINIMUM(1), 0x00,
	LOGICAL_MAXIMUM(1), 0x01,
	REPORT_SIZE(1), 0x01,
	REPORT_COUNT(1), 0x02,
	FEATURE(1), 0x22,
	REPORT_SIZE(1), 0x06,
	REPORT_COUNT(1), 0x01,
	FEATURE(1), 0x01,
	END_COLLECTION(0)
};

/*
 *  USB HID DEVICE CONFIGURATION DESCRIPTOR
 */
uint8_t configurationHidDescriptor[TOTAL_HID_DESCRIPTOR_LENGTH]  __attribute__ ((aligned (USB_DATA_ALIGN))) =
{
	CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
	CONFIGURATION_DESCRIPTOR,           // bDescriptorType
	(TOTAL_HID_DESCRIPTOR_LENGTH & 0xFF),   // wTotalLength (LSB)
	(TOTAL_HID_DESCRIPTOR_LENGTH >> 8),     // wTotalLength (MSB)
 	0x01,                               // bNumInterfaces
	0x01,                               // bConfigurationValue
	0x00,                               // iConfiguration
	C_RESERVED | C_SELF_POWERED | C_REMOTE_WAKEUP,	// bmAttributes
	C_POWER(100),                       // bMaxPower

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x00,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x01,                               // bNumEndpoints
	HID_CLASS,                          // bInterfaceClass
	0x01,                               // bInterfaceSubClass : 1=BOOT, 0=no boot
	HID_INTERFCAE_PROTOCOL,             // bInterfaceProtocol
	0x00,                               // iInterface

	HID_DESCRIPTOR_LENGTH,              // bLength
	HID_DESCRIPTOR,                     // bDescriptorType
	(HID_VERSION_1_11 & 0xFF),          // bcdHID (LSB)
	(HID_VERSION_1_11 >> 8),            // bcdHID (MSB)
	0x00,                               // bCountryCode
	0x01,                               // bNumDescriptors
	0x22,                               // bDescriptorType
	(sizeof(reportDescriptor) & 0xFF),  // wDescriptorLength (LSB)
	(sizeof(reportDescriptor) >> 8),    // wDescriptorLength (MSB)

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	EPIN_ADDR,                          // bEndpointAddress
	USB_EP_TYPE_INTR,                   // bmAttributes
	(EPIN_SIZE & 0xFF),                 // wMaxPacketSize (LSB)
	(EPIN_SIZE >> 8),                   // wMaxPacketSize (MSB)
	HID_INTERVAL_TIME                   // bInterval (milliseconds)
};

/*
 *  USB HID DEVICE OTHER CONFIGURATION DESCRIPTOR
 */
uint8_t configurationHidOtherDescriptor[TOTAL_HID_DESCRIPTOR_LENGTH]  __attribute__ ((aligned (USB_DATA_ALIGN))) =
{
	CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
	OTHER_SPEED_CONFIGURATION_DESC,     // bDescriptorType
	(TOTAL_HID_DESCRIPTOR_LENGTH & 0xFF),   // wTotalLength (LSB)
	(TOTAL_HID_DESCRIPTOR_LENGTH >> 8),     // wTotalLength (MSB)
 	0x01,                               // bNumInterfaces
	0x01,                               // bConfigurationValue
	0x00,                               // iConfiguration
	C_RESERVED | C_SELF_POWERED | C_REMOTE_WAKEUP,	// bmAttributes
	C_POWER(100),                       // bMaxPower

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x00,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x01,                               // bNumEndpoints
	HID_CLASS,                          // bInterfaceClass
	0x01,                               // bInterfaceSubClass : 1=BOOT, 0=no boot*/
	0x02,                               // bInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
	0x00,                               // iInterface

	HID_DESCRIPTOR_LENGTH,              // bLength
	HID_DESCRIPTOR,                     // bDescriptorType
	(HID_VERSION_1_11 & 0xFF),          // bcdHID (LSB)
	(HID_VERSION_1_11 >> 8),            // bcdHID (MSB)
	0x00,                               // bCountryCode
	0x01,                               // bNumDescriptors
	0x22,                               // bDescriptorType
	(sizeof(reportDescriptor) & 0xFF),  // wDescriptorLength (LSB)
	(sizeof(reportDescriptor) >> 8),    // wDescriptorLength (MSB)

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	EPIN_ADDR,                          // bEndpointAddress
	USB_EP_TYPE_INTR,                   // bmAttributes
	(EPIN_SIZE & 0xFF),                 // wMaxPacketSize (LSB)
	(EPIN_SIZE >> 8),                   // wMaxPacketSize (MSB)
	HID_INTERVAL_TIME                   // bInterval (milliseconds)
};

static TUSBD_HID_Handle_t HID_CLASS_DATA __attribute__ ((aligned (USB_DATA_ALIGN)));


/*
 *  USB DEVICE HID初期化
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 idx:      Configuration index
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdHidInit(TUSBD_Handle_t *pdevice, uint8_t idx)
{
	tusbdDriverOpenEp(pdevice, EPIN_ADDR, USB_EP_TYPE_INTR, EPIN_SIZE);
	memset(&HID_CLASS_DATA, 0, sizeof(TUSBD_HID_Handle_t));
	HID_CLASS_DATA.datastate = HID_DATA_IDLE;

	pdevice->pClassData = &HID_CLASS_DATA;
	return TUSBD_E_OK;
}

/*
 *  USB DEVICE HIDクラス終了
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 idx:      Configuration index
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdHidDeInit(TUSBD_Handle_t *pdevice, uint8_t idx)
{
	tusbdDriverCloseEp(pdevice, EPIN_ADDR);
	pdevice->pClassData = NULL;
	return TUSBD_E_OK;
}

/*
 *  USB DEVICE HIDクラスセットアップ
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 req:      usb control requests
 *  return     TUSBD_ERCODE
 */
void
tusbdHidSetup(TUSBD_Handle_t *pdevice, UsbSetupRequest *req)
{
	TUSBD_HID_Handle_t *hhid = (TUSBD_HID_Handle_t*) pdevice->pClassData;
	uint16_t len = 0;
	volatile uint8_t  *pbuf = NULL;
	uint8_t  *p;
	uint32_t i;

	switch (req->bmRequest & USB_REQUEST_TYPE_MASK){
	case USB_REQUEST_TYPE_STANDARD:
		switch (req->bRequest){
		case GET_DESCRIPTOR:
			if((req->wValue >> 8) == HID_REPORT_DESCRIPTOR){
				len = sizeof(reportDescriptor);
				pbuf = reportDescriptor;
			}
			else if((req->wValue >> 8) == HID_DESCRIPTOR){
				pbuf = tusbdFindDescriptor(pdevice, HID_DESCRIPTOR);
				if(pbuf != NULL){
					p   = pdevice->devData;
					len = pbuf[0];
					for(i = 0 ; i < len && i < 32 ; i++)
						*p++ = *pbuf++;
					pbuf = pdevice->devData;
				}
			}
			if(len > 0){
				if(len > req->wLength)
					len = req->wLength;
				tusbdControlSendData(pdevice, (uint8_t *)pbuf, len);
			}
			break;
		case GET_INTERFACE:
			pdevice->devData[0] = hhid->AltSetting;
			tusbdControlSendData(pdevice, (uint8_t *)pdevice->devData, 1);
			break;
		case SET_INTERFACE:
			hhid->AltSetting = (uint8_t)(req->wValue);
			break;
		}
		break;
	case USB_REQUEST_TYPE_CLASS:
		switch (req->bRequest){
		case USB_SET_PROTOCOL:
			hhid->Protocol = (uint8_t)(req->wValue);
			break;
		case USB_GET_PROTOCOL:
			pdevice->devData[0] = hhid->Protocol;
			tusbdControlSendData(pdevice, (uint8_t *)pdevice->devData, 1);
			break;
		case USB_SET_IDLE:
			hhid->IdleState = (uint8_t)(req->wValue >> 8);
			break;
		case USB_GET_IDLE:
			pdevice->devData[0] = hhid->IdleState;
			tusbdControlSendData(pdevice, (uint8_t *)pdevice->devData, 1);
			break;
		case USB_GET_REPORT:
			len = sizeof(reportDescriptor);
			pbuf = reportDescriptor;
			if(len > req->wLength)
				len = req->wLength;
			tusbdControlSendData(pdevice, (uint8_t *)pbuf, len);
			break;
		case USB_SET_REPORT:
			if(hhid->devReportLength > 0){
				hhid->devReport[0] = req->wValue & 0xff;
				len = hhid->devReportLength;
				if(len > (req->wLength + 1))
					len = req->wLength + 1;
				p = pdevice->devData;
				for(i = 0 ; i < len && i < 32 ; i++)
					*p++ = hhid->devReport[i+1];
				tusbdControlSendData(pdevice, pdevice->devData, len);
				break;
			}
		default:
			tusbdControlSendStall(pdevice, req);
			return;
		}
		break;
	}
}

/*
 *  USB DEVICE HIDクラスデータイン
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 epnum:    Endpoint#
 *  return     TUSBD_ERCODE
 */
void
tusbdHidDataIn(TUSBD_Handle_t *pdevice, uint8_t epnum)
{
	((TUSBD_HID_Handle_t *)pdevice->pClassData)->datastate = HID_DATA_IDLE;
}

/*
 *  USB DEVICE HIDクラスデータ送信
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 pdata:    送信データ
 *  parameter3 len:      送信データ長
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdHidSendData(TUSBD_Handle_t *pdevice, uint8_t *pdata, uint16_t len)
{
	TUSBD_HID_Handle_t *hhid = (TUSBD_HID_Handle_t*)pdevice->pClassData;
    uint32_t i;

	if(pdevice->dev_state == TUSBD_STATE_CONFIGURED){
		if(hhid->datastate == HID_DATA_IDLE){
			hhid->datastate = HID_DATA_BUSY;
			for(i = 0 ; i < len && i < 32 ; i++)
				hhid->hidData[i] = *pdata++;
			tusbdDriverStartTransmit(pdevice, EPIN_ADDR, hhid->hidData, i);
			return TUSBD_E_OK;
		}
	}
	return TUSBD_E_BUSY;
}

/*
 *  USB DEVICE SETUP REPORT
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 report:   送信リポートデータ
 *  parameter3 len:      送信リポートデータ長
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdHidSetupReport(TUSBD_Handle_t *pdevice, uint8_t *report, uint16_t len)
{
	TUSBD_HID_Handle_t *hhid = (TUSBD_HID_Handle_t*)pdevice->pClassData;
	uint32_t i;

	for(i = 0 ; i < len && i < (NUM_MAX_OUTREPORT-1) ; i++)
		hhid->devReport[i+1] = *report++;
	hhid->devReportLength = i;
	return 0;
}


