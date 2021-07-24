/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id: usbotest.c 2416 2012-09-07 08:06:20Z ertl-hiro $
 */

/* 
 *  USB OTGテストの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include <target_syssvc.h>
#include "device.h"
#include "usb_otg.h"

#include "usbotest.h"

#include "tusbd_base.h"
#if USB_OTG_MODE != USB_OTG_MODE_HOST
#include "tusbd_device.h" 
#endif

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

#define CURSOR_STEP     5

USBH_HandleTypeDef hUSBHost;
USBD_HandleTypeDef USBD_Device;

uint8_t ukey;
uint8_t device_state[16];
Line_Coding_t CDC_Info = {
	115200,
	0,
	0,
	8
};
uint8_t send_buffer[] = "help\r\nhelp\r\n";
uint8_t receive_buffer[512] __attribute__ ((aligned (32)));
uint8_t scratch[512];

uint8_t HID_Buffer[4];

static uint32_t heap_area[4*1024];

uint32_t heap_param[2] = {
	(uint32_t)heap_area,
	(4*4*1024)
};

/*
 *  SW1割込み
 */
void sw_int(void)
{
	ukey = 1;
	syslog_0(LOG_NOTICE, "## sw_int() ##");
}


#if USB_OTG_MODE != USB_OTG_MODE_DEVICE
/*
 *  USB HOST USER CALLBACK関数
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, TUSBH_Device_t *pdevice, uint8_t id)
{
	device_state[pdevice->type] = id;
	if(pdevice->pClass != NULL){
		syslog_3(LOG_NOTICE, "## CLASS[%s](%d)(%d) ##", pdevice->pClass->Name, pdevice->type, id);
	}
}

/*
 *  MSC SECTOR READ
 */
static void MSC_Read_Operations(void){
	TUSBH_ERCODE  status = TUSBH_E_OK;
	char pdrv = 0;

	if(tusbhMscUnitIsReady (&hUSBHost, pdrv)){
		syslog_0(LOG_NOTICE, "READY");
	}
	else{
		syslog_0(LOG_NOTICE, "NOT READY");
	}
	status = tusbhMscRead(&hUSBHost, pdrv, 0, (uint8_t *)scratch, 1);
	syslog_1(LOG_NOTICE, "## status(%d) ##", status);
}

/*
 *  HID CALLBACK関数
 */
static void HID_Callback(TUSBH_Device_t *pdevice, uint8_t length, uint8_t *buffer)
{
	void *phandle = pdevice->pData;
	uint8_t type = tusbhHidGetType(pdevice);
	if(length == 0){
		if(type == HID_KEYBOARD_TYPE)
			HidKeyboardInit(phandle);
		else if(type == HID_MOUSE_TYPE)
			HidMouseInit(phandle);
	}
	else{
#if 1	/* ROI DEBUG */
		uint8_t *p = buffer;
		syslog_5(LOG_NOTICE, "## HID_Callback len(%d) [%02x][%02x][%02x][%02x] ##", length, p[0], p[1], p[2], p[3]);
#endif	/* ROI DEBUG */

		if(type == HID_KEYBOARD_TYPE){
			KEYBOARD_Info_t k_info;
			if(HidGetKeyboardInfo(phandle, &k_info, buffer, length) == TUSBH_E_OK){
				char c = HidGetASCIICode(&k_info);
				if(c != 0){
					syslog_1(LOG_NOTICE, "## KEY[%02x] ##", c);
				}
			}
		}
		else if(type == HID_MOUSE_TYPE){
			MOUSE_Info_t m_info;
			if(HidMouseInfo(phandle, &m_info, buffer, length) == TUSBH_E_OK){
				syslog_3(LOG_NOTICE, "button left(%d) right(%d) middle(%d)", m_info.button_left, m_info.button_right, m_info.button_middle);
				syslog_3(LOG_NOTICE, "position x (%d) y    (%d) z     (%d)", m_info.x, m_info.y, m_info.z);
			}
		}
	}
}

/*
 *  CDC CALLBACK関数
 */
static void CDC_Callback(TUSBH_Device_t *pdevice, uint8_t type)
{
	syslog_2(LOG_NOTICE, "## CDC_Callback[%08x](%d) ##", pdevice, type);
}
#endif

#if USB_OTG_MODE != USB_OTG_MODE_HOST
#if USB_DEVICE_ID == 0

uint8_t HID_Buffer[4];

/*
 *  HID MOUSE DEVICE CLASS REQUEST
 *  parameter1:  pbuf  データ設定バッファ
 */
static void GetPointerData(uint8_t *pbuf)
{
  static int8_t cnt = 0;
  int8_t  x = 0, y = 0 ;
  
  if(cnt++ > 0)
  {
    x = CURSOR_STEP;
  }
  else
  {
    x = -CURSOR_STEP;
  }
  pbuf[0]  = 0;
  pbuf[1] += x;
  pbuf[2] += y;
  pbuf[3]  = 0;
}
#else

#define APP_TX_DATA_SIZE   2048

#define FLG_CDC_SET_LINE_CODING         0x0001
#define FLG_CDC_GET_LINE_CODING         0x0002
#define FLG_CDC_SET_CONTROL_LINE_STATE  0x0004
#define FLG_CDC_SEND_BREAK              0x0008
#define FLG_CDC_RECEIVE_DATA           	0x0010
#define FLG_CDC_SEND_FLUSH              0x0100

static uint8_t    txBuffer[APP_TX_DATA_SIZE] __attribute__ ((aligned (32)));/* Received Data over UART (CDC interface) are stored in this buffer */
static uint8_t    *pTransRxBuffer;
static uint16_t   TransRxLength;
static uint16_t   TransRxCount;


Line_Coding_t LineCoding =
{
	115200, /* baud rate*/
	0x00,   /* stop bits-1*/
	0x00,   /* parity - none*/
	0x08    /* nb. of bits 8*/
};

/*
 *  CDC DEVICE CLASS REQUEST
 *  parameter1:  hcdc   CDCハンドラ
 *  parameter2:  cmd    要求コマンド
 *  parameter3:  length データ長
 */
static void CdcDeviceRequest(TUSBD_CDC_Handle_t *hcdc, uint8_t cmd, uint32_t length)
{
	switch (cmd){
	case CDC_SEND_ENCAPSULATED_COMMAND:
		syslog_2(LOG_NOTICE, "## CdcDeviceRequest CDC_SEND_ENCAPSULATED_COMMAND hcdc[%08x] length(%d) ##", hcdc, length);
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		syslog_2(LOG_NOTICE, "## CdcDeviceRequest CDC_GET_ENCAPSULATED_RESPONSE hcdc[%08x] length(%d) ##", hcdc, length);
		break;

	case CDC_SET_COMM_FEATURE:
		syslog_2(LOG_NOTICE, "## CdcDeviceRequest CDC_SET_COMM_FEATURE hcdc[%08x] length(%d) ##", hcdc, length);
		break;

	case CDC_GET_COMM_FEATURE:
		syslog_2(LOG_NOTICE, "## CDC_GET_COMM_FEATURE hcdc[%08x] length(%d) ##", hcdc, length);
		break;

	case CDC_CLEAR_COMM_FEATURE:
		syslog_2(LOG_NOTICE, "## CDC_CLEAR_COMM_FEATURE hcdc[%08x] length(%d) ##", hcdc, length);
		break;

	case CDC_SET_LINE_CODING:
		memcpy(&LineCoding, hcdc->cmddata, sizeof(Line_Coding_t));
		iset_flg(USBCDC_FLG, FLG_CDC_SET_LINE_CODING);
		break;

	case CDC_GET_LINE_CODING:
		memcpy(hcdc->cmddata, &LineCoding, sizeof(Line_Coding_t));
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		iset_flg(USBCDC_FLG, FLG_CDC_SET_CONTROL_LINE_STATE);
		break;

	case CDC_SEND_BREAK:
		iset_flg(USBCDC_FLG, FLG_CDC_SEND_BREAK);
		break;

	case CDC_RECEIVED:
		pTransRxBuffer = hcdc->rxdata;
		TransRxLength = length;
		TransRxCount  = 0;
		iset_flg(USBCDC_FLG, FLG_CDC_RECEIVE_DATA);
		break;

	default:
		syslog_3(LOG_NOTICE, "## CdcDeviceRequest cmd[%02x] hcdc[%08x] length(%d) ##", cmd, hcdc, length);
		break;
	}
}
#endif
#endif

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	USB_OTG_Init_t USB_Data_Init;
	USB_OTG_Handle_t *husb;
	ER_UINT	ercd;
	TUSBH_ERCODE result;
#if USB_OTG_MODE != USB_OTG_MODE_DEVICE
	Line_Coding_t CDC_data;
	uint32_t length;
	uint32_t count = 0;
#endif

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	/*
	 *  USB OTG初期化
	 */
    USB_Data_Init.usb_otg_mode = USB_OTG_MODE;
	USB_Data_Init.host_channels = 11;	/* HOST */
	USB_Data_Init.dev_endpoints = 4;		/* DEV */
	USB_Data_Init.use_dedicated_ep1 = 0;	/* DEV */
	USB_Data_Init.dma_enable = 0;
	USB_Data_Init.low_power_enable = 0;
	USB_Data_Init.phy_itface = USB_PHY_EMBEDDED; 
	USB_Data_Init.sof_enable = 0;
	USB_Data_Init.speed = USB_SPEED_FULL;
#if USB_OTG_MODE != USB_OTG_MODE_HOST
	USB_Data_Init.vbus_sensing_enable = 1;	/* HOST/DEV */
#else
	USB_Data_Init.vbus_sensing_enable = 0;	/* HOST */
#endif
	USB_Data_Init.lpm_enable = 0;
	USB_Data_Init.use_external_vbus = 0;
	husb = usbo_init(USB1_PORTID, &USB_Data_Init);
#if 1	/* ROI DEBUG */
	if(husb == NULL){
		syslog_0(LOG_NOTICE, "## usbo_init error STOP ##");
		slp_tsk();
	}
#endif	/* ROI DEBUG */

#if USB_OTG_MODE != USB_OTG_MODE_HOST
	/*
	 *  USB DEVICEミドルウェア初期化
	 */
	USBD_Device.pSysData = husb;
	result = tusbdInit(&USBD_Device, USB_DEVICE_ID);
	syslog_1(LOG_NOTICE, "## tusbdInit result(%d) ##", result);
	MakeUsbDescriptor(&USBD_Device);

#if USB_DEVICE_ID != 0
	USBD_Device.pUsrData = CdcDeviceRequest;
#endif

	/*
	 *  USB DEVICEスタート
	 */
	result = tusbdStart(&USBD_Device);
	syslog_1(LOG_NOTICE, "## tusbdStart result(%d) ##", result);
	act_tsk(USBD_TASK);
#endif

#if USB_OTG_MODE != USB_OTG_MODE_DEVICE
	/*
	 *  USB HOSTミドルウェア設定
	 */
	hUSBHost.pSysData = husb;
	tusbhInit(&hUSBHost, USBH_UserProcess, 0);

	tusbhSetHidCallBack(&hUSBHost, HID_Callback);
	tusbhSetCdcCallBack(&hUSBHost, CDC_Callback);

	/*
	 *  USB HOSTスタート
	 */
	result = tusbhStart(&hUSBHost);
	syslog_1(LOG_NOTICE, "## tusbhStart result(%d) ##", result);

	/*
	 *  USB HOSTアプリ処理
	 */
	while (1){
		if(device_state[MSC_CLASS] == HOST_USER_CLASS_ACTIVE){
			syslog_0(LOG_NOTICE, "Press User button to start");
			syslog_0(LOG_NOTICE, "read and write operations");
			/* KEY待ち */
			ukey = 0;
			while((ukey != 1) && (device_state[MSC_CLASS] != HOST_USER_DISCONNECTION)){
				dly_tsk(1);
			}
			MSC_Read_Operations();
			dly_tsk(400);
		}

		if(device_state[MSC_CLASS] == HOST_USER_DISCONNECTION){
			device_state[MSC_CLASS] = 0;
			syslog_0(LOG_ERROR, "MSC device disconnected!\n");
		}

		/*
		 *  CDC SERIAL処理
		 */
		if(device_state[CDC_CLASS] == HOST_USER_CLASS_ACTIVE){
			static bool_t cdc_init = true;
			if(cdc_init){
				syslog_1(LOG_NOTICE, "## device_state(%d) ##", device_state[CDC_CLASS]);
				tusbhCdcSetLineCoding(&hUSBHost, 0, &CDC_Info);
				dly_tsk(100);
				tusbhCdcGetLineCoding(&hUSBHost, 0, &CDC_data);
				result = tusbhCdcSend(&hUSBHost, 0, send_buffer, sizeof(send_buffer));
				syslog_4(LOG_NOTICE, "CDC INFO baud(%d) format(%d) parity(%d) data(%d)", CDC_data.baudrate, CDC_data.stop_bits, CDC_data.parity, CDC_data.data_bits);
				syslog_1(LOG_NOTICE, "## send result(%d) ##", result);
				cdc_init = false;
			}
			length = 512;
			result = tusbhCdcReceive(&hUSBHost, 0, receive_buffer, &length, 500);
			syslog_3(LOG_NOTICE, "## tusbhCdcReceive status(%d) length(%d) buffer[%08x] ##", result, length, receive_buffer);
			if(length > 0){
				syslog_4(LOG_NOTICE, "## 00[%02x] 01[%02x] 02[%02x] 03[%02x] ##", receive_buffer[0], receive_buffer[1], receive_buffer[2], receive_buffer[3]);
				syslog_4(LOG_NOTICE, "## 04[%02x] 05[%02x] 06[%02x] 07[%02x] ##", receive_buffer[4], receive_buffer[5], receive_buffer[6], receive_buffer[7]);
				syslog_4(LOG_NOTICE, "## 08[%02x] 09[%02x] 10[%02x] 11[%02x] ##", receive_buffer[8], receive_buffer[9], receive_buffer[10], receive_buffer[11]);
				syslog_4(LOG_NOTICE, "## 12[%02x] 13[%02x] 14[%02x] 15[%02x] ##", receive_buffer[12], receive_buffer[13], receive_buffer[14], receive_buffer[15]);
			}
			if(length != 0 && count == 0){
				dly_tsk(1000);
				cdc_init = true;
			}
			count++;
		}
#if 1	/* ROI DEBUG */
		/*
		 *  PRINTER処理
		 */
		if(device_state[PRT_CLASS] == HOST_USER_CLASS_ACTIVE){
			static bool_t prt_init = true;
			if(prt_init){
				syslog_1(LOG_NOTICE, "## [PRT]device_state(%d) ##", device_state[PRT_CLASS]);
				dly_tsk(1000);
				syslog_0(LOG_NOTICE, "## SEND EOJ ##");
				tusbhPrtEOJ(&hUSBHost, 0, 1, 2, 3);
				tusbhPrtPortID(&hUSBHost, 0, receive_buffer);
				syslog_1(LOG_NOTICE, "## PORTID(%d) ##", receive_buffer[0]);
				prt_init = false;
			}
		}
#endif	/* ROI DEBUG */

		dly_tsk(500);
	}
#endif
#if 1	/* ROI DEBUG */
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
#endif	/* ROI DEBUG */

	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}

#if USB_OTG_MODE != USB_OTG_MODE_HOST
void usbd_task(intptr_t exinf)
{
	FLGPTN crcflag;
	ER ercd;

	while (1){
		ercd = twai_flg(USBCDC_FLG, 0xffff, TWF_ORW, &crcflag, 1000);
#if USB_DEVICE_ID == 0
		GetPointerData(HID_Buffer);
		if(USBD_HID_SendReport(&USBD_Device, HID_Buffer, 4) == TUSBD_E_OK)
			memset(&HID_Buffer, 0, sizeof(HID_Buffer));
#else
		if(ercd == E_OK){
			if((crcflag & FLG_CDC_RECEIVE_DATA) != 0 && TransRxLength > TransRxCount){
				pTransRxBuffer[TransRxLength] = 0;
				syslog_2(LOG_NOTICE, "## revlen(%d)[%s] ##", TransRxLength, pTransRxBuffer);
				memcpy(txBuffer, pTransRxBuffer, TransRxLength);
				tusbdCdcStartTransmit(&USBD_Device, txBuffer, TransRxLength);
				TransRxLength = 0;
				tusbdCdcSetReceivePacket(&USBD_Device);
			}
		}
#endif
		syslog_1(LOG_NOTICE, "## dev_state(%d) ##", USBD_Device.dev_state);
	}
}
#endif

