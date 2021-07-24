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
 *  @(#) $Id: tusbh_msc.c 698 2017-09-12 18:46:56Z roi $
 */
/*
 *  USB Host Middleware MSC CLASS部
 */

#include "tusbh_msc.h"


typedef enum {
	BOT_OK,
	BOT_BUSY,
	BOT_SENSE,
	BOT_ERROR
} BOT_TYPE;

#define CBW_SIGNATURE               0x43425355
#define CSW_SIGNATURE               0x53425355
#define CBW_TAG                     0x20304050

#define BOT_CBW_LENGTH              31			/* CBW送信パケット長 */
#define BOT_CSW_LENGTH              13			/* CSW受信パケット長 */

#define UNIT_READY_TIMEOUT          (10000)

#define BOT_REQUEST_GETMAXLUN(d, b) tusbhControlRequest((d), (b), BOT_REQUEST_GETMAXLUN_TYPE, USB_REQ_GET_MAX_LUN, 0, 0, 1)
#define BOT_REQUEST_RESET(d)        tusbhControlRequest((d), 0, BOT_REQUEST_RESET_TYPE, USB_REQ_BOT_RESET, 0, 0, 0)


/*
 *  SENSEデータ
 */
typedef struct
{
	uint8_t key;
	uint8_t asc;
	uint8_t ascq;
} SCSI_Sense_t;


static TUSBH_ERCODE tusbhMSCInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhMSCDeInit(TUSBH_Device_t *pdevice);
static TUSBH_ERCODE tusbhMSCProcess(TUSBH_Device_t *pdevice, uint8_t *mes);

static TUSBH_Class_t MSC_Class = {
	NULL,
	"MSC",
	MSC_CLASS,
	tusbhMSCInit,
	tusbhMSCDeInit,
	tusbhMSCProcess,
	NULL
};

/*
 *  MSCクラスセットアップ関数
 */
void tusbhLinkMSC(TUSBH_Handle_t *phost)
{
	tusbhLinkClass(phost, &MSC_Class);
}

/*
 * MSC SCSI Inquiry要求
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 lun:     LUN#
 *  return     TUSBH_E_BUSY
 */
static TUSBH_ERCODE
tusbhMscSCSIInquiry(MSC_Handle_t *hmsc, uint8_t lun)
{
	hmsc->hbot.cbw.DataTransferLength = DATA_LEN_INQUIRY;
	hmsc->hbot.cbw.Flags = USB_DEVICE_TO_HOST;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;

	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_INQUIRY;
	hmsc->hbot.cbw.CB[1]  = (lun << 5);
	hmsc->hbot.cbw.CB[2]  = 0;
	hmsc->hbot.cbw.CB[3]  = 0;
	hmsc->hbot.cbw.CB[4]  = 0x24;
	hmsc->hbot.cbw.CB[5]  = 0;

	hmsc->hbot.state = BOT_SEND_CBW;
	hmsc->hbot.pbuf = (uint8_t *)hmsc->hbot.data;
	return TUSBH_E_BUSY;
}

/*
 * MSC SCSI Test Unit Ready要求
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 lun:     LUN#
 *  return     TUSBH_E_BUSY
 */
static TUSBH_ERCODE
tusbhMscSCSITestUnitReady(MSC_Handle_t *hmsc, uint8_t lun)
{
	hmsc->hbot.cbw.DataTransferLength = DATA_LEN_MODE_TEST_UNIT_READY;
	hmsc->hbot.cbw.Flags = USB_HOST_TO_DEVICE;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;

	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_CB_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_TEST_UNIT_READY; 

	hmsc->hbot.state = BOT_SEND_CBW;
	return TUSBH_E_BUSY;
}

/*
 * MSC SCSI Read Capacity要求
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 lun:     LUN#
 *  return     TUSBH_E_BUSY
 */
static TUSBH_ERCODE
tusbhMscSCSIReadCapacity(MSC_Handle_t *hmsc, uint8_t lun)
{
	hmsc->hbot.cbw.DataTransferLength = DATA_LEN_READ_CAPACITY10;
	hmsc->hbot.cbw.Flags = USB_DEVICE_TO_HOST;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;

	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_CB_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_READ_CAPACITY10;

	hmsc->hbot.state = BOT_SEND_CBW;
    hmsc->hbot.pbuf = (uint8_t *)hmsc->hbot.data;
	return TUSBH_E_BUSY;
}

/*
 * MSC SCSI Request Sense要求
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 lun:     LUN#
 *  return     TUSBH_E_BUSY
 */
static TUSBH_ERCODE
tusbhMscSCSIRequestSense(MSC_Handle_t *hmsc, uint8_t lun)
{
	hmsc->hbot.cbw.DataTransferLength = DATA_LEN_REQUEST_SENSE;
	hmsc->hbot.cbw.Flags = USB_DEVICE_TO_HOST;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;

	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_CB_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_REQUEST_SENSE;
	hmsc->hbot.cbw.CB[1]  = (lun << 5);
	hmsc->hbot.cbw.CB[2]  = 0;
	hmsc->hbot.cbw.CB[3]  = 0;
	hmsc->hbot.cbw.CB[4]  = DATA_LEN_REQUEST_SENSE;
	hmsc->hbot.cbw.CB[5]  = 0;

	hmsc->hbot.state = BOT_SEND_CBW;
	hmsc->hbot.pbuf = (uint8_t *)hmsc->hbot.data;
	return TUSBH_E_BUSY;
}

/*
 *  SCCI データ転送
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 mes:     メッセージへのポインタ
 *  parameter3 lun:     LUN#
 *  return     BOT_TYPE
 */
static BOT_TYPE
tusbhMscSCSITransfer(MSC_Handle_t *hmsc, uint8_t *mes, uint8_t lun)
{
	TUSBH_Device_t *pdevice = hmsc->pDev;
	TUSBH_Handle_t *phost;
	BOT_TYPE       status = BOT_BUSY;
	TUSBH_ERCODE   ercd  = TUSBH_E_BUSY;
	TUSBH_URBState_t URB_Status = mes[3];
	uint8_t toggle = 0;

	phost = pdevice->pHost;
	switch (hmsc->hbot.state){
	case BOT_SEND_CBW:
		hmsc->hbot.cbw.LUN = lun;
		hmsc->hbot.state = BOT_SEND_CBW_WAIT;
		tusbhBulkWrite(pdevice, (uint8_t *)&hmsc->hbot.cbw, BOT_CBW_LENGTH, hmsc->OutPipe);
		break;

	case BOT_SEND_CBW_WAIT:
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			if(hmsc->hbot.cbw.DataTransferLength != 0){
				if(((hmsc->hbot.cbw.Flags) & USB_REQUEST_DIR_MASK) == USB_DEVICE_TO_HOST){
					/*
					 *  BOT DATA INケース
					 */
					tusbhBulkRead (pdevice, hmsc->hbot.pbuf, hmsc->InEpSize, hmsc->InPipe);
					hmsc->hbot.state  = BOT_DATA_IN_WAIT;
				}
				else{
					/*
					 *  BOT DATA OUTケース
					 */
					tusbhBulkWrite(pdevice, hmsc->hbot.pbuf, hmsc->OutEpSize, hmsc->OutPipe);
					hmsc->hbot.state  = BOT_DATA_OUT_WAIT;
				}
			}
			else{
				/*
				 *  ステータス通信ケース
				 */
				tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
				hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
			}
		}
		else if(URB_Status == TUSBH_URB_NOTREADY){
			/*
			 * CBWを再送
			 */
			hmsc->hbot.state = BOT_SEND_CBW;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		else if(URB_Status == TUSBH_URB_STALL){
			/*
			 *  Clear Fuature Out Endpoint(CBWSTALL)
			 */
			ercd = tusbhClearFeature(pdevice, hmsc->OutEp);
			if(ercd == TUSBH_E_OK){ 
				toggle = tusbhHDGetToggle(phost, hmsc->OutPipe);
				tusbhHDSetToggle(phost, hmsc->OutPipe, 1- toggle);
				tusbhHDSetToggle(phost, hmsc->InPipe, 0);
				/*
				 *  Clear Fuature In Endpoint
				 */
				ercd = tusbhClearFeature(pdevice, hmsc->InEp);
				if(ercd == TUSBH_E_OK){
					tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
					hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
				}
				else
					hmsc->hbot.state = BOT_STATE_ERROR;
			}
			else
				hmsc->hbot.state = BOT_STATE_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;

	case BOT_DATA_IN_WAIT:
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			/*
			 *  受信データ長アップデート
			 */
			if(hmsc->hbot.cbw.DataTransferLength > hmsc->InEpSize){
				hmsc->hbot.pbuf += hmsc->InEpSize;
				hmsc->hbot.cbw.DataTransferLength -= hmsc->InEpSize;
			}
			else
				hmsc->hbot.cbw.DataTransferLength = 0;
			if(hmsc->hbot.cbw.DataTransferLength > 0){
				/*
				 *  次のパケット要求
				 */
				tusbhBulkRead(pdevice, hmsc->hbot.pbuf, hmsc->InEpSize, hmsc->InPipe);
			}
			else{
				/*
				 *  受信終了、CSW受信要求
				 */
				tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
				hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
			}
		}
		else if(URB_Status == TUSBH_URB_STALL){
			/*
			 *  Clear Fuature In Endpoint(受信STALL)
			 */
			ercd = tusbhClearFeature(pdevice, hmsc->InEp);
			if(ercd == TUSBH_E_OK){
				tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
				hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
			}
			else
				hmsc->hbot.state = BOT_STATE_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;

	case BOT_DATA_OUT_WAIT:
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			/*
			 *  送信データ長アップデート
			 */
			if(hmsc->hbot.cbw.DataTransferLength > hmsc->OutEpSize){
				hmsc->hbot.pbuf += hmsc->OutEpSize;
				hmsc->hbot.cbw.DataTransferLength -= hmsc->OutEpSize; 
			}
			else
				hmsc->hbot.cbw.DataTransferLength = 0;
			if(hmsc->hbot.cbw.DataTransferLength > 0){
				/*
				 *  次のパケット送信
				 */
				tusbhBulkWrite(pdevice, hmsc->hbot.pbuf, hmsc->OutEpSize, hmsc->OutPipe);
			}
			else{
				/*
				 *  送信終了、CSW受信要求
				 */
				tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
				hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
			}
		}
		else if(URB_Status == TUSBH_URB_NOTREADY){
			/*
			 *  再送設定
			 */
			tusbhBulkWrite(pdevice, hmsc->hbot.pbuf, hmsc->OutEpSize, hmsc->OutPipe);
			hmsc->hbot.state  = BOT_DATA_OUT_WAIT;
		}
		else if(URB_Status == TUSBH_URB_STALL){
			/*
			 *  Clear Fuature Out Endpoint(送信STALL)
			 */
			ercd = tusbhClearFeature(pdevice, hmsc->OutEp);
			if(ercd == TUSBH_E_OK){ 
				toggle = tusbhHDGetToggle(phost, hmsc->OutPipe);
				tusbhHDSetToggle(phost, hmsc->OutPipe, 1- toggle);
				tusbhHDSetToggle(phost, hmsc->InPipe, 0);
				/*
				 *  Clear Fuature In Endpoint
				 */
				ercd = tusbhClearFeature(pdevice, hmsc->InEp);
				if(ercd == TUSBH_E_OK){
					tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
					hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
				}
				else
					hmsc->hbot.state = BOT_STATE_ERROR;
			}
			else
				hmsc->hbot.state = BOT_STATE_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;

	case BOT_RECEIVE_CSW_WAIT:
		if(mes[0] != TUSBH_URB_EVENT)
			break;
		if(URB_Status == TUSBH_URB_DONE){
			hmsc->hbot.state = BOT_SEND_CBW;
			if(hmsc->hbot.csw.Signature != CSW_SIGNATURE)
				status = BOT_ERROR;
			else if(hmsc->hbot.csw.Status == 0)
				status = BOT_OK;
			else if(hmsc->hbot.csw.Status == 1){
#if 1	/* ROI DEBUG */
				syslog_1(LOG_NOTICE, "## SBH_MSC_BOT_Process BOT_RECEIVE_CSW_WAIT ERROR CSW_Status(%d) ##", hmsc->hbot.csw.Status);
#endif	/* ROI DEBUG */
				status = BOT_SENSE;
			}
			else
				status = BOT_ERROR;
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		else if(URB_Status == TUSBH_URB_STALL){
			/*
			 *  Clear Fuature In Endpoint(CSW-STALL)
			 */
			ercd = tusbhClearFeature(pdevice, hmsc->InEp);
			if(ercd == TUSBH_E_OK){
				tusbhBulkRead(pdevice, (uint8_t *)&hmsc->hbot.csw, BOT_CSW_LENGTH, hmsc->InPipe);
				hmsc->hbot.state  = BOT_RECEIVE_CSW_WAIT;
			}
			else
				hmsc->hbot.state = BOT_STATE_ERROR;
			tusbSendData (phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		break;

	case BOT_STATE_ERROR:
		BOT_REQUEST_RESET(pdevice);
		hmsc->hbot.state = BOT_STATE_ERROR_WAIT;
		break;

	case BOT_STATE_ERROR_WAIT:
		if((ercd = tusbhControlWait(pdevice, mes)) == TUSBH_E_OK)
			hmsc->hbot.state = BOT_SEND_CBW;
		else if(ercd != TUSBH_E_BUSY && ercd != TUSBH_E_BUSY_URB)
			status = BOT_ERROR;
		break;

	default:
		break;
	}
	return status;
}

/*
 * MSC SCSI Disk Initialize
 *  parameter1 hmcs:    MSCハンドラ
 *  parameter2 lun:     LUN#
 *  return     TUSBH_E_BUSY
 */
static TUSBH_ERCODE
tusbhMscSCSIDiskInitailize(MSC_Handle_t *hmsc, uint8_t *mes, uint8_t lun)
{
	MSC_Unit_t   *punit = &hmsc->unit[lun];
	SCSI_Sense_t sense;
	TUSBH_ERCODE status = TUSBH_E_OK;
	BOT_TYPE bot_status = BOT_BUSY;

	switch (punit->state){
	case MSC_UNIT_INIT:
		syslog_1(LOG_NOTICE, "[MSC]LUN : #%d", lun);
		punit->state = MSC_UNIT_READ_INQUIRY_WAIT;
		status = tusbhMscSCSIInquiry(hmsc, lun);
	case MSC_UNIT_READ_INQUIRY_WAIT:
		bot_status = tusbhMscSCSITransfer(hmsc, mes, lun);
		if(bot_status == BOT_OK){
			/*
			 *  INQUIRY情報取得
			 */
			punit->DeviceType = hmsc->hbot.pbuf[0] & 0x1F;
			punit->PeripheralQualifier = hmsc->hbot.pbuf[0] >> 5;  
			punit->RemovableMedia = (hmsc->hbot.pbuf[1] & 0x80)== 0x80;
			tusbmemcpy(punit->vendor_id, &hmsc->hbot.pbuf[8], 8);
			tusbmemcpy(punit->product_id, &hmsc->hbot.pbuf[16], 16);
			tusbmemcpy(punit->revision_id, &hmsc->hbot.pbuf[32], 4);

			syslog_1(LOG_NOTICE, "[MSC]Inquiry Vendor  : %s", punit->vendor_id);
			syslog_1(LOG_NOTICE, "[MSC]Inquiry Product : %s", punit->product_id);
			syslog_1(LOG_NOTICE, "[MSC]Inquiry Version : %s", punit->revision_id);
			punit->state = MSC_UNIT_TEST_UNIT_READY_WAIT;
			status = tusbhMscSCSITestUnitReady(hmsc, lun);
		}
		else if(bot_status == BOT_SENSE){
			punit->state = MSC_UNIT_REQUEST_SENSE_WAIT;
			punit->pre_state = MSC_UNIT_READ_INQUIRY_WAIT;
			status = tusbhMscSCSIRequestSense(hmsc, lun);
		}
		else if(bot_status == BOT_ERROR){
			punit->state = MSC_UNIT_ERROR;
		}
		else{
			dly_tsk(10);
			if(++hmsc->urb_wait > 500)
				punit->state = MSC_UNIT_ERROR;
		}
		break;
	case MSC_UNIT_TEST_UNIT_READY_WAIT:
		bot_status = tusbhMscSCSITransfer(hmsc, mes, lun);
		if(bot_status == BOT_OK){
			syslog_1(LOG_NOTICE, "[MSC] Device ready(%d) !", lun);
			punit->state = MSC_UNIT_READ_CAPACITY_WAIT;
			status = tusbhMscSCSIReadCapacity(hmsc, lun);
		}
		else if(bot_status == BOT_SENSE){
			if(hmsc->timer >= UNIT_READY_TIMEOUT){
				syslog_1(LOG_NOTICE, "[MSC] Device NOT ready(%d) !", lun);
			}
			punit->state = MSC_UNIT_REQUEST_SENSE_WAIT; 
			punit->pre_state = MSC_UNIT_TEST_UNIT_READY_WAIT;
			status = tusbhMscSCSIRequestSense(hmsc, lun);
		}
		else if(bot_status == BOT_ERROR){
			punit->state = MSC_UNIT_ERROR;
		}
		else{
			dly_tsk(10);
			if(++hmsc->urb_wait > 500)
				punit->state = MSC_UNIT_ERROR;
		}
		break;
	case MSC_UNIT_READ_CAPACITY_WAIT:
		bot_status = tusbhMscSCSITransfer(hmsc, mes, lun);
		if(bot_status == BOT_OK){
			/*
			 *  ユニット容量を取得
			 */
			punit->num_block = hmsc->hbot.pbuf[3] | (hmsc->hbot.pbuf[2] << 8) |
								 (hmsc->hbot.pbuf[1] << 16) | (hmsc->hbot.pbuf[0] << 24);
			punit->block_size = hmsc->hbot.pbuf[7] | (hmsc->hbot.pbuf[6] << 8); 
			syslog_1(LOG_NOTICE, "[MSC] Device capacity : %u Bytes", \
				(uint32_t)(punit->num_block * punit->block_size));
			syslog_1(LOG_NOTICE, "      Block number : %u", (uint32_t)(punit->num_block));
			syslog_1(LOG_NOTICE, "      Block Size   : %u", (uint32_t)(punit->block_size));
			punit->state = MSC_UNIT_IDLE;
			hmsc->lun++;
		}
		else if(bot_status == BOT_SENSE){
			punit->state = MSC_UNIT_REQUEST_SENSE_WAIT;
			punit->pre_state = MSC_UNIT_READ_CAPACITY_WAIT;
			status = tusbhMscSCSIRequestSense(hmsc, lun);
		}
		else if(bot_status == BOT_ERROR){
			punit->state = MSC_UNIT_ERROR;
		}
		else{
			dly_tsk(10);
			if(++hmsc->urb_wait > 500)
				punit->state = MSC_UNIT_ERROR;
		}
		break;
	case MSC_UNIT_REQUEST_SENSE_WAIT:
		bot_status = tusbhMscSCSITransfer(hmsc, mes, lun);
		if(bot_status == BOT_OK){
			sense.key  = hmsc->hbot.pbuf[2] & 0x0F;
			sense.asc  = hmsc->hbot.pbuf[12];
			sense.ascq = hmsc->hbot.pbuf[13];
			if(sense.key == SCSI_SENSE_KEY_UNIT_ATTENTION || sense.key == SCSI_SENSE_KEY_NOT_READY){
				if(hmsc->timer < UNIT_READY_TIMEOUT){
					if(punit->pre_state == MSC_UNIT_READ_INQUIRY_WAIT){
						punit->state = MSC_UNIT_READ_INQUIRY_WAIT;
						status = tusbhMscSCSIInquiry(hmsc, lun);
						break;
					}
					else if(punit->pre_state == MSC_UNIT_TEST_UNIT_READY_WAIT){
						punit->state = MSC_UNIT_TEST_UNIT_READY_WAIT;
						status = tusbhMscSCSITestUnitReady(hmsc, lun);
						break;
					}
					else if(punit->pre_state == MSC_UNIT_READ_CAPACITY_WAIT){
						punit->state = MSC_UNIT_READ_CAPACITY_WAIT;
						status = tusbhMscSCSIReadCapacity(hmsc, lun);
						break;
					}
				}
			}
			syslog_1(LOG_NOTICE, "[MSC] Sense Key  : %x", sense.key);
			syslog_1(LOG_NOTICE, "[MSC] Additional Sense Code : %x", sense.asc);
			syslog_1(LOG_NOTICE, "[MSC] Additional Sense Code Qualifier: %x", sense.ascq);
			punit->state = MSC_UNIT_IDLE;
			lun++;
		}
		else if(bot_status == BOT_SENSE || bot_status == BOT_ERROR){
			punit->state = MSC_UNIT_ERROR;
		}
		else{
			dly_tsk(10);
			if(++hmsc->urb_wait > 500)
				punit->state = MSC_UNIT_ERROR;
		}
		break;
	case MSC_UNIT_ERROR:
		hmsc->lun++;
		break;
	default:
		break;
	}
	return status;
}


/*
 *  MSCクラス初期設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhMSCInit(TUSBH_Device_t *pdevice)
{
	uint8_t interface = 0; 
	TUSBH_ERCODE status = TUSBH_E_ERR ;
	MSC_Handle_t *hmsc;
	TUSBH_Handle_t *phost = pdevice->pHost;
	uint8_t max_ep, i;

	interface = tusbhFindInterface(pdevice, pdevice->pClass->classCode, MSC_TRANSPARENT_SUBCLASS, MSC_BOT_PROTOCOL);

	if(interface == NO_INTERFACE){
		syslog_1(LOG_ERROR, "[MSC] Cannot Find the interface for %s class.", pdevice->pClass->Name);
		return TUSBH_E_ERR;
	}
	tusbhSelectInterface(pdevice, interface);

	hmsc = (MSC_Handle_t *)tusbmalloc(sizeof(MSC_Handle_t));
	pdevice->pData = hmsc;
	memset(hmsc, 0, sizeof(MSC_Handle_t));
	max_ep = ((pdevice->ItfDesc[pdevice->sel_interface].bNumEndpoints <= 2) ?
              pdevice->ItfDesc[pdevice->sel_interface].bNumEndpoints : 2);

	for(i = 0 ; i < max_ep; i++){
		if(pdevice->EpDesc[pdevice->sel_interface][i].bEndpointAddress & 0x80){
			hmsc->InEp     = (pdevice->EpDesc[pdevice->sel_interface][i].bEndpointAddress);
			hmsc->InEpSize = pdevice->EpDesc[pdevice->sel_interface][i].wMaxPacketSize;
			hmsc->InPipe = tusbhAllocPipe(phost, hmsc->InEp);
		}
		else{
			hmsc->OutEp = (pdevice->EpDesc[pdevice->sel_interface][i].bEndpointAddress);
			hmsc->OutEpSize = pdevice->EpDesc[pdevice->sel_interface][i].wMaxPacketSize;
			hmsc->OutPipe = tusbhAllocPipe(phost, hmsc->OutEp);
		}
	}
	if(hmsc->OutPipe == 0 || hmsc->InPipe == 0)
		return TUSBH_E_ERR;

	hmsc->lun = 0;
	hmsc->state = MSC_PROCESS_INIT;
	hmsc->urb_wait = 0;
	hmsc->timer = 0;
	hmsc->pDev = pdevice;

	hmsc->hbot.cbw.Signature = CBW_SIGNATURE;
	hmsc->hbot.cbw.Tag = CBW_TAG;
	hmsc->hbot.state = BOT_SEND_CBW;

	/*
	 *  NSCチャネルオープン
	 */
	tusbhOpenPipe(pdevice, hmsc->OutPipe, hmsc->OutEp, USB_EP_TYPE_BULK, hmsc->OutEpSize);
	tusbhHDSetToggle(phost, hmsc->OutPipe, 0);
	tusbhOpenPipe(pdevice, hmsc->InPipe, hmsc->InEp, USB_EP_TYPE_BULK, hmsc->InEpSize);
	tusbhHDSetToggle(phost, hmsc->InPipe, 0);

	/*
	 *  最大LUNを取得
	 */
	status = BOT_REQUEST_GETMAXLUN(pdevice, ((uint8_t *)&hmsc->hbot.data));
	hmsc->max_lun = hmsc->hbot.data[0];

	/*
	 *  NO SUPPORT受信の場合、0を設定
	 */
	if(status == TUSBH_E_NOSPT){
		hmsc->max_lun = 0;
		status = TUSBH_E_OK;
	}

	/*
	 *  UNIT領域の初期化
	 */
	if(status == TUSBH_E_OK){
		uint32_t i;
		hmsc->max_lun = (uint8_t )(hmsc->max_lun) + 1;
		syslog_1(LOG_NOTICE, "[MSC] Number of supported LUN: %d", (int32_t)(hmsc->max_lun));

		for(i = 0; i < hmsc->max_lun; i++){
			tusbmemset(&hmsc->unit[i], 0, sizeof(hmsc->unit));
			hmsc->unit[i].state = MSC_UNIT_INIT;
		}
	}
	pdevice->numUnit = hmsc->max_lun;
	/* タイマースタート */
	pdevice->timeid = pdevice->timecount = 100;
	return TUSBH_E_OK;
}

/*
 *  MSCクラス終了設定
 *  parameter1 pdevice:  デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhMSCDeInit(TUSBH_Device_t *pdevice)
{
	MSC_Handle_t *hmsc;
	TUSBH_Handle_t *phost = pdevice->pHost;

	hmsc = (MSC_Handle_t *)pdevice->pData;
	if(hmsc->OutPipe != 0){
		tusbhClosePipe(pdevice, hmsc->OutPipe);
		tusbFreePipe(phost, hmsc->OutPipe);
		hmsc->OutPipe = 0;
	}

	if(hmsc->InPipe != 0){
		tusbhClosePipe(pdevice, hmsc->InPipe);
		tusbFreePipe(phost, hmsc->InPipe);
		hmsc->InPipe = 0;
	}

	if(pdevice->pData != NULL){
		tusbfree(pdevice->pData);
		pdevice->pData = NULL;
	}
	return TUSBH_E_OK;
}

/*
 *  MSCクラスプロセス実行
 *  parameter1 pdevice:  デバイスハンドラ
 *  parameter2 mes:      通信メッセージへのポインタ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhMSCProcess(TUSBH_Device_t *pdevice, uint8_t *mes)
{
	MSC_Handle_t   *hmsc;
	MSC_Unit_t     *punit;
	TUSBH_Handle_t *phost = pdevice->pHost;
	TUSBH_ERCODE status = TUSBH_E_OK;
	BOT_TYPE bot_status = BOT_BUSY;

	hmsc = (MSC_Handle_t *)pdevice->pData;
    if(mes[0] == TUSBH_URB_EVENT){
		hmsc->urb_wait = 0;
    }
	else if(mes[0] == TUSBH_TIME_EVENT){
		hmsc->timer += mes[3];
		pdevice->timecount = mes[3];
	}
	switch (hmsc->state){
	case MSC_PROCESS_INIT:
		if(hmsc->lun < hmsc->max_lun){
			status = tusbhMscSCSIDiskInitailize(hmsc, mes, hmsc->lun);
			tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
		}
		else{
			hmsc->lun = 0;
			hmsc->state = MSC_PROCESS_IDLE;
			if(phost->usrcallback != NULL)
				phost->usrcallback(phost, pdevice, HOST_USER_CLASS_ACTIVE);
		}
		break;
	case MSC_PROCESS_READ_WAIT:
	case MSC_PROCESS_WRITE_WAIT:
		if(hmsc->lun >= hmsc->max_lun)
			break;
		punit = &hmsc->unit[hmsc->lun];
		if(punit->rw_status == TUSBH_E_BUSY){
			bot_status = tusbhMscSCSITransfer(hmsc, mes, hmsc->lun);
			if(bot_status == BOT_SENSE){
				hmsc->state = MSC_PROCESS_REQUEST_SENSE_WAIT;
				tusbhMscSCSIRequestSense(hmsc, hmsc->lun);
				tusbSendData( phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);
			}
			else if(bot_status == BOT_ERROR){
				punit->rw_status = TUSBH_E_ERR;
			}
			else if(bot_status != BOT_BUSY){
				punit->rw_status = TUSBH_E_OK;
				if(punit->rw_taskid != 0)
					tusbWakeup(punit->rw_taskid);
			}
		}
		break;
    case MSC_PROCESS_REQUEST_SENSE_WAIT:
		if(hmsc->lun >= hmsc->max_lun)
			break;
		punit = &hmsc->unit[hmsc->lun];
		if(punit->rw_status == TUSBH_E_BUSY){
			bot_status = tusbhMscSCSITransfer(hmsc, mes, hmsc->lun);
			if(bot_status == BOT_OK){
				punit->rw_status = TUSBH_E_OK;
				if(punit->rw_taskid != 0)
					tusbWakeup(punit->rw_taskid);
			}
			else if(bot_status != BOT_BUSY){
				punit->rw_status = TUSBH_E_ERR;
			}
		}
		break;
	case MSC_PROCESS_IDLE:
		status = TUSBH_E_OK;
		break;
	default:
		break;
	}
	return status;
}

/*
 *  ユニット READY確認
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 lun:     unit#
 *  return     status:  (0: not ready / 1: ready)
 */
uint8_t
tusbhMscUnitIsReady(TUSBH_Handle_t *phost, uint8_t lun)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, MSC_Class.classCode, &lun);
	MSC_Handle_t *hmsc;

	if(pdevice == NULL)
		return 0;
	hmsc = (MSC_Handle_t *)pdevice->pData;

	if(pdevice->dstate == DEVICE_CLASS)
		return (hmsc->unit[lun].state >= MSC_UNIT_IDLE);
	else
		return 0;
}

/*
 *  ユニット情報を取得
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 lun:     unit#
 *  parameter3 info:    ユニット取得領域へのポインタ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhMscGetLUNInfo(TUSBH_Handle_t *phost, uint8_t lun, MSC_Unit_t *info)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, MSC_Class.classCode, &lun);
	MSC_Handle_t *hmsc;

	if(pdevice == NULL)
		return TUSBH_E_OBJ;
	hmsc = (MSC_Handle_t *) pdevice->pData;

	if(pdevice->dstate == DEVICE_CLASS){
		tusbmemcpy(info, &hmsc->unit[lun], sizeof(MSC_Unit_t));
		return TUSBH_E_OK;
	}
	else
		return TUSBH_E_OBJ;
}

/*
 *  MSC BLOCK READ
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 lun:     unit#
 *  parameter3 address: ユニットアドレス
 *  parameter4 pbuf:    受信データ領域へのポインタ
 *  parameter5 num:     受信ブロック数
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhMscRead(TUSBH_Handle_t *phost, uint8_t lun, uint32_t address, uint8_t *pbuf, uint32_t num)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, MSC_Class.classCode, &lun);
	MSC_Handle_t *hmsc;
	TUSBH_ERCODE status = TUSBH_E_BUSY;
	uint32_t i;

	if(pdevice != NULL){
		hmsc = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS || hmsc->unit[lun].state != MSC_UNIT_IDLE)
			return TUSBH_E_OBJ;
	}
	else
		return TUSBH_E_OBJ;
	if(lun >= hmsc->max_lun)
		return TUSBH_E_PAR;

	hmsc->state = MSC_PROCESS_READ_WAIT;
	hmsc->unit[lun].rw_status = TUSBH_E_BUSY;
	hmsc->unit[lun].rw_taskid = tusbGetTaskID();
	hmsc->unit[lun].state = MSC_UNIT_READ_WAIT;
	hmsc->lun = lun;

    /*
	 *  BLOCK READ CBW設定
	 */
	hmsc->hbot.cbw.DataTransferLength = num * 512;
	hmsc->hbot.cbw.Flags = USB_DEVICE_TO_HOST;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;
	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_CB_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_READ10; 

	hmsc->hbot.cbw.CB[2]  = (address >> 24) & 0xff;
	hmsc->hbot.cbw.CB[3]  = (address >> 16) & 0xff;
	hmsc->hbot.cbw.CB[4]  = (address >> 8) & 0xff;
	hmsc->hbot.cbw.CB[5]  = address & 0xff;

	hmsc->hbot.cbw.CB[7]  = (num >> 8) & 0xff;
	hmsc->hbot.cbw.CB[8]  = num & 0xff;

    hmsc->hbot.state = BOT_SEND_CBW;
    hmsc->hbot.pbuf = pbuf;
    tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);

	for(i = 0 ; i < (num * 1000) ; i++){
		tusbSleep(50);
		if((status = hmsc->unit[lun].rw_status) != TUSBH_E_BUSY)
			break;
	}
	if(status == TUSBH_E_BUSY)
		status = TUSBH_E_TMOUT;
	hmsc->unit[lun].rw_taskid = 0;
	hmsc->unit[lun].state = MSC_UNIT_IDLE;
	hmsc->state = MSC_PROCESS_IDLE;
	return status;
}

/*
 *  MSC BLOCK WRITE
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 lun:     unit#
 *  parameter3 address: ユニットアドレス
 *  parameter4 pbuf:    送信データ領域へのポインタ
 *  parameter5 num:     送信ブロック数
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhMscWrite(TUSBH_Handle_t *phost, uint8_t lun, uint32_t address, uint8_t *pbuf, uint32_t num)
{
	TUSBH_Device_t *pdevice = tusbhSearchDevice(phost, MSC_Class.classCode, &lun);
	MSC_Handle_t *hmsc;
	TUSBH_ERCODE status = TUSBH_E_BUSY;
	uint32_t i;

	if(pdevice != NULL){
		hmsc = pdevice->pData;
		if(pdevice->dstate != DEVICE_CLASS || hmsc->unit[lun].state != MSC_UNIT_IDLE)
			return TUSBH_E_OBJ;
	}
	else
		return TUSBH_E_OBJ;
	if(lun >= hmsc->max_lun)
		return TUSBH_E_PAR;

	hmsc->state = MSC_PROCESS_WRITE_WAIT;
	hmsc->unit[lun].rw_taskid = tusbGetTaskID();
	hmsc->unit[lun].rw_status = TUSBH_E_BUSY;
	hmsc->unit[lun].state = MSC_UNIT_WRITE_WAIT;
	hmsc->lun = lun;

    /*
	 *  BLOCK WRITE CBW設定
	 */
	hmsc->hbot.cbw.DataTransferLength = num * 512;
	hmsc->hbot.cbw.Flags = USB_HOST_TO_DEVICE;
	hmsc->hbot.cbw.CBLength = CBW_LENGTH;
	tusbmemset(hmsc->hbot.cbw.CB, 0, CBW_CB_LENGTH);
	hmsc->hbot.cbw.CB[0]  = OPCODE_WRITE10; 

	hmsc->hbot.cbw.CB[2]  = (address >> 24) & 0xff;
	hmsc->hbot.cbw.CB[3]  = (address >> 16) & 0xff;
	hmsc->hbot.cbw.CB[4]  = (address >> 8) & 0xff;
	hmsc->hbot.cbw.CB[5]  = (((uint8_t*)&address)[0]);

	hmsc->hbot.cbw.CB[7]  = (num >> 8) & 0xff;
	hmsc->hbot.cbw.CB[8]  = num & 0xff;

	hmsc->hbot.state = BOT_SEND_CBW;
	hmsc->hbot.pbuf = pbuf;
	tusbSendData(phost->process_event, TUSBH_CLASS_EVENT, pdevice->idx, 0, 0);

	for(i = 0 ; i < (num * 1000) ; i++){
		tusbSleep(50);
		if((status = hmsc->unit[lun].rw_status) != TUSBH_E_BUSY)
			break;
	}
	if(status == TUSBH_E_BUSY)
		status = TUSBH_E_TMOUT;
	hmsc->unit[lun].rw_taskid = 0;
	hmsc->unit[lun].state = MSC_UNIT_IDLE;
	hmsc->state = MSC_PROCESS_IDLE;
	return status;
}


