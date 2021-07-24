/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
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
 *  @(#) $Id: usb_device.c 698 2017-11-07 22:49:28Z roi $
 */
/*
 * STM USB DEVICE用デバイスドライバ
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <sil.h>
#include "t_syslog.h"
#include "device.h"
#include "stm32l0xx.h"
#include "usb_device.h"

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_orh_mem(a, b)		sil_wrh_mem((a), sil_reh_mem(a) | (b))
#define sil_andh_mem(a, b)		sil_wrh_mem((a), sil_reh_mem(a) & ~(b))

/*
 *  USBポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_USB(usbid)        ((uint_t)((usbid) - 1))


#define BTABLE_ADDRESS                  (0x000U)  

#define EP0                             ((uint8_t)0)

#define USBD_DEFAULT_IMASK (USB_CNTR_CTRM  | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM \
							| USB_CNTR_ESOFM | USB_CNTR_RESETM)
#define USBD_DTORR_VALUE   (USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_RX)
#define USBD_DTORT_VALUE   (USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_TX)

#define USB_OTG_FS_WAKEUP_EXTI_LINE     (EXTI_IMR_MR18)	/* External interrupt line 18 Connected to the USB FS EXTI Line */


/*
 *  エンドポイント・アドレス取出しマクロ
 */
#define GET_ENDPOINT_ADDR(b, bEpNum)            ((b)+TOFF_USB_EP0R + (bEpNum) * 4)
#define GET_ENDPOINT_TX_ADDRESS(b, bEpNum)      ((uint16_t *)((sil_reh_mem((uint16_t *)((b)+TOFF_USB_BTABLE))+bEpNum*8U)+  ((b) + 0x400U)))
#define GET_ENDPOINT_TX_CNT_ADDRESS(b, bEpNum)  ((uint16_t *)((sil_reh_mem((uint16_t *)((b)+TOFF_USB_BTABLE))+bEpNum*8U+2U)+  ((b) + 0x400U)))
#define GET_ENDPOINT_RX_ADDRESS(b, bEpNum)      ((uint16_t *)((sil_reh_mem((uint16_t *)((b)+TOFF_USB_BTABLE))+bEpNum*8U+4U)+  ((b) + 0x400U)))
#define GET_ENDPOINT_RX_CNT_ADDRESS(b, bEpNum)  ((uint16_t *)((sil_reh_mem((uint16_t *)((b)+TOFF_USB_BTABLE))+bEpNum*8U+6U)+  ((b) + 0x400U)))


static const uint16_t ep_type_table[4] = {USB_EP_CONTROL, USB_EP_ISOCHRONOUS, USB_EP_BULK, USB_EP_INTERRUPT};
static USB_DEV_Handle_t Husb[NUM_USBPORT];

/*
 *  ダブルバッファ・ユーザーバッファ解放
 */
static void
usbd_freeuserbuffer(volatile uint16_t *pendp, uint8_t is_in)
{
	if(is_in)	/* IN ダブル・バッファエンドポイント */
		*pendp = (*pendp & USB_EPREG_MASK) | USBD_DTORR_VALUE;
	else	/* OUT ダブル・バッファエンドポイント */
		*pendp = (*pendp & USB_EPREG_MASK) | USBD_DTORT_VALUE;
}

/*
 *  送信DTOGトグル
 */
static void
usbd_toggle_txdtog(volatile uint16_t *pendp, uint16_t valid)
{
	uint16_t value = *pendp & USB_EPTX_DTOGMASK;
	if((USB_EPTX_DTOG1 & valid) != 0)	/* 1stビットトグル ? */
		value ^= USB_EPTX_DTOG1;
	if((USB_EPTX_DTOG2 & valid) != 0)	/* 2ndビットトグル ?  */
		value ^= USB_EPTX_DTOG2;
	*pendp = (value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

/*
 *  受信DTOGトグル
 */
static void
usbd_toggle_rxdtog(volatile uint16_t *pendp, uint16_t valid)
{
	uint16_t value = *pendp & USB_EPRX_DTOGMASK;
	if((USB_EPRX_DTOG1 & valid) != 0)	/* 1stビットトグル ? */
		value ^= USB_EPRX_DTOG1;
	if((USB_EPRX_DTOG2 & valid) != 0)	/* 2ndビットトグル ?  */
		value ^= USB_EPRX_DTOG2;
	*pendp = (value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

/*
 *  受信バッファカウンタ設定
 */
static void
usbd_calc_blk(uint16_t *dwReg, uint16_t len)
{
	uint16_t wNBlocks;

	if(len > 62){
		wNBlocks = len >> 5;
		if((len & 0x1f) == 0)
			wNBlocks--;
		*dwReg = (uint16_t)((wNBlocks << 10U) | 0x8000U);
	}
	else{
		wNBlocks = len >> 1;
		if((len & 0x1) != 0)
			wNBlocks++;
	    *dwReg = (uint16_t)(wNBlocks << 10U);
	}
}

/*
 *  PMAメモリへのデータ書込み
 */
static void
usbd_writePMA(uint32_t base, uint8_t *src, uint16_t addr, uint16_t len)
{
	uint32_t n = (len + 1) >> 1;
	uint32_t i;
	uint16_t *p = (uint16_t *)(addr + base + 0x400U);

	for(i = 0; i < n ; i++, src += 2)
		*p++ = src[0] | (src[1] << 8);
}

/*
 *  PMAメモリからのデータ読み出し
 */
static void
usbd_readPMA(uint32_t base, uint8_t *dst, uint16_t wPMABufAddr, uint16_t len)
{
	uint32_t n = (len + 1) >> 1;
	uint32_t i;
	uint16_t *p, tmp;
	p = (uint16_t *)(wPMABufAddr + base + 0x400U);
	for(i = 0 ; i < n ; i++){
		tmp = *p++;
		*dst++ = tmp & 0xff;
		*dst++ = tmp >> 8;
	}
}


/*
 *  USB DEVICEハードウェア初期化
 *  parameter1 portid USBポートID
 *  parameter2 pini   初期化構造体へのポインタ
 *  return     NULLでなければUSBハンドラへのポインタ
 */
USB_DEV_Handle_t *
usbd_init(int portid, USB_DEV_Init_t *pini)
{
	GPIO_Init_t GPIO_Init_Data;
	USB_DEV_Handle_t *husb;
	uint32_t         no;
	uint16_t         imask = USBD_DEFAULT_IMASK;

	if(portid < USB1_PORTID || portid > NUM_USBPORT || pini == NULL)
		return NULL;

	no = INDEX_USB(portid);
	husb = &Husb[no];
	memset(husb, 0, sizeof(USB_DEV_Handle_t));

	/*
	 *  GPIOAクロック供給
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_IOPENR), RCC_IOPENR_GPIOAEN);

	/*
	 *  USB DM/DPピン設定
	 */
	GPIO_Init_Data.mode      = GPIO_MODE_INPUT;
	/* プルアップ プロダウン設定 */
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	/* 出力モード設定 */
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	/* スピード設定 */
	GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
	gpio_setup(TADR_GPIOA_BASE, &GPIO_Init_Data, 11);
	gpio_setup(TADR_GPIOA_BASE, &GPIO_Init_Data, 12);

	/*
	 *  USB DEVICEクロック供給
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR), RCC_APB1ENR_USBEN);

	memcpy(&husb->Init, pini, sizeof(USB_DEV_Init_t));
	if(pini->low_power_enable == 1){	/* Enable EXTI Line 18 for USB wakeup */
		sil_wrw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_PR), USB_OTG_FS_WAKEUP_EXTI_LINE);
		sil_andw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_FTSR), USB_OTG_FS_WAKEUP_EXTI_LINE);
		sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_RTSR), USB_OTG_FS_WAKEUP_EXTI_LINE);
		sil_orw_mem((uint32_t *)(TADR_EXTI_BASE+TOFF_EXTI_IMR), USB_OTG_FS_WAKEUP_EXTI_LINE);
	}

	husb->base = TADR_USB_BASE;

	/*
	 *  デバイス初期化
	 */
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_FRES);
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), 0x0000);
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), 0x0000);
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_BTABLE), BTABLE_ADDRESS);

	/*
	 *  割込み設定
	 */
	if(pini->sof_enable == 1)
		imask |= USB_CNTR_SOFM;
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USBD_DEFAULT_IMASK);

	husb->usb_address = 0;
	return husb;
}

/*
 *  USB DEVICEハードウェア停止
 *  parameter1 husb  USBハンドラへのポインタ
 *  return     常にE_OK
 */
ER
usbd_deinit(USB_DEV_Handle_t *husb)
{
	/*
	 *  ハンドラの確認
	 */
	if(husb == NULL){
		return E_PAR;
	}

	/*
	 *  USB FS クロック停止
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB1ENR), RCC_APB1ENR_USBEN);
	return E_OK;
}


/*
 *  デバイスコネクト
 *  parameter1 husb  USBハンドラへのポインタ
 *  return     常にE_OK
 */
ER
usbd_devconnect(USB_DEV_Handle_t *husb)
{
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_DPPU);
	return E_OK;
}

/*
 *  デバイスディスコネクト
 *  parameter1 husb  USBハンドラへのポインタ
 *  return     常にE_OK
 */
ER
usbd_devdisconnect(USB_DEV_Handle_t *husb)
{
	/*
	 *  USB DEVICEリセット
	 */
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_FRES);

	/*
	 *  割込みクリア
	 */
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), 0);

	/*
	 *  POWER DOWN
	 */
	sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), (USB_CNTR_FRES | USB_CNTR_PDWN));
	return E_OK;
}

/*
 *  エンドポイント用割込みハンドラ
 */
static ER
usbd_endpoint_handler(USB_DEV_Handle_t *husb)
{
	USB_DEV_EPTypeDef *ep;
	uint8_t  num;
	volatile uint16_t istr;
	volatile uint16_t *pEndP;

	while(((istr = sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR))) & USB_ISTR_CTR) != 0){
		/*
		 *  エンドポイント番号取得
		 */
		if((num = (uint8_t)(istr & USB_ISTR_EP_ID)) == 0){	/* EP0 */
			pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, EP0);
			if((istr & USB_ISTR_DIR) == 0){	/* ep0_in */
				*pEndP = (*pEndP & ((~USB_EP_CTR_TX) /*0xFF7FU*/ & USB_EPREG_MASK));
				ep = &husb->IN_ep[0];
				ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
				ep->xfer_buff += ep->xfer_count;

				/* TX COMPLETE */
				if(husb->devdatainstagecallback != NULL)
					husb->devdatainstagecallback(husb, 0U);

				if(husb->usb_address > 0 && ep->xfer_len == 0){
					sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_DADDR), (husb->usb_address | USB_DADDR_EF));
					husb->usb_address = 0;
				}

			}
			else{	/* ep0_out */
				ep = &husb->OUT_ep[0];
				if((*pEndP & USB_EP_SETUP) != 0){	/* setup受信 */
					ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
					usbd_readPMA(husb->base, (uint8_t*)husb->Setup ,ep->pmaadress , ep->xfer_count);
					*pEndP = (*pEndP & ((~USB_EP_CTR_RX) & USB_EPREG_MASK));

					/*
					 *  セットアップステージコールバック
					 */
					if(husb->devsetupstagecallback != NULL)
						husb->devsetupstagecallback(husb);
				}
				else if((*pEndP & USB_EP_CTR_RX) != 0){	/* data受信 */
					*pEndP = (*pEndP & ((~USB_EP_CTR_RX) & USB_EPREG_MASK));
					/*
					 *  受信データ取得
					 */
					ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
					if(ep->xfer_count != 0){
						usbd_readPMA(husb->base, ep->xfer_buff, ep->pmaadress, ep->xfer_count);
						ep->xfer_buff += ep->xfer_count;
					}

					/*
					 *  DATA OUTステージコールバック
					 */
					if(husb->devdataoutstagecallback != NULL)
						husb->devdataoutstagecallback(husb, 0);
					usbd_calc_blk(GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num), ep->maxpacket);
					usbd_toggle_rxdtog(pEndP, USB_EP_RX_VALID);
				}
			}
		}
		else{	/* epn */
			uint16_t count=0;

			/*
			 *  エンドポイントアドレス取得
			 */
			pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, num);
			if((*pEndP & USB_EP_CTR_RX) != 0){	/* epn 受信 */
				ep = &husb->OUT_ep[num];
				/*
				 *  割込みクリア
				 */
				*pEndP = (*pEndP & ((~USB_EP_CTR_RX) & USB_EPREG_MASK));

				/* OUT double Buffering*/
				if(ep->doublebuffer == 0){
					count = ((uint16_t)(*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
					if(count != 0)
						usbd_readPMA(husb->base, ep->xfer_buff, ep->pmaadress, count);
				}
				else{
					pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);
					if((*pEndP & USB_EP_DTOG_RX) != 0){	/* BUF0読み出し */
						count = ((uint16_t)(*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
						if(count != 0)
							usbd_readPMA(husb->base, ep->xfer_buff, ep->pmaaddr0, count);
					}
					else{	/* BUF1読み出し */
						count = ((uint16_t)(*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
						if(count != 0)
							usbd_readPMA(husb->base, ep->xfer_buff, ep->pmaaddr1, count);
					}
					usbd_freeuserbuffer(pEndP, 0);
				}
				/*multi-packet on the NON control OUT endpoint*/
				ep->xfer_count += count;
				ep->xfer_buff  += count;

				if(ep->xfer_len == 0 || count < ep->maxpacket){
					/*
					 * 受信終了
					 */
					if(husb->devdataoutstagecallback != NULL)
						husb->devdataoutstagecallback(husb, ep->num);
				}
				else{	/* 受信設定 */
					ep->xfer_count = 0;
					ep->is_in = 0;
					usbd_epstartreceive(husb, ep);
				}
			}

			if((*pEndP & USB_EP_CTR_TX) != 0){	/* epn送信 */
				ep = &husb->IN_ep[num];
				/*
				 * 割込みクリア
				 */
				*pEndP = (*pEndP & ((~USB_EP_CTR_TX) & USB_EPREG_MASK));

				if(ep->doublebuffer == 0){	/* シングル・バッファ */
					ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
					if(ep->xfer_count != 0)
						usbd_writePMA(husb->base, ep->xfer_buff, ep->pmaadress, ep->xfer_count);
				}
				else{	/* ダブル・バッファ */
					if((*pEndP & USB_EP_DTOG_TX) != 0){	/* BUF0設定 */
						ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
						if(ep->xfer_count != 0)
							usbd_writePMA(husb->base, ep->xfer_buff, ep->pmaaddr0, ep->xfer_count);
					}
					else{	/* BUF1設定 */
						ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
						if(ep->xfer_count != 0)
							usbd_writePMA(husb->base, ep->xfer_buff, ep->pmaaddr1, ep->xfer_count);
					}
					usbd_freeuserbuffer(pEndP, 1);
				}
				/*
				 * 送信長を取得
				 */
				ep->xfer_count = ((uint16_t)(*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num)) & 0x3ffU);
				ep->xfer_buff += ep->xfer_count;
				if(ep->xfer_len == 0){	/* 転送終了 ? */
					if(husb->devdatainstagecallback != NULL)
						husb->devdatainstagecallback(husb, ep->num);
				}
				else{	/* 次のデータを設定 */
					ep->xfer_count = 0U;
					ep->is_in = 1;
					usbd_epstartsend(husb, ep);
				}
			}
		}
	}
	return E_OK;
}

/*
 *  USB-DEVICE割込みサービスルーチン
 */
void
usb_device_isr(intptr_t exinf)
{
	USB_DEV_Handle_t *husb = &Husb[INDEX_USB((uint32_t)exinf)];
	uint32_t mask = 0;
	uint32_t isr = sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR));

	if((isr & USB_ISTR_CTR) != 0)	/* ENDPOINT割込み */
		usbd_endpoint_handler(husb);

	isr = sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR));
	if((isr & USB_ISTR_RESET) != 0){	/* RESET割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_RESET);
		if(husb->devresetcallback != NULL)
			husb->devresetcallback(husb);
		usbd_setDevAddress(husb, 0U);
	}

	if((isr & USB_ISTR_PMAOVR) != 0){
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_PMAOVR);
	}

	if((isr & USB_ISTR_ERR) != 0){	/* ERROR割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_ERR);
	}

	if((isr & USB_ISTR_WKUP) != 0){	/* RESUME割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_LPMODE);

		/*
		 * USB DEVICE割込み設定
		 */
		mask = USBD_DEFAULT_IMASK;
		if(husb->Init.sof_enable == 1)
			mask |= USB_CNTR_SOFM;

		if(husb->Init.lpm_enable == 1){	/* LPM有効設定 */
			mask |= USB_CNTR_L1REQM;
			husb->lpm_active = 1;
			husb->lpm_state = PCD_LPM_L0_ACTIVE;

			sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_LPMCSR), USB_LPMCSR_LMPEN);
			sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_LPMCSR), USB_LPMCSR_LPMACK);
		}
		sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), mask);

		if(husb->devresumecallback != NULL)
			husb->devresumecallback(husb);

		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_WKUP);
	}

	if((isr & USB_ISTR_SUSP) != 0){	/* SUSPEND割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_SUSP);

		/* Force low-power mode in the macrocell */
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_FSUSP);
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_LPMODE);

		if((isr & USB_ISTR_WKUP) == 0){
			if(husb->devsuspendcallback != NULL)
				husb->devsuspendcallback(husb);
		}
	}
  
	if((isr & USB_ISTR_L1REQ) != 0){	/* LPM割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_L1REQ);
		if( husb->lpm_state == PCD_LPM_L0_ACTIVE){
			/*
			 *  LPMサスペンド設定
			 */
			sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_LPMODE);
			sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_CNTR), USB_CNTR_FSUSP);

			husb->lpm_state = PCD_LPM_L1_ACTIVE;
			husb->BESL = (sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_LPMCSR)) & USB_LPMCSR_BESL) >> 2;
			if(husb->devlpmcallback != NULL)
				husb->devlpmcallback(husb, PCD_LPM_L1_ACTIVE);
		}
		else{
			if(husb->devsuspendcallback != NULL)
				husb->devsuspendcallback(husb);
		}
	}

	if((isr & USB_ISTR_SOF) != 0){	/* SOF割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_SOF);
		if(husb->devsofcallback != NULL)
			husb->devsofcallback(husb);
	}

	if((isr & USB_ISTR_ESOF) != 0){	/* ESOF割込み */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_ISTR), USB_ISTR_ESOF);
	}
}


/*
 *  USBにデバイスアドレスを設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 addree デバイスアドレス(0-255)
 *  return     正常時E_OK
 */
ER
usbd_setDevAddress(USB_DEV_Handle_t *husb, uint8_t address)
{
	if(husb == NULL)
		return E_PAR;
	if(address == 0)	/* アドレスゼロならイネーブルに設定 */
		sil_wrh_mem((uint16_t *)(husb->base+TOFF_USB_DADDR), USB_DADDR_EF);
	else
		husb->usb_address = address;
	return E_OK;
}

/*
 *  アクティブエンドポイント設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_activateEndpoint(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	uint16_t type = USB_EP_CONTROL;
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);

	/*
	 * エンドポイント設定
	 */
	type  = ep_type_table[ep->type & 3];
	type |= USB_EP_CTR_RX | USB_EP_CTR_TX | ep->num;
	*pEndP = (*pEndP & (~USB_EP_T_FIELD & USB_EPREG_MASK)) | type;

	if(ep->doublebuffer == 0){	/* シングル・バッファ設定 */
		if(ep->is_in){	/* 送信バッファ・アドレス設定 */
			*GET_ENDPOINT_TX_ADDRESS(husb->base, ep->num) = ep->pmaadress & ~1;
			/*
			 * TX DTOGクリア
			 */
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;

			/*
			 *  送信NAK設定
			 */
			usbd_toggle_txdtog(pEndP, USB_EP_TX_NAK);
		}
		else{	/* 受信バッファ設定 */
			*GET_ENDPOINT_RX_ADDRESS(husb->base, ep->num) = ep->pmaadress & ~1;
			usbd_calc_blk(GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num), ep->maxpacket);

			/*
			 * RX DTOGクリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			/*
			 *  受信設定
			 */
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_VALID);
		}
	}
	else{	/* ダブル・バッファ設定 */
		*pEndP = ((*pEndP & USB_EPREG_MASK) | (USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_KIND));

		/*
		 * バッファ・アドレス設定
		 */
		*GET_ENDPOINT_TX_ADDRESS(husb->base, ep->num) = ep->pmaaddr0 & ~1;
		*GET_ENDPOINT_RX_ADDRESS(husb->base, ep->num) = ep->pmaaddr1 & ~1;

		if(ep->is_in){	/* INエンドポイント設定 */
			/*
			 *  IN/OUT用のデータトグル設定クリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;

			usbd_toggle_txdtog(pEndP, USB_EP_TX_DIS);
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_DIS);
		}
		else{	/* OUTエンドポイント設定 */
			/*
			 *  IN/OUT用のデータトグル設定クリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;

			usbd_toggle_rxdtog(pEndP, USB_EP_RX_VALID);
			usbd_toggle_txdtog(pEndP, USB_EP_TX_DIS);
		}
	}
	return E_OK;
}


/*
 *  ディスアクティブエンドポイント設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_deactivateEndpoint(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);
	if(ep->doublebuffer == 0){
		if(ep->is_in){
			/*
			 * TX DTOGクリア
			 */
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
			/*
			 * TXエンドポイントを無効化
			 */
			usbd_toggle_txdtog(pEndP, USB_EP_TX_DIS);
		}
		else{
			/*
			 * RX DTOGクリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			/*
			 * RXエンドポイントを無効化
			 */
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_DIS);
		}
	}
	else{	/* ダブル・バッファ設定 */
		if(ep->is_in){	/* IN endpoint */
			/*
			 *  IN/OUTデータトグルビットをクリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;

			/*
			 * エンドポイントを無効化
			 */
			usbd_toggle_txdtog(pEndP, USB_EP_TX_DIS);
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_DIS);
		}
		else{	/* OUT endpoint */
			/*
			 *  IN/OUTデータトグルビットをクリア
			 */
			if((*pEndP & USB_EP_DTOG_RX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
			if((*pEndP & USB_EP_DTOG_TX) != 0)
				*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;

			/*
			 * エンドポイントを無効化
			 */
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_DIS);
			usbd_toggle_txdtog(pEndP, USB_EP_TX_DIS);
		}
	}
	return E_OK;
}


/*
 *  受信開始設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_epstartreceive(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	uint32_t len;
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);
	/*
	 * マルチ・パケット処理
	 */
	if(ep->xfer_len > ep->maxpacket){
		len = ep->maxpacket;
		ep->xfer_len -= len;
	}
	else{
		len = ep->xfer_len;
		ep->xfer_len = 0;
	}

	if(ep->doublebuffer == 0)	/* シングル・バッファ受信カウント設定 */
		usbd_calc_blk(GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num), len);
	else{	/* ダブル・バッファ受信カウント設定 */
		if(ep->is_in){	/* IN endpoint */
			*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num) = (uint32_t)len;
			*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num) = (uint32_t)len;
		}
		else{	/* OUT endpoint */
			usbd_calc_blk(GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num), len);
			usbd_calc_blk(GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num), len);
		}

	}
	usbd_toggle_rxdtog(pEndP, USB_EP_RX_VALID);
	return E_OK;
}

/*
 *  送信開始設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_epstartsend(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	uint16_t pmabuffer = 0U;
	uint32_t len;
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);

	/*
	 * マルチ・パケット処理
	 */
	if(ep->xfer_len > ep->maxpacket){
		len = ep->maxpacket;
		ep->xfer_len -= len;
	}
	else{
		len = ep->xfer_len;
		ep->xfer_len = 0;
	}

	if(ep->doublebuffer == 0){	/* シングル・バッファデータ書込み */
		usbd_writePMA(husb->base, ep->xfer_buff, ep->pmaadress, len);
		*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num) = len;
	}
	else{	/* ダブル・バッファデータ書込み */
		if(ep->is_in){	/* IN endpoint */
			*GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num) = (uint32_t)len;
			*GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num) = (uint32_t)len;
		}
		else{	/* OUT endpoint */
			usbd_calc_blk(GET_ENDPOINT_TX_CNT_ADDRESS(husb->base, ep->num), len);
			usbd_calc_blk(GET_ENDPOINT_RX_CNT_ADDRESS(husb->base, ep->num), len);
		}

		/*
		 * データ設定
		 */
		if((*pEndP  & USB_EP_DTOG_TX) != 0)
			pmabuffer = ep->pmaaddr1;
		else
			pmabuffer = ep->pmaaddr0;

		usbd_writePMA(husb->base, ep->xfer_buff, pmabuffer, len);
		usbd_freeuserbuffer(pEndP, ep->is_in);
	}
	usbd_toggle_txdtog(pEndP, USB_EP_TX_VALID);
	return E_OK;
}

/*
 *  エンドポイントにステール状態を設定
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_epsetStall(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);
	if(ep->num == 0){
		usbd_toggle_rxdtog(pEndP, USB_EP_RX_STALL);
		usbd_toggle_txdtog(pEndP, USB_EP_TX_STALL);
	}
	else{
		if(ep->is_in)
			usbd_toggle_txdtog(pEndP, USB_EP_TX_STALL);
		else
			usbd_toggle_rxdtog(pEndP, USB_EP_RX_STALL);
	}
	return E_OK;
}

/*
 *  エンドポイントのステール状態をクリア
 *  parameter1 husb  USBハンドラへのポインタ
 *  parameter2 ep    エンドポイント構造体へのポインタ
 *  return     正常時E_OK
 */
ER
usbd_epclearStall(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep)
{
	volatile uint16_t *pEndP;

	if(husb == NULL || ep == NULL)
		return E_PAR;
	pEndP = (volatile uint16_t *)GET_ENDPOINT_ADDR(husb->base, ep->num);
	if(ep->is_in){
		/*
		 * TX DTOGクリア
		 */
		if((*pEndP & USB_EP_DTOG_TX) != 0)
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORT_VALUE;
		usbd_toggle_txdtog(pEndP, USB_EP_TX_VALID);
	}
	else{
		/*
		 * RX DTOGクリア
		 */
		if((*pEndP & USB_EP_DTOG_RX) != 0)
			*pEndP = (*pEndP & USB_EPREG_MASK) | USBD_DTORR_VALUE;
		usbd_toggle_rxdtog(pEndP, USB_EP_RX_VALID);
	}
	return E_OK;
}


/*
 *  PMAを設定する
 *  parameter1 husb      USBハンドラへのポインタ
 *  parameter2 ep_addr   EPアドレス
 *  parameter3 kind      EP種別(Signle/Double)
 *  parameter4 phmaddess PMA中のスタート位置
 *  return     常にE_OK
 */
ER
usbd_setupPMA(USB_DEV_Handle_t *husb, uint16_t ep_addr, uint16_t kind, uint32_t pmaadress)
{
	USB_DEV_EPTypeDef *ep;

	/* initialize ep structure*/
	if((ep_addr & 0x80) == 0x80)
		ep = &husb->IN_ep[ep_addr & 0x7F];
	else
		ep = &husb->OUT_ep[ep_addr];

	if(kind == PCD_SNG_BUF){	/* シングル・バッファ設定 */
		ep->doublebuffer = 0;
		ep->pmaadress = (uint16_t)pmaadress;
	}
	else{	/* ダブル・バッファ設定 */
		ep->doublebuffer = 1;
		ep->pmaaddr0 =  pmaadress & 0xFFFF;
		ep->pmaaddr1 =  pmaadress >> 16;
	}
	return E_OK;
}

/*
 *  LPM初期設定
 */
ER
usbd_init_lpm(USB_DEV_Handle_t *husb)
{
	if(husb->Init.lpm_enable == 1){
		husb->lpm_active = 1;
		husb->lpm_state = PCD_LPM_L0_ACTIVE;
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_LPMCSR), USB_LPMCSR_LMPEN);
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_LPMCSR), USB_LPMCSR_LPMACK);
	}
	return E_OK;
}

/*
 *  BCD初期設定
 */
ER
usbd_init_BCD(USB_DEV_Handle_t *husb)
{
	if(husb->Init.battery_charging_enable == 1){
		husb->battery_charging_active = 1;
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_BCDEN);
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_DCDEN);
	}
	return E_OK;
}

/*
 *  BCDテストタスク
 *  parameter1 exinf     USBID
 */
void
usbd_bcd_task(intptr_t exinf)
{
	USB_DEV_Handle_t *husb = &Husb[INDEX_USB((uint32_t)exinf)];
	uint32_t tick = 0;

 	/*
	 *  DETECT FLAGセット待ち
	 */
	while((sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR)) & USB_BCDR_DCDET) == 0){
		if(tick++  > 1000){
			if(husb->devbcdcallback != NULL)
				husb->devbcdcallback(husb, USBD_BCD_ERROR);
			return;
		}
		dly_tsk(1);
	}
	dly_tsk(300);

	/*
	 * DETECT FLAGリポート
	 */
	if((sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR)) & USB_BCDR_DCDET) != 0){
		if(husb->devbcdcallback != NULL)
			husb->devbcdcallback(husb, USBD_BCD_CONTACT_DETECTION);
	}
	/*
	 * 1ST: STANDARD DOWNSTREAM PORTチェック
	 */
	sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_DCDEN);
	sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_PDEN);
	dly_tsk(300);

	if((sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR)) & USB_BCDR_PDET) != 0){
		/*
		 * 2ND: CHARGING OR DETECTED CHARGINE PORTチェック
		 */
		sil_andh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_PDEN);
		sil_orh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR), USB_BCDR_SDEN);
		dly_tsk(300);

		if((sil_reh_mem((uint16_t *)(husb->base+TOFF_USB_BCDR)) & USB_BCDR_SDET) != 0){
			/* DOWNSTREAM PORT DCP */
			if(husb->devbcdcallback != NULL)
				husb->devbcdcallback(husb, USBD_BCD_DEDICATED_CHARGING_PORT);
		}
		else{	/* CHARGINE DOWNSTREAM PORT CDP */
			if(husb->devbcdcallback != NULL)
				husb->devbcdcallback(husb, USBD_BCD_CHARGING_DOWNSTREAM_PORT);
		}
	}
	else{	/* Standard DOWNSTREAM PORT */
		if(husb->devbcdcallback != NULL)
			husb->devbcdcallback(husb, USBD_BCD_STD_DOWNSTREAM_PORT);
	}
	/*
	 * BATTERY SENSE終了
	 */
	if(husb->devbcdcallback != NULL)
		husb->devbcdcallback(husb, USBD_BCD_DISCOVERY_COMPLETED);
}

