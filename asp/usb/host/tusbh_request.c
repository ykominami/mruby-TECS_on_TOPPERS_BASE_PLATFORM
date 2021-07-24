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
 *  @(#) $Id: tusbh_request.c 698 2017-09-10 16:57:23Z roi $
 */
/*
 *  USB Host Middleware 通信リクエスト部
 */

#include "tusbh_base.h"


/*
 *  BULK通信：WRITE
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhBulkWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num)
{ 
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_WRITE, USB_EP_TYPE_BULK, buff, length);
}

/*
 *  BULK通信：READ
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhBulkRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_READ, USB_EP_TYPE_BULK, buff, length);
}

/*
 *  INTERRUPT通信：WRITE
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhInterruptWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_WRITE, USB_EP_TYPE_INTR, buff, length);
}

/*
 *  INTERRUPT通信：READ
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhInterruptRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_READ, USB_EP_TYPE_INTR, buff, length);
}

/*
 *  ISOC通信：WRITE
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhIsocWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint32_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_WRITE, USB_EP_TYPE_ISOC, buff, length);
}

/**
  * @brief  USBH_IsocReceiveData
  *         Receives the Device Response to the Isochronous IN token
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer in which the response needs to be copied
  * @param  length: Length of the data to be received
  * @param  pipe_num: Pipe Number
  * @retval USBH Status. 
  */
TUSBH_ERCODE
tusbhIsocRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint32_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_READ, USB_EP_TYPE_ISOC, buff, length);
}


/*
 *  コントロール通信：SETUP送信
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     セットアップデータ領域
 *  parameter3 pipe_num: PIPE番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhControlSendSetup(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_SETUP, USB_EP_TYPE_CTRL, buff, SETUP_PKT_LENGTH);
}

/*
 *  コントロール通信：データ送信
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: パイプ番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhControlSendData(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_WRITE, USB_EP_TYPE_CTRL, buff, length);
}

/*
 *  コントロール通信：データ受信
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 buff:     データ領域
 *  parameter3 length:   データ長
 *  parameter4 pipe_num: パイプ番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhControlReceiveData(TUSBH_Device_t *pdevice, uint8_t* buff, uint16_t length, uint8_t pipe_num)
{
	return tusbhSubmitURB(pdevice, pipe_num, SUBMIT_READ, USB_EP_TYPE_CTRL, buff, length);
}

/*
 *  コントロール通信：DESCRIPTOR取得
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 req_type: DESCRIPTORタイプ
 *  parameter3 value:    DESCRIPTOR要求値
 *  parameter4 buff:     DESCRIPTO格納領域
 *  parameter5 length:   DESCRIPTOR長
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhGetDescriptor(TUSBH_Device_t *pdevice, uint8_t req_type, uint16_t value, uint8_t* buff, uint16_t length)
{
	uint16_t index;

	if((value & 0xff00) == ((STRING_DESCRIPTOR << 8) & 0xFF00)){
		index = 0x0409;
	}
	else
		index = 0;
	return tusbhControlRequest(pdevice, buff, (USB_DEVICE_TO_HOST | req_type), GET_DESCRIPTOR, value, index, length );
}

/*
 *  コントロール通信：SET INTERFACEの送信
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 ep_num:  エンドポイント番号
 *  parameter3 interface: インタフェース値
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhSetInterface(TUSBH_Device_t *pdevice, uint8_t ep_num, uint8_t interface)
{
	return tusbhControlRequest(pdevice, 0, SET_INTERFACE_TYPE, SET_INTERFACE, interface, ep_num, 0);
}

/*
 *  コントロール通信：CLEAR FEATUREの送信
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 ep_num:  エンドポイント番号
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhClearFeature(TUSBH_Device_t *pdevice, uint8_t ep_num) 
{
	return tusbhControlRequest(pdevice, 0, CLEAR_FEATURE_TYPE, CLEAR_FEATURE, FEATURE_SELECTOR_ENDPOINT, ep_num, 0 );
}


/*
 *  コントロール通信の要求
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 buff:    通信バッファ
 *  parameter3 type:    bmRequestType
 *  parameter4 request: bRequest
 *  parameter5 value:   wValue
 *  parameter6 index:   wIndex
 *  parameter7 length:  wLength
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhControlRequest(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t type, uint8_t request, uint16_t value, uint16_t index, uint16_t length)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	TUSBH_ERCODE ercd = TUSBH_E_BUSY;
	uint8_t mes[4];

	pdevice->cstate = CONTROL_SETUP;
	pdevice->setupPacket[0] = type;
	pdevice->setupPacket[1] = request;
	pdevice->setupPacket[2] = (uint8_t)value;
	pdevice->setupPacket[3] = (uint8_t)(value >> 8);
	pdevice->setupPacket[4] = (uint8_t)index;
	pdevice->setupPacket[5] = (uint8_t)(index >> 8);
	pdevice->setupPacket[6] = (uint8_t)length; 
	pdevice->setupPacket[7] = (uint8_t)(length >> 8); 
	pdevice->cbuff          = buff;
	mes[0]                  = TUSBH_PORT_EVENT;

	do{
		ercd = tusbhControlWait(pdevice, mes);
		if(pdevice->dstate == DEVICE_CLASS)
			return ercd;
		while(ercd == TUSBH_E_BUSY_URB){
			if(tusbRecvData(phost->process_event, mes, 4000) != E_OK){
				syslog_1(LOG_ERROR, "tusbhControlRequest urb timeout state(%d) !", pdevice->cstate);
				ercd = TUSBH_E_TMOUT;
				break;
			}
			else if(mes[0] == TUSBH_URB_EVENT && mes[1] == pdevice->idx){
				ercd = TUSBH_E_BUSY;
			}
			else if(mes[1] != pdevice->idx){
				TUSBH_Device_t *pdevice2 = phost->pDevice;
				uint8_t i;
				for(i = 0 ; i < mes[1] ; i++)
					pdevice2 = pdevice2->pNext;
				if(pdevice2 != NULL && pdevice2->pClass != NULL && pdevice2->dstate == DEVICE_CLASS)
					pdevice2->pClass->Process(pdevice2, mes);
				else{
#if 1	/* ROI DEBUG */
					syslog_4(LOG_ERROR, "### tusbhControlRequest Can't EXEC [%02x][%02x][%02x][%02x] ###", mes[0], mes[1], mes[2], mes[3]);
#endif	/* ROI DEBUG */
					tusbDelay(10);
					tusbSendData(phost->process_event, mes[0], mes[1], mes[2], mes[3]);
				}
			}
		}
	}while(ercd == TUSBH_E_BUSY);
	return ercd;
}

/*
 *  コントロール通信の待ち
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 mes:     メッセージへのポインタ
 *  return     TUSBH_ERCOCE
 */
TUSBH_ERCODE
tusbhControlWait(TUSBH_Device_t *pdevice, uint8_t *mes)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	TUSBH_ERCODE ercd = TUSBH_E_BUSY;
	TUSBH_URBState_t URB_Status = mes[3];
	uint8_t  type = pdevice->setupPacket[0];
	uint16_t length = (pdevice->setupPacket[7] << 8) | (pdevice->setupPacket[6] & 0xFF);

	switch(pdevice->cstate){
	case CONTROL_SETUP:	/* PACKETセットアップ */
		tusbhControlSendSetup(pdevice, (uint8_t *)&pdevice->setupPacket, pdevice->cntl_pipe_out); 
		pdevice->cstate = CONTROL_SETUP_WAIT;
		ercd = TUSBH_E_BUSY_URB;
		break;
	case CONTROL_SETUP_WAIT:	/* PACKETセットアップ URB待ち */
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){	/* 正常終了 */
			uint8_t direction = (type & USB_REQUEST_DIR_MASK);
			if(length != 0 ){
				if(direction == USB_DEVICE_TO_HOST){	/* データREAD */
					tusbhControlReceiveData(pdevice, pdevice->cbuff, length, pdevice->cntl_pipe_in);
					pdevice->cstate = CONTROL_DATA_IN_WAIT;
				}
				else{						/* データWRITE */
					tusbhControlSendData(pdevice, pdevice->cbuff, length, pdevice->cntl_pipe_out);
					pdevice->cstate = CONTROL_DATA_OUT_WAIT;
				}
			}
			else{			/* ステータス通信 */
				if(direction == USB_DEVICE_TO_HOST){	/* ステータス受信 */
					tusbhControlSendData(pdevice, 0, 0, pdevice->cntl_pipe_out);
					pdevice->cstate = CONTROL_STATUS_OUT_WAIT;
				}
				else{						/* ステータス送信 */
					tusbhControlReceiveData(pdevice, 0, 0, pdevice->cntl_pipe_in);
					pdevice->cstate = CONTROL_STATUS_IN_WAIT;
				}
			}
			ercd = TUSBH_E_BUSY_URB;
		}
		else if(URB_Status == TUSBH_URB_ERROR)	/* USBエラー */
			pdevice->cstate = CONTROL_ERROR;
		else if(URB_Status == TUSBH_URB_NOTREADY){
			syslog_0(LOG_NOTICE, "### tusbhControlRequest REAET STATE1 ###");
			pdevice->cstate = CONTROL_SETUP;
		}
		else{
			syslog_2(LOG_ERROR, "tusbhControlRequest urb error state(%d) urb[%02x] !", pdevice->cstate, URB_Status);
			ercd = TUSBH_E_URB;
		}
		break;
	case CONTROL_DATA_IN_WAIT:		/* データ受信待ち */
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){	/* 正常終了 */
			tusbhControlSendData(pdevice, 0, 0, pdevice->cntl_pipe_out);
			pdevice->cstate = CONTROL_STATUS_OUT_WAIT;
			ercd = TUSBH_E_BUSY_URB;
		}
		else if(URB_Status == TUSBH_URB_STALL){	/* ステール状態 */
			pdevice->cstate = CONTROL_IDLE;
			ercd = TUSBH_E_NOSPT;
		}
		else if(URB_Status == TUSBH_URB_ERROR)	/* URBエラー */
			pdevice->cstate = CONTROL_ERROR;
		else if(URB_Status == TUSBH_URB_NOTREADY){
			syslog_0(LOG_NOTICE, "### tusbhControlRequest REAET STATE2 ###");
			pdevice->cstate = CONTROL_SETUP;
		}
		else{
			syslog_2(LOG_ERROR, "tusbhControlRequest urb error state(%d) urb[%02x] !", pdevice->cstate, URB_Status);
			ercd = TUSBH_E_URB;
		}
		break;
	case CONTROL_DATA_OUT_WAIT:	/* データ送信終了待ち */
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){	/* 正常終了 */
			tusbhControlReceiveData(pdevice, 0, 0, pdevice->cntl_pipe_in);
			pdevice->cstate = CONTROL_STATUS_IN_WAIT;
			ercd = TUSBH_E_BUSY_URB;
		}
		else if(URB_Status == TUSBH_URB_STALL){	/* ステール状態 */
			pdevice->cstate = CONTROL_IDLE;
			ercd = TUSBH_E_NOSPT;
		}
		else if(URB_Status == TUSBH_URB_NOTREADY){
			/* Nack received from device */
			tusbhControlSendData(pdevice, pdevice->cbuff, length, pdevice->cntl_pipe_out);
			ercd = TUSBH_E_BUSY_URB;
		}
		else if(URB_Status == TUSBH_URB_ERROR)	/* USBエラー */
			pdevice->cstate = CONTROL_ERROR;
		else{
			syslog_2(LOG_ERROR, "tusbhControlRequest urb error state(%d) urb[%02x] !", pdevice->cstate, URB_Status);
			ercd = TUSBH_E_URB;
		}
		break;
	case CONTROL_STATUS_IN_WAIT:	/* ステータス送信待ち */
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){	/* 正常終了 */
			pdevice->cstate = CONTROL_IDLE;
			ercd = TUSBH_E_OK;
			tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
		}
		else if(URB_Status == TUSBH_URB_ERROR)	/* USBエラー */
			pdevice->cstate = CONTROL_ERROR;
		else if(URB_Status == TUSBH_URB_STALL){	/* ステール状態 */
			pdevice->cstate = CONTROL_IDLE;
			ercd = TUSBH_E_NOSPT;
		}
		else if(URB_Status == TUSBH_URB_NOTREADY){
			syslog_0(LOG_NOTICE, "### tusbhControlRequest REAET STATE3 ###");
			pdevice->cstate = CONTROL_SETUP;
		}
		else{
			syslog_2(LOG_ERROR, "tusbhControlRequest urb error state(%d) urb[%02x] !", pdevice->cstate, URB_Status);
			ercd = TUSBH_E_URB;
		}
		break;
	case CONTROL_STATUS_OUT_WAIT:	/* ステータス受信待ち */
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			pdevice->cstate = CONTROL_IDLE;
			ercd = TUSBH_E_OK;
			tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
		}
		else if(URB_Status == TUSBH_URB_NOTREADY){ 
			tusbhControlSendData(pdevice, 0, 0, pdevice->cntl_pipe_out);
			ercd = TUSBH_E_BUSY_URB;
		}
		else if(URB_Status == TUSBH_URB_ERROR)	/* USBエラー */
			pdevice->cstate = CONTROL_ERROR;
		else{
			syslog_2(LOG_ERROR, "tusbhControlRequest urb error state(%d) urb[%02x] !", pdevice->cstate, URB_Status);
			ercd = TUSBH_E_URB;
		}
		break;
	case CONTROL_ERROR:
		/*
		 *  コントロール通信エラーケース
		 *  エラーカウントが TUSBH_MAX_ERROR_COUNT以下ならばリトライする
		 */
		if(++pdevice->cntl_errcount <= TUSBH_MAX_ERROR_COUNT){
			if(pdevice->hub == 0)
				tusbhHDStop(phost);
			else
				tusbhResetHub(pdevice);
			/*
			 *  リトライする
			 */
			pdevice->cstate = CONTROL_SETUP;
		}
		else{
			if(phost->usrcallback != NULL)
				phost->usrcallback(phost, pdevice, HOST_USER_UNRECOVERED_ERROR);
			pdevice->cntl_errcount = 0;
			syslog_1(LOG_ERROR, "tusbhControlRequest error count(%d) !", pdevice->cntl_errcount);
			ercd = TUSBH_E_ERR;
		}
		break;
	default:
		break;
	}
	return ercd;
}

