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
 *  @(#) $Id: dwc2_host_driver.c 698 2017-09-23 16:39:26Z roi $
 */
/*
 *  USB HOST HIGH DRIVERS
 */

#include "tusbh_base.h"
#include "kernel_cfg.h"
#include "usb_otg.h"

/*
 *  URBの変換テーブル
 */
static const uint8_t urb_table[8] = {
	TUSBH_URB_IDLE,
	TUSBH_URB_DONE,
	TUSBH_URB_NOTREADY,
	TUSBH_URB_NYET,
	TUSBH_URB_ERROR,
	TUSBH_URB_STALL,
	TUSBH_URB_ERROR,
	TUSBH_URB_ERROR
};

/*
 *  USB HOST CLASS リンク関数
 */
__attribute__((weak)) void tusbhLinkHID(TUSBH_Handle_t *phost)
{
}

__attribute__((weak)) void tusbhLinkMSC(TUSBH_Handle_t *phost)
{
}

__attribute__((weak)) void tusbhLinkSERIAL(TUSBH_Handle_t *phost)
{
}

__attribute__((weak)) void tusbhLinkPRT(TUSBH_Handle_t *phost)
{
}

__attribute__((weak)) void tusbhLinkHUB(TUSBH_Handle_t *phost)
{
}



#ifdef TOPPERS_MPCORE
/*
 *  IDCHANGE CALLBACK関数
 */
static void
dwc2_host_idchangecallback(USB_OTG_Handle_t *hhcd)
{
	tusbiSendData(((TUSBH_Handle_t *)(hhcd->pHost))->connect_event, TUSBH_IDCHANGE_EVENT, 0, 1, 0);
}
#endif	/* TOPPERS_MPCORE */

/*
 *  CONNECT CALLBACK関数
 */
static void
dwc2_host_connectcallback(USB_OTG_Handle_t *hhcd)
{
	tusbiSendData(((TUSBH_Handle_t *)(hhcd->pHost))->connect_event, TUSBH_CONNECT_EVENT, 0, 1, 0);
}

/*
 *  DISCONNECT CALLBACK関数
 */
static void
dwc2_host_disconnectcallback(USB_OTG_Handle_t *hhcd)
{
	tusbiSendData(((TUSBH_Handle_t *)(hhcd->pHost))->connect_event, TUSBH_DISCONNECT_EVENT, 0, 1, 0);
}

/*
 *  URB CHANGE CALLBACK関数
 */
static void
dwc2_host_changeurbcallback(USB_OTG_Handle_t *hhcd, uint8_t chnum, uint8_t urb_state)
{
	TUSBH_Handle_t *phost = hhcd->pHost;
	uint8_t urb;

	if(urb_state < 8)
		urb = urb_table[urb_state];
	else
		urb = TUSBH_URB_ERROR;
	tusbiSendData(phost->process_event, TUSBH_URB_EVENT, phost->pipes[chnum].index, chnum, urb);
}


/*
 *  USB_OTG DWC2 HIGH DRIVER INITIALIZE
 *  parameter1 phost:  USBホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_init(TUSBH_Handle_t *phost)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)(phost->pSysData);

	/*
	 *  RTOSリソース設定
	 */
	phost->processTaskID = TUSBH_P_TASK;
	phost->eventTaskID   = TUSBH_E_TASK;
	phost->process_event = USBH_EVT1_DTQ;
	phost->connect_event = USBH_EVT2_DTQ;
	phost->numPipes = husb->Init.host_channels;

	/*
	 *  USB-OTGドライバ設定
	 */
	husb->pHost = phost;

	/*
	 *  コールバック関数設定
	 */
#ifdef TOPPERS_MPCORE
	husb->otgidchangecallback    = dwc2_host_idchangecallback;
#endif	/* TOPPERS_MPCORE */
	husb->hostsofcallback        = NULL;
	husb->hostconnectcallback    = dwc2_host_connectcallback;
	husb->hostdisconnectcallback = dwc2_host_disconnectcallback;
	husb->hostchangeurbcallback  = dwc2_host_changeurbcallback;

	/*
	 *  USB-OTG初期化
	 */
	usbo_hostinit(husb);

	/*
	 *  USBホストクラスのリンク
	 */
	tusbhLinkHID(phost);
	tusbhLinkMSC(phost);
	tusbhLinkSERIAL(phost);
	tusbhLinkPRT(phost);
	tusbhLinkHUB(phost);
	return TUSBH_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER DE-INITIALIZE
 *  parameter1 phost:  USBホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_deinit(TUSBH_Handle_t *phost)
{
	USB_OTG_Handle_t *husb = phost->pSysData;

	/*
	 *  USB-OTG終了処理
	 */
	usbo_deinit(husb);

#if 0	/* ROI DEBUG クロック停止なので意味がない */
	usbo_disableglobalint(husb);
#endif	/* ROI DEBUG */
	return TUSBH_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER START
 *  parameter1 phost:  USBホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_start(TUSBH_Handle_t *phost)
{
	usbo_starthost(phost->pSysData);
	usbo_enableglobalint(phost->pSysData);
	return TUSBH_E_OK; 
}

/*
 *  USB_OTG DWC2 HIGH DRIVER GET SPEED
 *  parameter1 phost:  USBホストハンドラ
 *  return     usb speed value
 */
uint8_t
dwc2_host_getspeed(TUSBH_Handle_t *phost)
{
	uint8_t speed = USB_DEVICE_SPEED_FULL;

	switch(usbo_gethostspeed(phost->pSysData)){
	case 0:
		speed = USB_DEVICE_SPEED_HIGH;
		break;
	case 2:
		speed = USB_DEVICE_SPEED_LOW;
		break;
	case 1:
	default:
		speed = USB_DEVICE_SPEED_FULL;
		break;
	}
	return speed;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER OPEN PIPE
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  parameter3 epnum:   EP番号
 *  parameter4 dev_addr:デバイスアドレス
 *  parameter5 speed:   デバイススピード
 *  parameter6 type:    エンドポイントタイプ
 *  parameter7 eps:     MAXパケットサイズ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_openpipe(TUSBH_Handle_t *phost, uint8_t pipe, uint8_t epnum, uint8_t dev_addr,
					uint8_t speed, uint8_t type, uint16_t mps)
{
	USB_OTG_Handle_t  *husb = (USB_OTG_Handle_t *)(phost->pSysData);
	USB_OTG_HCTypeDef *phc = &husb->hc[pipe];

	phc->dev_addr = dev_addr;
	phc->max_packet = mps;
	phc->ch_num = pipe;
	phc->ep_type = type;
	phc->ep_num = epnum & 0x7F;
	phc->ep_is_in = ((epnum & 0x80) == 0x80);
	phc->speed = speed;

	if(usbo_hc_init(husb, pipe, epnum) == E_OK)
		return TUSBH_E_OK;
	else
		return TUSBH_E_ERR;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER CLOSE PIPE
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_closepipe(TUSBH_Handle_t *phost, uint8_t pipe)
{
	loc_cpu();
	usbo_hc_halt(phost->pSysData, pipe);
	unl_cpu();
	return TUSBH_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER SUBMIT URB
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  parameter3 dir:     接続方向:0Out/1:In
 *  parameter4 token:   エンドポイント設定タイプ:0:SETUP/1:DATA
 *  parameter5 pbuff:   通信領域へのポインタ
 *  parameter6 length:  通信サイズ
 *  parameter7 do_ping: DO PING設定
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_submiturb(TUSBH_Handle_t *phost, uint8_t pipe, uint8_t dir,
					uint8_t token, uint8_t* pbuff, uint16_t length, uint8_t do_ping)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)phost->pSysData;

#ifdef TOPPERS_MPCORE
	usbo_dma_wait(husb, pipe);
#endif	/* TOPPERS_MPCORE */
	husb->hc[pipe].ep_is_in = dir;

	if(token == 0)
		husb->hc[pipe].data_pid = HC_PID_SETUP;
	else
		husb->hc[pipe].data_pid = HC_PID_DATA1;

	/* Manage Data Toggle */
	switch(husb->hc[pipe].ep_type){
	case EP_TYPE_CTRL:
		if(token == 1 && dir == 0){ /*send data */
			if(length == 0){	/* For Status OUT stage, Length==0, Status Out PID = 1 */
				husb->hc[pipe].toggle_out = 1;
			}

			/* Set the Data Toggle bit as per the Flag */
			if(husb->hc[pipe].toggle_out == 0){	/* Put the PID 0 */
				husb->hc[pipe].data_pid = HC_PID_DATA0;
			}
			else{	/* Put the PID 1 */
				husb->hc[pipe].data_pid = HC_PID_DATA1;
			}
			if(husb->hc[pipe].urb_state != URB_NOTREADY){
				husb->hc[pipe].do_ping = do_ping;
			}
		}
		break;

	case EP_TYPE_BULK:
		if(dir == 0){
			/* Set the Data Toggle bit as per the Flag */
			if(husb->hc[pipe].toggle_out == 0){	/* Put the PID 0 */
				husb->hc[pipe].data_pid = HC_PID_DATA0;
			}
			else{	/* Put the PID 1 */
				husb->hc[pipe].data_pid = HC_PID_DATA1;
			}
			if(husb->hc[pipe].urb_state != URB_NOTREADY){
				husb->hc[pipe].do_ping = do_ping;
			}
		}
		else{
			if(husb->hc[pipe].toggle_in == 0){
				husb->hc[pipe].data_pid = HC_PID_DATA0;
			}
			else{
				husb->hc[pipe].data_pid = HC_PID_DATA1;
			}
		}
		break;
	case EP_TYPE_INTR:
		if(dir == 0){
			/* Set the Data Toggle bit as per the Flag */
			if(husb->hc[pipe].toggle_out == 0){	/* Put the PID 0 */
				husb->hc[pipe].data_pid = HC_PID_DATA0;
			}
			else{	/* Put the PID 1 */
				husb->hc[pipe].data_pid = HC_PID_DATA1;
			}
		}
		else{
			if(husb->hc[pipe].toggle_in == 0){
				husb->hc[pipe].data_pid = HC_PID_DATA0;
			}
			else{
				husb->hc[pipe].data_pid = HC_PID_DATA1;
			}
		}
		break;
	case EP_TYPE_ISOC: 
		husb->hc[pipe].data_pid = HC_PID_DATA0;
		break;
	}

	husb->hc[pipe].xfer_buff = pbuff;
	husb->hc[pipe].xfer_len  = length;
	husb->hc[pipe].urb_state = URB_IDLE;
	husb->hc[pipe].xfer_count = 0;
	husb->hc[pipe].ch_num = pipe;
	husb->hc[pipe].state = USBO_HC_IDLE;

	usbo_hc_startxfer(husb, pipe);
	return TUSBH_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER CHECK PIPE STATE
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  return     TUSBH_ERCODE
 */
bool_t
dwc2_host_checkpipe(TUSBH_Handle_t *phost, uint8_t pipe)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)(phost->pSysData);

	if(pipe < husb->Init.host_channels && husb->hc[pipe].state != USBO_HC_FAIL)
		return true;
	else
		return false;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER SET PIPE TOGGLE VALUE
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  parameter3 toggle:  トグル値
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
dwc2_host_settoggle(TUSBH_Handle_t *phost, uint8_t pipe, uint8_t toggle)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)(phost->pSysData);

	if(husb->hc[pipe].ep_is_in){
		husb->hc[pipe].toggle_in = toggle;
	}
	else{
		husb->hc[pipe].toggle_out = toggle;
	}
	return TUSBH_E_OK; 
}

/*
 *  USB_OTG DWC2 HIGH DRIVER GET PIPE TOGGLE VALUE
 *  parameter1 phost:   USBホストハンドラ
 *  parameter2 pipe:    PIPE#
 *  return    トグル値
 */
uint8_t
dwc2_host_gettoggle(TUSBH_Handle_t *phost, uint8_t pipe)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)(phost->pSysData);
	uint8_t toggle = 0;

	if(husb->hc[pipe].ep_is_in){
		toggle = husb->hc[pipe].toggle_in;
	}
	else{
		toggle = husb->hc[pipe].toggle_out;
	}
	return toggle;
}

