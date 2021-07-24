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
 *  $Id: usbdtest.c 2416 2017-05-28 11:02:20Z roi $
 */

/* 
 *  USB DEVICEテストの本体
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
#include "usbdtest.h"

#include "tusbd_base.h"
#include "tusbd_device.h" 

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))

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

USBD_HandleTypeDef USBD_Device;
uint8_t ukey;

uint32_t heap_param[2] = {
	(uint32_t)SRAM2_BASE,
	SRAM2_SIZE
};

/*
 *  SW1割込み
 */
void sw_int(void)
{
	ukey = 1;
	syslog_0(LOG_NOTICE, "## sw_int() ##");
}

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


/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	USB_OTG_Init_t USB_Data_Init;
	USB_OTG_Handle_t *pusb;
	ER_UINT	ercd;
	USBD_StatusTypeDef result;
	uint32_t tmp;

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
	/* Enable Power Clock*/
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR1), RCC_APB1ENR1_PWREN);
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR1));

	/* Enable VddUSB */
	sil_orw_mem((uint32_t *)(TADR_PWR_BASE+TOFF_PWR_CR2), PWR_CR2_USV);

    USB_Data_Init.usb_otg_mode = USB_OTG_MODE_DEVICE;
	USB_Data_Init.dev_endpoints = 4;		/* DEV */
	USB_Data_Init.use_dedicated_ep1 = 0;	/* DEV */
	USB_Data_Init.dma_enable = 0;
//  USB_Data_Init.low_power_enable = 1;
	USB_Data_Init.low_power_enable = 0;
	USB_Data_Init.phy_itface = USB_PHY_EMBEDDED;
	USB_Data_Init.sof_enable = 0;
	USB_Data_Init.speed = USB_SPEED_FULL;
	USB_Data_Init.vbus_sensing_enable = 0;	/* HOST/DEV */
	USB_Data_Init.lpm_enable = 0;
	USB_Data_Init.use_external_vbus = 0;
	pusb = usbo_init(USB1_PORTID, &USB_Data_Init);

	/*
	 *  USB DEVICEミドルウェア初期化
	 */
	USBD_Device.pSysData = pusb;
	result = tusbdInit(&USBD_Device, 0);
	syslog_1(LOG_NOTICE, "## USBD_Init result(%d) ##", result);
	MakeUsbDescriptor(&USBD_Device);

#if USB_DEVICE_ID != 0
	USBD_Device.pUsrData = CdcDeviceRequest;
#endif

	/*
	 *  USB DEVICEスタート
	 */
	result = tusbdStart(&USBD_Device);
	syslog_1(LOG_NOTICE, "## USBD_Start result(%d) ##", result);

#if 0	/* ROI DEBUG */
	while (1){
		dly_tsk(1000 /*200*/);
		GetPointerData(HID_Buffer);
		USBD_HID_SendReport(&USBD_Device, HID_Buffer, 4);
		syslog_1(LOG_NOTICE, "## dev_state(%d) ##", USBD_Device.dev_state);
	}
#else	/* ROI DEBUG */
	act_tsk(USBD_TASK);
#endif	/* ROI DEBUG */

#if 1	/* ROI DEBUG */
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
#endif	/* ROI DEBUG */

	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}


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

