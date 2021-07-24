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
 *  @(#) $Id: tusbh_hub.c 698 2017-09-15 18:46:48Z roi $
 */
/*
 *  USB Host Middleware HUB CLASS部
 */

#include "tusbh_hub.h"

#define CLASS_GETDESC_TYPE          (USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS)
#define PORT_POWER_FEATURE(d, p)    tusbhControlRequest((d), NULL, PORT_SETFEATURE_TYPE, SET_FEATURE, PORT_POWERFEATURE, (p), 0)
#define PORT_RESET_FEATURE(d, p)    tusbhControlRequest((d), NULL, PORT_SETFEATURE_TYPE, SET_FEATURE, PORT_RESETFEATURE, (p), 0)
#define PORT_CLEAR_FEATURE(d, f, p) tusbhControlRequest((d), NULL, PORT_CLEARFEATURE_TYPE, CLEAR_FEATURE, (f), (p), 0)
#define PORT_GET_STATUS(d, b, p)    tusbhControlRequest((d), (b), PORT_GETSTATUS_TYPE, GET_STATUS, 0, (p), 4)

#define GET_PORT(i)                 (i+1)


static TUSBH_ERCODE tusbhHUBInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhHUBDeInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhHUBProcess(TUSBH_Device_t *pdevice, uint8_t *mes);

static TUSBH_Class_t HUB_Class = {
	NULL,
	"HUB",
	HUB_CLASS,
	tusbhHUBInit,
	tusbhHUBDeInit,
	tusbhHUBProcess,
	NULL
};

/*
 *  HUBクラスセットアップ関数
 */
void tusbhLinkHUB(TUSBH_Handle_t *phost)
{
	tusbhLinkClass(phost, &HUB_Class);
}


/*
 *  HUBクラス初期設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhHUBInit(TUSBH_Device_t *pdevice)
{
	uint8_t interface = 0; 
	HUB_Handle_t *hhub;
	TUSBH_Handle_t *phost = pdevice->pHost;
	uint8_t max_ep, i;

	interface = tusbhFindInterface(pdevice, pdevice->pClass->classCode, 0, 0);

	if(interface == NO_INTERFACE){
		syslog_1(LOG_ERROR, "[HUB] Cannot Find the interface for %s class.", pdevice->pClass->Name);
		return TUSBH_E_ERR;
	}

	tusbhSelectInterface(pdevice, interface);
	hhub = (HUB_Handle_t *)tusbmalloc(sizeof(HUB_Handle_t));
	pdevice->pData = hhub;
	memset(hhub, 0, sizeof(HUB_Handle_t));
	hhub->hubid     = ++phost->numHub;
	pdevice->hubid  = hhub->hubid;
	hhub->state     = THUB_IDLE;
	hhub->cstate    = THUB_CONNCET_IDEL;
	hhub->ep_addr   = pdevice->EpDesc[pdevice->sel_interface][0].bEndpointAddress;
	hhub->length    = pdevice->EpDesc[pdevice->sel_interface][0].wMaxPacketSize;
	hhub->poll      = pdevice->EpDesc[pdevice->sel_interface][0].bInterval;

	max_ep = pdevice->ItfDesc[pdevice->sel_interface].bNumEndpoints;
	if(max_ep == 0 || (pdevice->EpDesc[pdevice->sel_interface][0].bEndpointAddress & 0x80) == 0)
		return TUSBH_E_ERR;


	syslog_4(LOG_NOTICE, "## max_ep(%d) length(%d) poll(%d) b(%d) ##", max_ep, hhub->length, hhub->poll, offsetof(HUB_Handle_t, buffer));
	hhub->InPipe  = tusbhAllocPipe(phost, hhub->ep_addr);

	/*
	 *  IN ENDPOINT OPEN
	 */
	tusbhOpenPipe(pdevice, hhub->InPipe, hhub->ep_addr, USB_EP_TYPE_INTR, hhub->length);
	tusbhHDSetToggle(phost, hhub->InPipe, 0);

	if(tusbhGetDescriptor(pdevice, CLASS_GETDESC_TYPE, (0x29 << 8), hhub->buffer, sizeof(HubDescriptor)) != TUSBH_E_OK)
		return TUSBH_E_ERR;
	hhub->numport         = hhub->buffer[2];
	hhub->characteristics = hhub->buffer[3];
	hhub->starttime       = hhub->buffer[5] * 2;
	syslog_1(LOG_NOTICE, "## hhub->numport(%d) ##", hhub->numport);

	for(i = 0 ; i < hhub->numport ; i++){
		PORT_POWER_FEATURE(pdevice, GET_PORT(i));
	}

	/* タイマースタート */
	hhub->timer = 0;
	pdevice->timeid = pdevice->timecount = hhub->poll;
	return TUSBH_E_OK;
}

/*
 *  HUBクラス終了設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhHUBDeInit(TUSBH_Device_t *pdevice)
{
	HUB_Handle_t *hhub = (HUB_Handle_t *)pdevice->pData;
	TUSBH_Handle_t *phost = pdevice->pHost;

	if(hhub->InPipe != 0){
		tusbhClosePipe(pdevice, hhub->InPipe);
		tusbFreePipe(phost, hhub->InPipe);
		hhub->InPipe = 0;
	}
	if(phost->numHub > 0)
		phost->numHub--;

	if(pdevice->pData != NULL){
		tusbfree(pdevice->pData);
		pdevice->pData = NULL;
	}
	return TUSBH_E_OK;
}

/*
 *  HUBクラスプロセス実行
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 mes:      通信メッセージへのポインタ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhHUBProcess(TUSBH_Device_t *pdevice, uint8_t *mes)
{
	HUB_Handle_t   *hhub;
	TUSBH_Handle_t *phost = pdevice->pHost;
	TUSBH_Device_t *pdevicep;
	TUSBH_ERCODE   status = TUSBH_E_OK;
	TUSBH_URBState_t URB_Status = mes[3];
	uint32_t pstatus = 0;
	uint32_t i;

	hhub = (HUB_Handle_t *)pdevice->pData;
	if(mes[0] == TUSBH_TIME_EVENT)
		hhub->timer += mes[3];
	else if(mes[0] == TUSBH_RESET_EVENT)
		hhub->reset_portid = mes[3];
	switch (hhub->state){
	case THUB_IDLE:
		if(phost->usrcallback != NULL)
			phost->usrcallback(phost, pdevice, HOST_USER_CLASS_ACTIVE);
		hhub->state = THUB_SYNC;
	case THUB_SYNC:
		if(hhub->timer < hhub->starttime){
			pdevice->timecount = hhub->poll;
			break;
		}
		hhub->portno = 0;
		hhub->cstate = THUB_CONNCET_IDEL;
		if(hhub->reset_portid > 0){
			PORT_RESET_FEATURE(pdevice, hhub->reset_portid);
			hhub->state = THUB_PORT_RESET_WAIT;
			hhub->reset_portid = 0;
		}
	case THUB_GET_DATA:
		tusbhInterruptRead(pdevice, hhub->buffer, hhub->length, hhub->InPipe);
		hhub->state = THUB_POLL;
		break;
	case THUB_POLL:
		if(mes[0] != TUSBH_URB_EVENT)
			 break;
		if(URB_Status == TUSBH_URB_DONE){
			PORT_GET_STATUS(pdevice, hhub->buffer, GET_PORT(hhub->portno));
			hhub->state = THUB_GET_STATUS_WAIT;
		}
		else if(URB_Status == TUSBH_URB_STALL){ /* IN Endpoint Stalled */
			hhub->state = THUB_PORT_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		else{
			hhub->state = THUB_GET_DATA;
			pdevice->timecount = hhub->poll;
		}
		break;
	case THUB_GET_STATUS_WAIT:
		if((status = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			uint8_t speed;
			pstatus  = hhub->buffer[0];
			pstatus |= hhub->buffer[1] << 8;
			pstatus |= hhub->buffer[2] << 16;
			pstatus |= hhub->buffer[3] << 24;
#if 1	/* ROI DEBUG */
			if((pstatus & 0xffff0000) != 0)
				syslog_2(LOG_NOTICE, "## PSTATUS port(%d) status[%08x] ##", hhub->portno, pstatus);
#endif	/* ROI DEBUG */

			if((pstatus & PORT_CHANGE_CONNECTION) != 0){
				if((pstatus & PORT_STATUS_CONNECTION) != 0)
					hhub->cstate = THUB_CONNECT_RESET;
				else
					tusbSendData(phost->connect_event, TUSBH_DISCONNECT_EVENT, hhub->hubid, GET_PORT(hhub->portno), 0);
				PORT_CLEAR_FEATURE(pdevice, PORT_CLEAR_CONNECTION_FEATURE, GET_PORT(hhub->portno));
				hhub->state = THUB_CLEAR_FEATURE_WAIT;
			}
			else if((pstatus & PORT_CHANGE_RESET) != 0){
				PORT_CLEAR_FEATURE(pdevice, PORT_CLEAR_RESET_FEATURE, GET_PORT(hhub->portno));
				hhub->state = THUB_CLEAR_FEATURE_WAIT;
				if(hhub->cstate == THUB_CONNECT_RESET){
					if((pstatus & (PORT_STATUS_CONNECTION | PORT_STATUS_ENABLE)) == (PORT_STATUS_CONNECTION | PORT_STATUS_ENABLE)){
						hhub->cstate = THUB_CONNECT_ENABLE;
						if((pstatus & PORT_STATUS_LOW_SPEED) != 0)
							speed = USB_DEVICE_SPEED_LOW;
						else
							speed = USB_DEVICE_SPEED_FULL;
						tusbSendData(phost->connect_event, TUSBH_CONNECT_EVENT, hhub->hubid, GET_PORT(hhub->portno), speed);
					}
				}
			}
			else if((pstatus & PORT_CHANGE_ENABLE) != 0){
				PORT_CLEAR_FEATURE(pdevice, PORT_CLEAR_ENABLE_FEATURE, GET_PORT(hhub->portno));
				hhub->state = THUB_CLEAR_FEATURE_WAIT;
				if(hhub->cstate == THUB_CONNECT_RESET){
					if((pstatus & (PORT_STATUS_CONNECTION | PORT_STATUS_ENABLE)) == (PORT_STATUS_CONNECTION | PORT_STATUS_ENABLE)){
						hhub->cstate = THUB_CONNECT_ENABLE;
						if((pstatus & PORT_STATUS_LOW_SPEED) != 0)
							speed = USB_DEVICE_SPEED_LOW;
						else
							speed = USB_DEVICE_SPEED_FULL;
						tusbSendData(phost->connect_event, TUSBH_CONNECT_EVENT, hhub->hubid, GET_PORT(hhub->portno), speed);
					}
				}
			}
			else if((pstatus & PORT_CHANGE_OVER_CURRENT) != 0){
				tusbSendData(phost->connect_event, TUSBH_DISCONNECT_EVENT, hhub->hubid, GET_PORT(hhub->portno), 0);
				PORT_CLEAR_FEATURE(pdevice, PORT_OVER_CURRENT_FEATURE, GET_PORT(hhub->portno));
				hhub->state = THUB_CLEAR_FEATURE_WAIT;
			}
//			else if(hhub->connect != 0){
			else if(hhub->cstate != THUB_CONNCET_IDEL){
				hhub->state = THUB_GET_DATA;
				pdevice->timecount = hhub->poll;
			}
			else if(++hhub->portno < hhub->numport){
				PORT_GET_STATUS(pdevice, hhub->buffer, GET_PORT(hhub->portno));
				hhub->state = THUB_GET_STATUS_WAIT;
			}
			else{
				hhub->state = THUB_SYNC;
				pdevice->timecount = hhub->poll;
			}
		}
		else if(status != TUSBH_E_BUSY && status != TUSBH_E_BUSY_URB){
			hhub->state = THUB_PORT_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;
	case THUB_CLEAR_FEATURE_WAIT:
		if((status = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			if(hhub->cstate == THUB_CONNECT_RESET){
				PORT_RESET_FEATURE(pdevice, GET_PORT(hhub->portno));
				hhub->state = THUB_PORT_RESET_WAIT;
			}
			else if(hhub->cstate == THUB_CONNECT_ENABLE){
				hhub->state = THUB_CONNECT_WAIT;
				pdevice->timecount = 50;
			}
			else if(++hhub->portno < hhub->numport){
				PORT_GET_STATUS(pdevice, hhub->buffer, GET_PORT(hhub->portno));
				hhub->state = THUB_GET_STATUS_WAIT;
			}
			else{
				hhub->state = THUB_SYNC;
				pdevice->timecount = hhub->poll;
			}
		}
		else if(status != TUSBH_E_BUSY && status != TUSBH_E_BUSY_URB){
			hhub->state = THUB_PORT_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;
	case THUB_PORT_RESET_WAIT:
		if((status = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK){
			hhub->state = THUB_GET_DATA;
			pdevice->timecount = hhub->poll;
		}
		else if(status != TUSBH_E_BUSY && status != TUSBH_E_BUSY_URB){
			hhub->state = THUB_PORT_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;
	case THUB_CONNECT_WAIT:
		pdevice->timecount = 200;
		for(i = 0, pdevicep = phost->pDevice ; i < phost->numDevice && pdevicep != NULL ; i++){
			if(pdevicep->hub == hhub->hubid && pdevicep->port == GET_PORT(hhub->portno)){
				if(pdevicep->dstate >= DEVICE_CLASS){
					if(pdevicep->pClass != NULL)
						syslog_2(LOG_NOTICE, "[HUB] connected idx(%d) class[%s] !", pdevicep->idx, pdevicep->pClass->Name);
					hhub->cstate  = THUB_CONNCET_IDEL;
					if(++hhub->portno < hhub->numport){
						PORT_GET_STATUS(pdevice, hhub->buffer, GET_PORT(hhub->portno));
						hhub->state = THUB_GET_STATUS_WAIT;
					}
					else{
						hhub->state = THUB_SYNC;
						pdevice->timecount = hhub->poll;
					}
				}
				break;
			}
			pdevicep = pdevicep->pNext;
		}
		break;
	case THUB_PORT_ERROR:
		tusbhClearFeature(pdevice, hhub->ep_addr);
		hhub->state = THUB_PORT_ERROR_WAIT;
		break;
	case THUB_PORT_ERROR_WAIT:
		if(tusbhControlWait(pdevice, mes) == TUSBH_E_OK){
			hhub->state = THUB_SYNC;
			pdevice->timecount = hhub->poll;
		}
		else if(status == TUSBH_E_BUSY_URB)
			status = TUSBH_E_BUSY;
		break;

	default:
		break;
	}
	return status;
}

