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
 *  @(#) $Id: dwc2_device_driver.c 698 2017-10-28 18:10:40Z roi $
 */
/*
 *  USB DEVICE HIGH DRIVERS
 */

#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"

#include "stdlib.h"
#include "tusbd_base.h"
#include "usb_otg.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))

#ifndef MAX_DEVICE_EPS
#define MAX_DEVICE_EPS          (MAX_EPS_CHANNELS-1)
#endif

/*
 *  SETUPSTAGE CALLBACK関数
 */
static void
tusbdSetupStageCallback(USB_OTG_Handle_t *husb)
{
	tusbdSetupStage(husb->pDev, (uint8_t *)husb->Setup);
}

/*
 *  DATAOUTSTAGE CALLBACK関数
 */
static void
tusbdDataOutStageCallback(USB_OTG_Handle_t *husb, uint8_t epnum)
{
	tusbdDataOutStage(husb->pDev, epnum, husb->OUT_ep[epnum].xfer_buff);
}

/*
 *  DATAINSTAGE CALLBACK関数
 */
static void
tusbdDataInStageCallback(USB_OTG_Handle_t *husb, uint8_t epnum)
{
	tusbdDataInStage(husb->pDev, epnum, husb->IN_ep[epnum].xfer_buff);
}

/*
 *  DEVICE RESET CALLBACK関数
 */
static void
tusbdResetCallback(USB_OTG_Handle_t *husb)
{
	uint8_t speed = USB_DEVICE_SPEED_FULL;

	/* Set USB Current Speed */
	switch(husb->Init.speed){
	case USB_SPEED_HIGH:
		speed = USB_DEVICE_SPEED_HIGH;
		break;
	case USB_SPEED_FULL:
		speed = USB_DEVICE_SPEED_FULL;
		break;
	default:
		speed = USB_DEVICE_SPEED_FULL;
		break;
	}

	/*
	 *  DEVICEのリセット
	 */
	tusbdReset(husb->pDev);
	tusbdSetSpeed(husb->pDev, speed);
}

/*
 *  DEVICE SUSPEND CALLBACK関数
 */
static void
tusbdSuspendCallback(USB_OTG_Handle_t *husb)
{
	tusbdSuspend(husb->pDev);
#if 1	/* ROI DEBUG */
	syslog_0(LOG_NOTICE, "## tusbdSuspendCallback ##");
#endif
}

/*
 *  DEVICE RESUME CALLBACK関数
 */
static void
tusbdResumeCallback(USB_OTG_Handle_t *husb)
{
	tusbdResume(husb->pDev);
#if 1	/* ROI DEBUG */
	syslog_0(LOG_NOTICE, "## tusbdResumeCallback ##");
#endif
}

/*
 *  ISOOUTCOMPLETE CALLBACK関数
 */
static void
tusbdISOOUTIncompleteCallback(USB_OTG_Handle_t *husb, uint8_t epnum)
{
	tusbdIsoOutIncomplete(husb->pDev, epnum);
}

/*
 *  ISOINCOMPLETE CALLBACK関数
 */
static void
tusbdISOINIncompleteCallback(USB_OTG_Handle_t *husb, uint8_t epnum)
{
	tusbdIsoInIncomplete(husb->pDev, epnum);
}

/*
 *  DEVICE CONNECT CALLBACK関数
 */
static void
tusbdConnectCallback(USB_OTG_Handle_t *husb)
{
	tusbdDeviceConnected(husb->pDev);
}

/*
 *  DEVICE DISCONNECT CALLBACK関数
 */
static void
tusbdDisconnectCallback(USB_OTG_Handle_t *husb)
{
	tusbdDeviceDisconnected(husb->pDev);
}

/*
 *  ENDPOINT取得
 */
static USB_OTG_EPTypeDef *
tusbdGetEndpoint(USB_OTG_Handle_t *husb, uint8_t addr)
{
	USB_OTG_EPTypeDef *ep;

	if((addr & USB_DEVICE_TO_HOST) != 0){
		ep = &husb->IN_ep[addr & 0x7F];
	}
	else{
		ep = &husb->OUT_ep[addr & 0x7F];
	}
	ep->num   = addr & 0x7F;
	ep->is_in = (USB_DEVICE_TO_HOST & addr) != 0;
	return ep;
}


/*
 *  USB_OTG DWC2 HIGH DRIVER INITIALIZE
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_init(TUSBD_Handle_t *pdevice)
{
	USB_OTG_Handle_t *husb = (USB_OTG_Handle_t *)(pdevice->pSysData);
	uint32_t i = 0;

	/*
	 *  USBハンドラに、USBデバイスハンドラをセット
	 */
	husb->pDev = pdevice;

	/*
	 *  コールバック関数をセット
	 */
	husb->devsetupstagecallback   = tusbdSetupStageCallback;
	husb->devdataoutstagecallback = tusbdDataOutStageCallback;
	husb->devdatainstagecallback  = tusbdDataInStageCallback;
	husb->devsofcallback          = NULL;
	husb->devresetcallback        = tusbdResetCallback;
	husb->devsuspendcallback      = tusbdSuspendCallback;
	husb->devresumecallback       = tusbdResumeCallback;
	husb->devisooutcallback       = tusbdISOOUTIncompleteCallback;
	husb->devisoincallback        = tusbdISOINIncompleteCallback;
	husb->devconnectcallback      = tusbdConnectCallback;
	husb->devdisconnectcallback   = tusbdDisconnectCallback;
	husb->devlpmcallback          = NULL;

	/*
	 *  エンドポイントの初期化
	 */
	for(i = 0 ; i < MAX_DEVICE_EPS ; i++){
		husb->IN_ep[i].is_in = 1;
		husb->IN_ep[i].num = i;
		husb->IN_ep[i].tx_fifo_num = i;
		husb->IN_ep[i].type = EP_TYPE_CTRL;
		husb->IN_ep[i].maxpacket = 0;
		husb->IN_ep[i].xfer_buff = 0;
		husb->IN_ep[i].xfer_len = 0;

		husb->OUT_ep[i].is_in = 0;
		husb->OUT_ep[i].num = i;
		husb->OUT_ep[i].tx_fifo_num = 0;
 		husb->OUT_ep[i].type = EP_TYPE_CTRL;
		husb->OUT_ep[i].maxpacket = 0;
		husb->OUT_ep[i].xfer_buff = 0;
		husb->OUT_ep[i].xfer_len = 0;
	}

	/*
	 *  USBデバイスの初期化
	 */
	usbo_devinit(husb);

	/*
	 *  LPMの有効化
	 */
	usbo_init_lpm(husb);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER DE-INITIALIZE
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
  */
TUSBD_ERCODE
dwc2_device_deinit(TUSBD_Handle_t *pdevice)
{
	/*
	 *  デバイス停止
	 */
	dwc2_device_stop(pdevice);

	/*
	 *  デバイスハードウェア終了
	 */
	usbo_deinit(pdevice->pSysData);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER START
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
  */
TUSBD_ERCODE
dwc2_device_start(TUSBD_Handle_t *pdevice)
{
	usbo_devconnect(pdevice->pSysData);
	dly_tsk(3);
	usbo_enableglobalint(pdevice->pSysData);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER STOP
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
  */
TUSBD_ERCODE
dwc2_device_stop(TUSBD_Handle_t *pdevice)
{
	usbo_disableglobalint(pdevice->pSysData);
	usbo_stopdevice(pdevice->pSysData);
	usbo_devdisconnect(pdevice->pSysData);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER OPEN ENDPOINT
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  parameter3 type:  endpointタイプ
 *  parameter4 mps:   MAX PACKET SIZE
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_openep(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t type, uint16_t mps)
{
	USB_OTG_Handle_t  *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, addr);
	ep->maxpacket = mps;
	ep->type = type;
	if(ep->is_in){
		/* 送信FIFO設定 */
		ep->tx_fifo_num = ep->num;
	}

	/*
	 *  DATA PIDの初期化
	 */
	if (type == EP_TYPE_BULK){
		ep->data_pid_start = 0;
	}

	usbo_activateEndpoint(husb, ep);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER CLOSE ENDPOINT
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_closeep(TUSBD_Handle_t *pdevice, uint8_t addr)
{
	USB_OTG_Handle_t  *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, addr);
	usbo_deactivateEndpoint(husb, ep);
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER FLUSH ENDPOINT
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_flushep(TUSBD_Handle_t *pdevice, uint8_t addr)
{
	USB_OTG_Handle_t *husb = pdevice->pSysData;

	if((addr & USB_DEVICE_TO_HOST) != 0){
		usbo_flushTxFifo(husb, addr & 0x7F);
	}
	else{
		usbo_flushRxFifo(husb);
	}
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER STALL ENDPOINT
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_stallep(TUSBD_Handle_t *pdevice, uint8_t addr)
{
	USB_OTG_Handle_t  *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, addr);
	ep->is_stall = 1;

	usbo_epsetStall(husb, ep);
	if((addr & 0x7FU) == 0U){
		usbo_ep0_outstart(husb, (uint8_t *)husb->Setup);
	}
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER CLEAR STALL-ENDPOINT
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_clearep(TUSBD_Handle_t *pdevice, uint8_t addr)
{
	USB_OTG_Handle_t *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, addr);
	ep->is_stall = 0;

	usbo_epclearStall(husb, ep);
	return TUSBD_E_OK; 
}

/*
 *  USB_OTG DWC2 HIGH DRIVER GET STALL CONDITION
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  return     1:stall 0:not stall
 */
uint8_t
dwc2_device_getstallcondition(TUSBD_Handle_t *pdevice, uint8_t addr)
{
	USB_OTG_Handle_t *husb = pdevice->pSysData;

	if((addr & USB_DEVICE_TO_HOST) != 0)
		return husb->IN_ep[addr & 0x7F].is_stall;
	else
		return husb->OUT_ep[addr & 0x7F].is_stall;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER TRANSMIT A DATA
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  parameter3 pbuf:  送信データバッファ
 *  parameter4 size:  送信サイズ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_transmitdata(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t *pbuf, uint16_t size)
{
	USB_OTG_Handle_t  *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, (addr | USB_DEVICE_TO_HOST));

	/*
	 *  送信データ設定
	 */
	ep->xfer_buff = pbuf;
	ep->xfer_len = size;
	ep->xfer_count = 0;

	if(husb->Init.dma_enable == 1)
		ep->dma_addr = (uint32_t)pbuf;

	if((addr & 0x7F) == 0U){
		usbo_ep0startxfer(husb, ep);
	}
	else{
		usbo_epstartxfer(husb, ep);
	}
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER START to RECEIVE
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  parameter2 addr:  endpointアドレス
 *  parameter3 pbuf:  受信データバッファ
 *  parameter4 size:  受信サイズ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_startreceive(TUSBD_Handle_t *pdevice, uint8_t addr, uint8_t *pbuf, uint16_t size)
{
	USB_OTG_Handle_t  *husb = pdevice->pSysData;
	USB_OTG_EPTypeDef *ep;

	ep = tusbdGetEndpoint(husb, (addr & ~USB_DEVICE_TO_HOST));

	/*
	 *  受信データ設定
	 */
	ep->xfer_buff = pbuf;
	ep->xfer_len = size;
	ep->xfer_count = 0;

	if(husb->Init.dma_enable == 1)
		ep->dma_addr = (uint32_t)pbuf;

	if((addr & 0x7F) == 0){
		usbo_ep0startxfer(husb, ep);
	}
	else{
		usbo_epstartxfer(husb, ep);
	}
	return TUSBD_E_OK;
}

/*
 *  USB_OTG DWC2 HIGH DRIVER TEST MODE
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
 */
TUSBD_ERCODE
dwc2_device_testmode(TUSBD_Handle_t *pdevice)
{
	return TUSBD_E_OK;
}

#ifndef TOPPERS_MPCORE
/*
 *  USB_OTG DWC2 HIGH DRIVER REMOTE WAKEUP
 *  parameter1 pdevice:  USBデバイスハンドラ
 *  return     TUSBD_ERCODE
 */
void
dwc2_device_wakeup(USBD_HandleTypeDef *pdevice)
{
	USB_OTG_Handle_t *husb = pdevice->pSysData;

	if(pdevice->dev_remote_wakeup == 1 && pdevice->dev_state == TUSBD_STATE_SUSPENDED){
		if(husb->Init.low_power_enable){
			/* Reset SLEEPDEEP bit of Cortex System Control Register */
			sil_andw_mem((uint32_t *)(TADR_SCB_BASE+TOFF_SCB_SCR), (SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
			sysclock_config(1);
		}

		/* Ungate PHY clock */
		sil_andw_mem((uint32_t *)(USBPGC_BASE(husb->base)), USB_OTG_PCGCCTL_STOPCLK);

		/* Activate Remote wakeup */
		if((sil_rew_mem((uint32_t *)(USBD_BASE(husb->base)+TOFF_USBD_DSTS)) & USB_OTG_DSTS_SUSPSTS) != 0){
			/* Activate Remote wakeup signaling */
			sil_orw_mem((uint32_t *)(USBD_BASE(husb->base)+TOFF_USBD_DCTL), USB_OTG_DCTL_RWUSIG);
		}

		/* Remote wakeup delay */
		sil_dly_nse(10*1000*1000);

		/* Disable Remote wakeup */
		sil_andw_mem((uint32_t *)(USBD_BASE(husb->base)+TOFF_USBD_DCTL), USB_OTG_DCTL_RWUSIG);

		/* change state to configured */
		pdevice->dev_state = TUSBD_STATE_CONFIGURED;

		/* Change remote_wakeup feature to 0*/
		pdevice->dev_remote_wakeup = 0;
	}
}
#endif	/* TOPPERS_MPCORE */

