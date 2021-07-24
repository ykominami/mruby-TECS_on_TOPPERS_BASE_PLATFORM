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
 *  @(#) $Id: tusbh_hid.c 698 2017-09-17 16:35:15Z roi $
 */
/*
 *  USB Host Middleware HID CLASS部
 */

#include "tusbh_hid.h"


#define HID_SETIDLE(d, dur, rep)     tusbhControlRequest((d), NULL, HID_SET_TYPE, USB_HID_SET_IDLE, ((dur << 8) | (rep)), 0, 0)
#define HID_SETPROTOCOL(d, p)        tusbhControlRequest((d), NULL, HID_SET_TYPE, USB_HID_SET_PROTOCOL, (((p) != 0) ? 0 : 1), 0, 0)
#define HID_GETREPORT(d, t, i, b, l) tusbhControlRequest((d), (b), HID_GET_TYPE, USB_HID_GET_REPORT, ((t << 8 ) | i), 0, (l))
#define HID_SETREPORT(d, t, i, b, l) tusbhControlRequest((d), (b), HID_SET_TYPE, USB_HID_SET_REPORT, ((t << 8 ) | i), 0, (l))

static TUSBH_ERCODE tusbhHIDInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhHIDDeInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbdHIDProcess(TUSBH_Device_t *pdevice, uint8_t *mes);

TUSBH_Class_t HID_Class = {
	NULL,
	"HID",
	HID_CLASS,
	tusbhHIDInit,
	tusbhHIDDeInit,
	tusbdHIDProcess,
	NULL
};

/*
 *  HIDクラスセットアップ関数
 */
void tusbhLinkHID(TUSBH_Handle_t *phost)
{
	tusbhLinkClass(phost, &HID_Class);
}

/*
 *  HIDクラス初期設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhHIDInit(TUSBH_Device_t *pdevice)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	uint8_t max_ep;
	uint8_t interface;
	TUSBH_ERCODE status = TUSBH_E_ERR;
	HID_Handle_t *hhid;

	interface = tusbhFindInterface(pdevice, pdevice->pClass->classCode, HID_BOOT_CODE, 0xFF);

	if(interface == NO_INTERFACE){
		syslog_1(LOG_ERROR, "[HID] Cannot Find the interface for %s class !", pdevice->pClass->Name);
		return TUSBH_E_ERR;
	}

	tusbhSelectInterface(pdevice, interface);
	pdevice->pData = tusbmalloc(sizeof(HID_Handle_t));
	hhid = (HID_Handle_t *)pdevice->pData;
	memset(hhid, 0, sizeof(HID_Handle_t));
	hhid->state = THID_ERROR;

	/*
	 *  デバイスの検索
	 */
	if(pdevice->ItfDesc[pdevice->sel_interface].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE){
		syslog_0(LOG_NOTICE, "[HID] found KeyBoard device !"); 
		hhid->type =  HID_KEYBOARD_TYPE;
	}
	else if(pdevice->ItfDesc[pdevice->sel_interface].bInterfaceProtocol == HID_MOUSE_BOOT_CODE){
		syslog_0(LOG_NOTICE, "[HID] found Mouse device !");
		hhid->type =  HID_MOUSE_TYPE;
	}
	else{
		syslog_0(LOG_ERROR, "[HID] no device !");
		return TUSBH_E_ERR;
	}

	hhid->ep_addr   = pdevice->EpDesc[pdevice->sel_interface][0].bEndpointAddress;
	hhid->length    = pdevice->EpDesc[pdevice->sel_interface][0].wMaxPacketSize;
	hhid->poll      = pdevice->EpDesc[pdevice->sel_interface][0].bInterval ;

	if(hhid->poll < HID_MIN_POLL)
		hhid->poll = HID_MIN_POLL;

	max_ep = pdevice->ItfDesc[pdevice->sel_interface].bNumEndpoints;
	if(max_ep == 0 || (pdevice->EpDesc[pdevice->sel_interface][0].bEndpointAddress & 0x80) == 0)
		return TUSBH_E_ERR;

#if 1	/* ROI DEBUG */
	syslog_3(LOG_NOTICE, "## max_ep(%d) length(%d) poll(%d) ##", max_ep, hhid->length, hhid->poll);
#endif	/* ROI DEBUG */
	if(hhid->length > HID_MAX_REPORT_LENGTH)
		hhid->length = HID_MAX_REPORT_LENGTH;

	/*
	 *  インターラプト インエンドポイント オープン
	 */
	hhid->InPipe  = tusbhAllocPipe(phost, hhid->ep_addr);
	tusbhOpenPipe(pdevice, hhid->InPipe, hhid->ep_addr, USB_EP_TYPE_INTR, hhid->length);
	tusbhHDSetToggle(phost, hhid->InPipe, 0);

	/*
	 *  HID DESCRIPTOR取得
	 */
	if(tusbhGetDescriptor(pdevice, (USB_RECIPIENT_INTERFACE | USB_REQUEST_TYPE_STANDARD),
						((HID_DESCRIPTOR << 8) & 0xFF00), pdevice->Data, HID_DESCRIPTOR_LENGTH) != TUSBH_E_OK)
		return TUSBH_E_ERR;
	hhid->ReportDescLength  = (pdevice->Data[7] & 0xff) | (pdevice->Data[8] << 8);
	syslog_1(LOG_NOTICE, "## hhid->DescLength(%d) ##", hhid->ReportDescLength);
	if(hhid->ReportDescLength > 64)
		hhid->ReportDescLength = 64;

	/*
	 *  HID REPORT DESCRIPTOR取得
	 */
	if(tusbhGetDescriptor(pdevice, (USB_RECIPIENT_INTERFACE | USB_REQUEST_TYPE_STANDARD),
						((HID_REPORT_DESCRIPTOR << 8) & 0xFF00), pdevice->Data, hhid->ReportDescLength) != TUSBH_E_OK)
		return TUSBH_E_ERR;
	tusbmemcpy(hhid->ReportDesc, pdevice->Data, hhid->ReportDescLength);
	syslog_1(LOG_NOTICE, "## hhid->ReportDesc[%08x] ##", hhid->ReportDesc);

	/*
	 *  IDEL設定
	 */
	status = HID_SETIDLE(pdevice, 0, 0);
	if(status != TUSBH_E_OK && status != TUSBH_E_NOSPT)
		return status;

	/*
	 *  PROTOCOL #0 設定
	 */
	if((status = HID_SETPROTOCOL(pdevice, 0)) != TUSBH_E_OK)
		return status;

	/* タイマースタート */
	hhid->state     = THID_INIT;
	pdevice->timeid = pdevice->timecount = hhid->poll;
	return TUSBH_E_OK;
}

/*
 *  HUBクラス終了設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhHIDDeInit(TUSBH_Device_t *pdevice)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	HID_Handle_t *hhid;

	hhid =  (HID_Handle_t *)pdevice->pData;
	if(hhid->InPipe != 0x00){
		tusbhClosePipe(pdevice, hhid->InPipe);
		tusbFreePipe(phost, hhid->InPipe);
		hhid->InPipe = 0;
	}

	if(pdevice->pData != NULL){
		tusbfree(pdevice->pData);
		pdevice->pData = NULL;
	}
	return TUSBH_E_OK;
}


/*
 *  HIDクラスプロセス実行
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 mes:      通信メッセージへのポインタ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbdHIDProcess(TUSBH_Device_t *pdevice, uint8_t *mes)
{
	TUSBH_ERCODE status = TUSBH_E_OK, status2;
	HID_Handle_t *hhid = (HID_Handle_t *)pdevice->pData;
	TUSBH_URBState_t URB_Status = mes[3];
	TUSBH_Handle_t *phost = pdevice->pHost;
	void (*func)(TUSBH_Device_t *, uint8_t, uint8_t *) = pdevice->pClass->subfunc;

	switch (hhid->state){
	case THID_INIT:
		if(phost->usrcallback != NULL)
			phost->usrcallback(phost, pdevice, HOST_USER_CLASS_ACTIVE);
	case THID_IDLE:
		hhid->ReportType = 0x01;
		hhid->ReportId   = 0;
		HID_GETREPORT(pdevice, hhid->ReportType, hhid->ReportId, hhid->buffer, hhid->length);
		hhid->state = THID_GETREPORT_WAIT;
		break;
	case THID_GETREPORT_WAIT:
		if((status2 = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			hhid->ReportLength = hhid->length;
			tusbmemcpy(hhid->Report, hhid->buffer, hhid->ReportLength);
			if(func != NULL)
				func(pdevice, 0, hhid->Report);
			syslog_2(LOG_NOTICE, "## THID_GETREPORT_WAIT hhid->Report[%08x] (%d) ##", hhid->Report, hhid->ReportLength);
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 == TUSBH_E_NOSPT){
			if(func != NULL)
				func(pdevice, 0, 0);
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 != TUSBH_E_BUSY && status2 != TUSBH_E_BUSY_URB){
			syslog_1(LOG_NOTICE, "## tusbdHIDProcess ERROR status(%d) ##", status2);
			hhid->state = THID_ERROR;
		}
		break;
	case THID_SETREPORT:
		tusbmemcpy(hhid->buffer, hhid->Report, hhid->ReportLength);
		HID_SETREPORT(pdevice, hhid->ReportType, hhid->ReportId, hhid->buffer, hhid->ReportLength);
		hhid->state = THID_SETREPORT_WAIT;
		break;
	case THID_SETREPORT_WAIT:
		if((status2 = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			syslog_2(LOG_NOTICE, "## THID_SETREPORT_WAIT(1) hhid->Report[%08x] (%d) ##", hhid->Report, hhid->ReportLength);
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 == TUSBH_E_NOSPT){
			syslog_2(LOG_NOTICE, "## THID_SETREPORT_WAIT(2) hhid->Report[%08x] (%d) ##", hhid->Report, hhid->ReportLength);
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 != TUSBH_E_BUSY && status2 != TUSBH_E_BUSY_URB){
			syslog_1(LOG_NOTICE, "## tusbdHIDProcess ERROR status(%d) ##", status2);
			hhid->state = THID_ERROR;
		}
		break;
	case THID_GET_DATA:
		if(pdevice->is_connected == 0)
			break;
		if(hhid->ReqReport == 0xFF){
			hhid->state = THID_SETREPORT;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
			break;
		}
		tusbhInterruptRead(pdevice, hhid->buffer, hhid->length, hhid->InPipe);
		hhid->state = THID_POLL;
		hhid->DataReady = 0;
		break;
	case THID_POLL:
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			if(hhid->DataReady == 0){
        		hhid->DataReady = 1;
				if(func != NULL)
					func(pdevice, hhid->length, hhid->buffer);
				pdevice->timeid    = hhid->poll;
				pdevice->timecount = hhid->poll;
				hhid->state = THID_GET_DATA;
			}
		}
		else if(URB_Status == TUSBH_URB_STALL){
			/*
			 *  STALLならCLEAR FEATURE実行
			 */
			if(tusbhClearFeature(pdevice, hhid->ep_addr) == TUSBH_E_OK){
				/* Change state to issue next IN token */
				hhid->state =THID_CLEARFEATURE_WAIT;
			}
		}
		else{
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
			hhid->state = THID_GET_DATA;
		}
		break;
	case THID_CLEARFEATURE_WAIT:
		if((status2 = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 == TUSBH_E_NOSPT){
			hhid->state = THID_GET_DATA;
			pdevice->timeid    = hhid->poll;
			pdevice->timecount = hhid->poll;
		}
		else if(status2 != TUSBH_E_BUSY && status2 != TUSBH_E_BUSY_URB){
			syslog_1(LOG_ERROR, "tusbdHIDProcess ERROR status(%d) !", status2);
			hhid->state = THID_ERROR;
		}
		break;

	default:
		break;
	}
	return status;
}

/*
 *  HIDデバイスリポート設定
 *  parameter1 hhid:    HIDハンドラ
 *  parameter2 type:    report type
 *  parameter3 id:      report id
 *  parameter4 pbuf:    report data
 *  parameter5 len:     report data length
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tubhHidSetReport(HID_Handle_t *hhid, uint8_t type, uint8_t id, uint8_t* pbuf, uint8_t len)
{
	hhid->ReportType   = type;
	hhid->ReportId     = id;
	hhid->ReportLength = len;
	tusbmemcpy(hhid->Report, pbuf, hhid->ReportLength);
	hhid->ReqReport = 0xFF;
	while(hhid->ReqReport == 0xFF){
		tusbSleep(10);
	}
	return (TUSBH_ERCODE)hhid->ReqReport;
}

/*
 *  HIDデバイスタイプ取得
 *  parameter1 hhid:    HIDハンドラ
 *  return     poll     デバイスタイプ
 */
uint8_t
tusbhHidGetType(TUSBH_Device_t *pdevice)
{
	HID_Handle_t *hhid = (HID_Handle_t *)pdevice->pData;

	if(hhid == NULL)
		return HID_UNKNOWN_TYPE;
	else
		return hhid->type;
}


/*
 *  HIDポーリングインターバル時間取得
 *  parameter1 hhid:    HIDハンドラ
 *  return     poll     インターバル時間(ms)
 */
uint8_t
tusbhHidGetPollInterval(TUSBH_Device_t *pdevice)
{
	HID_Handle_t *hhid = (HID_Handle_t *)pdevice->pData;

	if(hhid == NULL)
		return 0;
	else
		return hhid->poll;
}

/*
 *  HID コールバック関数設定
 */
void
tusbhSetHidCallBack(TUSBH_Handle_t *phost, void (*func)(TUSBH_Device_t *pdevice, uint8_t, uint8_t *))
{
	HID_Class.subfunc = func;
}

