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
 *  @(#) $Id: tusbd_serial.c 698 2017-10-15 15:14:19Z roi $
 */
/*
 *  USB Device Middleware CDC:SERIAL CLASS部
 */

#include "tusbd_serial.h"


#define CDC_INTERVAL_TIME    16

/*
 *  USB CDC HS DEVICE CONFIGURATION DESCRIPTOR
 */
uint8_t configurationCdcHsDescriptor[TOTAL_CDC_DESCRIPTOR_LENGTH] __attribute__ ((aligned (USB_DATA_ALIGN))) =
{
	CONFIGURATION_DESCRIPTOR_LENGTH,	// bLength
	CONFIGURATION_DESCRIPTOR,			// bDescriptorType
	(TOTAL_CDC_DESCRIPTOR_LENGTH & 0xFF),   // wTotalLength (LSB)
	(TOTAL_CDC_DESCRIPTOR_LENGTH >> 8),     // wTotalLength (MSB)
	0x02,                               // bNumInterfaces
	0x01,                               // bConfigurationValue
	0x00,                               // iConfiguration
	C_RESERVED | C_SELF_POWERED,        // bmAttributes
	C_POWER(100),                       // bMaxPower

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x00,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x01,                               // bNumEndpoints
	CDC_CLASS,                          // bInterfaceClass
	0x02,                               // bInterfaceSubClass
	0x01,                               // bInterfaceProtocol
	0x00,                               // iInterface

	/* CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26 */
 	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x00,                               // bDescriptorSubtype
	0x10, 0x01,                         // bcdCDC
	/* Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x01,                               // bDescriptorSubtype
	0x03,                               // bmCapabilities
	0x01,                               // bDataInterface
	/* Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28 */
	0x04,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x02,                               // bDescriptorSubtype
	0x06,                               // bmCapabilities
	/* Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x06,                               // bDescriptorSubtype
	0x00,                               // bMasterInterface
	0x01,                               // bSlaveInterface0

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPINT_ADDR,                     // bEndpointAddress
	USB_EP_TYPE_INTR,                   // bmAttributes
	(CDC_PACKET_SIZE_EPINT & 0xFF),     // wMaxPacketSize (LSB)
	(CDC_PACKET_SIZE_EPINT >> 8),       // wMaxPacketSize (MSB)
	CDC_INTERVAL_TIME,                  // bInterval

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x01,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x02,                               // bNumEndpoints
	DATA_INTERFACE_CLASS,               // bInterfaceClass
	0x00,                               // bInterfaceSubClass
	0x00,                               // bInterfaceProtocol
	0x00,                               // iInterface

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKOUT_ADDR,                 // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_HS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_HS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00,                               // bInterval (milliseconds)

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKIN_ADDR,                  // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_HS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_HS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00                                // bInterval (milliseconds)
};

/*
 *  USB CDC FS DEVICE CONFIGURATION DESCRIPTOR
 */
uint8_t configurationCdcFsDescriptor[TOTAL_CDC_DESCRIPTOR_LENGTH] __attribute__ ((aligned (USB_DATA_ALIGN))) =
{
  /*Configuration Descriptor*/
	CONFIGURATION_DESCRIPTOR_LENGTH,	// bLength
	CONFIGURATION_DESCRIPTOR,			// bDescriptorType
	(TOTAL_CDC_DESCRIPTOR_LENGTH & 0xFF),   // wTotalLength (LSB)
	(TOTAL_CDC_DESCRIPTOR_LENGTH >> 8),     // wTotalLength (MSB)
	0x02,                               // bNumInterfaces
	0x01,                               // bConfigurationValue
	0x00,                               // iConfiguration
	C_RESERVED | C_SELF_POWERED,        // bmAttributes
	C_POWER(100),                       // bMaxPower

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x00,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x01,                               // bNumEndpoints
	CDC_CLASS,                          // bInterfaceClass
	0x02,                               // bInterfaceSubClass
	0x01,                               // bInterfaceProtocol
	0x00,                               // iInterface

	/* CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26 */
 	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x00,                               // bDescriptorSubtype
	0x10, 0x01,                         // bcdCDC
	/* Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x01,                               // bDescriptorSubtype
	0x03,                               // bmCapabilities
	0x01,                               // bDataInterface
	/* Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28 */
	0x04,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x02,                               // bDescriptorSubtype
	0x06,                               // bmCapabilities
	/* Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x06,                               // bDescriptorSubtype
	0x00,                               // bMasterInterface
	0x01,                               // bSlaveInterface0

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPINT_ADDR,                     // bEndpointAddress
	USB_EP_TYPE_INTR,                   // bmAttributes
	(CDC_PACKET_SIZE_EPINT & 0xFF),     // wMaxPacketSize (LSB)
	(CDC_PACKET_SIZE_EPINT >> 8),       // wMaxPacketSize (MSB)
	CDC_INTERVAL_TIME,                  // bInterval

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x01,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x02,                               // bNumEndpoints
	DATA_INTERFACE_CLASS,               // bInterfaceClass
	0x00,                               // bInterfaceSubClass
	0x00,                               // bInterfaceProtocol
	0x00,                               // iInterface

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKOUT_ADDR,                 // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_FS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_FS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00,                               // bInterval (milliseconds)

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKIN_ADDR,                  // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_FS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_FS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00                                // bInterval (milliseconds)
} ;

/*
 *  USB CDC OTHER DEVICE CONFIGURATION DESCRIPTOR
 */
uint8_t configurationCdcOtrDescriptor[TOTAL_CDC_DESCRIPTOR_LENGTH] __attribute__ ((aligned (USB_DATA_ALIGN))) =
{
	CONFIGURATION_DESCRIPTOR_LENGTH,	// bLength
	OTHER_SPEED_CONFIGURATION_DESC,     // bDescriptorType
	(TOTAL_CDC_DESCRIPTOR_LENGTH & 0xFF),   // wTotalLength (LSB)
	(TOTAL_CDC_DESCRIPTOR_LENGTH >> 8),     // wTotalLength (MSB)
	0x02,                               // bNumInterfaces
	0x01,                               // bConfigurationValue
	0x04,                               // iConfiguration
	C_RESERVED | C_SELF_POWERED,        // bmAttributes
	C_POWER(100),                       // bMaxPower

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x00,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x01,                               // bNumEndpoints
	CDC_CLASS,                          // bInterfaceClass
	0x02,                               // bInterfaceSubClass
	0x01,                               // bInterfaceProtocol
	0x00,                               // iInterface

	/* CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26 */
 	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x00,                               // bDescriptorSubtype
	0x10, 0x01,                         // bcdCDC
	/* Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x01,                               // bDescriptorSubtype
	0x03,                               // bmCapabilities
	0x01,                               // bDataInterface
	/* Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28 */
	0x04,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x02,                               // bDescriptorSubtype
	0x06,                               // bmCapabilities
	/* Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33 */
	0x05,                               // bFunctionLength
	0x24,                               // bDescriptorType
	0x06,                               // bDescriptorSubtype
	0x00,                               // bMasterInterface
	0x01,                               // bSlaveInterface0

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPINT_ADDR,                     // bEndpointAddress
	USB_EP_TYPE_INTR,                   // bmAttributes
	(CDC_PACKET_SIZE_EPINT & 0xFF),     // wMaxPacketSize (LSB)
	(CDC_PACKET_SIZE_EPINT >> 8),       // wMaxPacketSize (MSB)
	CDC_INTERVAL_TIME,                  // bInterval

	INTERFACE_DESCRIPTOR_LENGTH,        // bLength
	INTERFACE_DESCRIPTOR,               // bDescriptorType
	0x01,                               // bInterfaceNumber
	0x00,                               // bAlternateSetting
	0x02,                               // bNumEndpoints
	DATA_INTERFACE_CLASS,               // bInterfaceClass
	0x00,                               // bInterfaceSubClass
	0x00,                               // bInterfaceProtocol
	0x00,                               // iInterface

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKOUT_ADDR,                 // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_FS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_FS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00,                               // bInterval (milliseconds)

	ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
	ENDPOINT_DESCRIPTOR,                // bDescriptorType
	CDC_EPBULKIN_ADDR,                  // bEndpointAddress
	USB_EP_TYPE_BULK,                   // bmAttributes
	(MAX_PACKET_SIZE_FS_EPBULK & 0xFF), // wMaxPacketSize (LSB)
	(MAX_PACKET_SIZE_FS_EPBULK >> 8),   // wMaxPacketSize (MSB)
	0x00                                // bInterval (milliseconds)
};

static TUSBD_CDC_Handle_t CDC_CLASS_DATA  __attribute__ ((aligned (USB_DATA_ALIGN)));


/*
 *  USB DEVICE CDC初期化
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 idx:      Configuration index
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdCdcInit(TUSBD_Handle_t *pdevice, uint8_t idx)
{
	TUSBD_CDC_Handle_t *hcdc;

	memset(&CDC_CLASS_DATA, 0, sizeof(TUSBD_CDC_Handle_t));
	pdevice->pClassData = &CDC_CLASS_DATA;
	hcdc = (TUSBD_CDC_Handle_t*) pdevice->pClassData;
	if(pdevice->dev_speed == USB_DEVICE_SPEED_HIGH)
		hcdc->packetsize = MAX_PACKET_SIZE_HS_EPBULK;
	else
		hcdc->packetsize = MAX_PACKET_SIZE_FS_EPBULK;
	hcdc->TxState = 0;
	hcdc->CmdCode = 0xFF;

	/*
	 *  ENDPOINT OPEN
	 */
	tusbdDriverOpenEp(pdevice, CDC_EPBULKIN_ADDR, USB_EP_TYPE_BULK, hcdc->packetsize);
	tusbdDriverOpenEp(pdevice, CDC_EPBULKOUT_ADDR, USB_EP_TYPE_BULK, hcdc->packetsize);
	tusbdDriverOpenEp(pdevice, CDC_EPINT_ADDR, USB_EP_TYPE_INTR, CDC_PACKET_SIZE_EPINT);

	/*
	 *  SETUP RECEIVED PACKET AREA
	 */
	tusbdCdcSetReceivePacket(pdevice);
	return TUSBD_E_OK;
}

/*
 *  USB DEVICE HIDクラス終了
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 idx:      Configuration index
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdCdcDeInit(TUSBD_Handle_t *pdevice, uint8_t cfgidx)
{
	/*
	 * ENDPOINT CLOSE
	 */
	tusbdDriverCloseEp(pdevice, CDC_EPBULKIN_ADDR);
	tusbdDriverCloseEp(pdevice, CDC_EPBULKOUT_ADDR);
	tusbdDriverCloseEp(pdevice, CDC_EPINT_ADDR);
	return TUSBD_E_OK;
}

/*
 *  USB DEVICE CDCクラス EP0受信READY
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  return     TUSBD_ERCODE
 */
void
tusbdCdcEP0RxReady(TUSBD_Handle_t *pdevice)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*)pdevice->pClassData;
	void (*func)(TUSBD_CDC_Handle_t *, uint8_t, uint32_t);

	if(((func = pdevice->pUsrData) != NULL) && (hcdc->CmdCode != 0xFF)){
		func(hcdc, hcdc->CmdCode, hcdc->CmdLength);
		hcdc->CmdCode = 0xFF;
	}
}

/*
 *  USB DEVICE CDCクラスセットアップ
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 req:      usb control requests
 *  return     TUSBD_ERCODE
 */
void
tusbdCdcSetup(TUSBD_Handle_t *pdevice, UsbSetupRequest *req)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*) pdevice->pClassData;
	void (*func)(TUSBD_CDC_Handle_t *, uint8_t, uint32_t);

	func = pdevice->pUsrData;
	switch (req->bmRequest & USB_REQUEST_TYPE_MASK){
	case USB_REQUEST_TYPE_CLASS:
		if(req->wLength){
			if(req->bmRequest & USB_DEVICE_TO_HOST){
				if(func != NULL)
					func(hcdc, req->bRequest, req->wLength);
				tusbdControlSendData(pdevice, (uint8_t *)hcdc->cmddata, req->wLength);
			}
			else{
				hcdc->CmdCode   = req->bRequest;
				hcdc->CmdLength = req->wLength;
				tusbdControlReceiveStart(pdevice, (uint8_t *)hcdc->cmddata, req->wLength);
			}
		}
		else if(func != NULL)
			func(hcdc, req->bRequest, 0);
		break;
	case USB_REQUEST_TYPE_STANDARD:
		switch(req->bRequest){
		case GET_INTERFACE:
			pdevice->devData[0] = 0;
			tusbdControlSendData(pdevice, (uint8_t *)pdevice->devData, 1);
			break;
		case SET_INTERFACE:
		default:
			break;
		}
	default:
		break;
	}
}

/*
 *  USB DEVICE CDCクラスデータイン
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 epnum:    Endpoint#
 *  return     TUSBD_ERCODE
 */
void
tusbdCdcDataIn(TUSBD_Handle_t *pdevice, uint8_t epnum)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*) pdevice->pClassData;

	if(pdevice->pClassData != NULL){
		hcdc->TxState = 0;
	}
}

/*
 *  USB DEVICE CDCクラスデータアウト
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 epnum:    Endpoint#
 *  return     TUSBD_ERCODE
 */
void
tusbdCdcDataOut(TUSBD_Handle_t *pdevice, uint8_t epnum)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*) pdevice->pClassData;
	void (*func)(TUSBD_CDC_Handle_t *, uint8_t, uint32_t);

	hcdc->RxLength = tusbdDriverGetRxDataSize(pdevice, epnum);
	if((func = pdevice->pUsrData) != NULL){
		func(hcdc, CDC_RECEIVED, hcdc->RxLength);
	}
}

/*
 *  USB DEVICE CDC START SENS DATA
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  parameter2 pbuff:    送信データ
 *  parameter2 length:   送信データ長
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdCdcStartTransmit(TUSBD_Handle_t *pdevice, uint8_t *pbuff, uint32_t length)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*)pdevice->pClassData;

	if(pdevice->pClassData != NULL){
		if(hcdc->TxState == 0){
			hcdc->TxState = 1;
			hcdc->TxBuffer = pbuff;
			hcdc->TxLength = length;

			tusbCpuLock();
			tusbdDriverStartTransmit(pdevice, CDC_EPBULKIN_ADDR, hcdc->TxBuffer, hcdc->TxLength);
			tusbCpuUnLock();
			return TUSBD_E_OK;
		}
		else
			return TUSBD_E_BUSY;
	}
	else
		return TUSBD_E_ERR;
}

/*
 *  USB DEVICE CDC SETUP RECEVE PACKET
 *  parameter1 pdevice:  USB DEVICEハンドラ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
tusbdCdcSetReceivePacket(TUSBD_Handle_t *pdevice)
{
	TUSBD_CDC_Handle_t *hcdc = (TUSBD_CDC_Handle_t*)pdevice->pClassData;

	if(pdevice->pClassData != NULL){
		tusbCpuLock();
		tusbdDriverSetupReceive(pdevice, CDC_EPBULKOUT_ADDR, hcdc->rxdata, hcdc->packetsize);
		tusbCpuUnLock();
		return TUSBD_E_OK;
	}
	else
		return TUSBD_E_ERR;
}

