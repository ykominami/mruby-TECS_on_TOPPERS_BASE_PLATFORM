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
 *  @(#) $Id: usb_device.h 698 2017-11-07 22:49:05Z roi $
 */
/*
 * STM USB DEVICE用デバイスドライバの外部宣言
 */

#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  USBポート定義
 */
#define USB1_PORTID             1
#define NUM_USBPORT             1

/*
 *  エンドポイント・デバイスチャネルの最大数＋１
 */
#define MAX_DEVICE_EPS          8
#define MAX_EPS_CHANNELS        (MAX_DEVICE_EPS+1)

/*
 *  USB PHY定義
 */
#define USB_PHY_ULPI            1
#define USB_PHY_EMBEDDED        2

/*
 *  エンドポイントタイプ
 */
#define EP_TYPE_CTRL            0
#define EP_TYPE_ISOC            1
#define EP_TYPE_BULK            2
#define EP_TYPE_INTR            3
#define EP_TYPE_MSK             3

/*
 *  USBスピード
 */
#define USB_SPEED_HIGH          0
#define USB_SPEED_HIGH_IN_FULL  1
#define USB_SPEED_LOW           2
#define USB_SPEED_FULL          3

/*
 *  LPMアクティブパラメータ
 */
#define PCD_LPM_L0_ACTIVE       0x00		/* on */
#define PCD_LPM_L1_ACTIVE       0x01		/* LPM L1 sleep */
#define PCD_LPM_L2              0x02		/* suspend */
#define PCD_LPM_L3              0x03		/* off */

/*
 *  エンドポイント種別
 */
#define PCD_SNG_BUF             0U
#define PCD_DBL_BUF             1U

/*
 *  BCDイベント定義
 */
#define USBD_BCD_CONTACT_DETECTION          0
#define USBD_BCD_STD_DOWNSTREAM_PORT        2
#define USBD_BCD_CHARGING_DOWNSTREAM_PORT   3
#define USBD_BCD_DEDICATED_CHARGING_PORT    4
#define USBD_BCD_DISCOVERY_COMPLETED        1
#define USBD_BCD_ERROR                      -1


/*
 *  USBデバイス初期設定定義
 */
typedef struct
{
	uint32_t              dev_endpoints;			/* デバイスエンドポイント数(1-8) */
	uint32_t              speed;					/* USBコアスピード */
	uint32_t              phy_itface;				/* PHYインターフェイス選択 */
	uint32_t              sof_enable;				/* SOF割込みの有効無効設定(0 or 1) */
    uint32_t              low_power_enable;			/* LOW POWERモード有効無効設定(0 or 1) */
    uint32_t              lpm_enable;				/* Link Power Management有効無効設定(0 or 1) */
	uint32_t              battery_charging_enable;	/* バッテリチャージ有効無効設定(0 or 1) */ 
}USB_DEV_Init_t;

/*
 *  エンドポイント定義
 */
typedef struct
{
    uint8_t               num;					/* エンドポイント番号(0-7) */
	uint8_t               is_in : 1;			/* エンドポイントスチール状態(0 or 1) */
	uint8_t               is_stall : 1;			/* エンドポイントスチール状態(0 or 1) */
	uint8_t               type : 2;				/* エンドポイントタイプ */
	uint8_t               doublebuffer : 1;		/* エンドポイント：ダブルバッファ指定 */
	uint16_t              pmaadress;			/* PMA Address */
	uint16_t              pmaaddr0;				/* PMA Address0 */
	uint16_t              pmaaddr1;				/* PMA Address1 */
    uint16_t              maxpacket;			/* エンドポイント最大パケットサイズ(0-64KB) */
	uint8_t               *xfer_buff;			/* 転送バッファポインター */
	uint32_t              xfer_len;				/* 現在の転送数 */
	uint32_t              xfer_count;			/* 指定転送カウント */
}USB_DEV_EPTypeDef;

typedef struct _USB_DEV_Handle USB_DEV_Handle_t;

/*
 *   USBデバイスハンドラ定義
 */
struct _USB_DEV_Handle
{
	uint32_t              base;						/* USB DEVICE レジスタベースアドレス */
	USB_DEV_Init_t        Init;						/* USB DEVICE 初期設定 */
	USB_DEV_EPTypeDef     IN_ep[MAX_DEVICE_EPS];	/* IN エンドポイントデータ */
	USB_DEV_EPTypeDef     OUT_ep[MAX_DEVICE_EPS];	/* OUT エンドポイントデータ */
	volatile uint32_t     Setup[12];				/* Setup パケットバッファ */
	uint32_t              BESL;						/* BESL保存領域 */
	uint8_t               lpm_state;				/* LPM 状態 */
	uint8_t               lpm_active;				/* Link Power Management有効無効設定(0 or 1) */
	uint8_t               battery_charging_active;	/* Enable or disable Battery charging */
	volatile uint8_t      usb_address;				/* USB Address */
	void                  (*devsetupstagecallback)(USB_DEV_Handle_t * hhcd);
	void                  (*devdataoutstagecallback)(USB_DEV_Handle_t * hhcd, uint8_t epnum);
	void                  (*devdatainstagecallback)(USB_DEV_Handle_t * hhcd, uint8_t epnum);
	void                  (*devsofcallback)(USB_DEV_Handle_t * hhcd);
	void                  (*devresetcallback)(USB_DEV_Handle_t * hhcd);
	void                  (*devsuspendcallback)(USB_DEV_Handle_t * hhcd);
	void                  (*devresumecallback)(USB_DEV_Handle_t * hhcd);
	void                  (*devconnectcallback)(USB_DEV_Handle_t * hhcd);					/* */
	void                  (*devdisconnectcallback)(USB_DEV_Handle_t * hhcd);				/* */
	void                  (*devlpmcallback)(USB_DEV_Handle_t * hhcd, uint8_t msg);
	void                  (*devbcdcallback)(USB_DEV_Handle_t * hhcd, int8_t msg);
	void                  *pDev;					/* Pointer Device Stack Handler */
};

extern ER usbd_setupPMA(USB_DEV_Handle_t *husb, uint16_t ep_addr, uint16_t kind, uint32_t pmaadress);
extern ER usbd_init_lpm(USB_DEV_Handle_t *husb);
extern ER usbd_init_BCD(USB_DEV_Handle_t *husb);
extern void usbd_bcd_task(intptr_t exinf);

extern USB_DEV_Handle_t *usbd_init(int id, USB_DEV_Init_t *pini);
extern ER usbd_deinit (USB_DEV_Handle_t *husb);
extern ER usbd_devconnect(USB_DEV_Handle_t *husb);
extern ER usbd_devdisconnect(USB_DEV_Handle_t *husb);
extern ER usbd_setDevAddress(USB_DEV_Handle_t *husb, uint8_t address);
extern ER usbd_activateEndpoint(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern ER usbd_deactivateEndpoint(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern ER usbd_epstartreceive(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern ER usbd_epstartsend(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern ER usbd_epsetStall(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern ER usbd_epclearStall(USB_DEV_Handle_t *husb, USB_DEV_EPTypeDef *ep);
extern void usb_device_isr(intptr_t exinf);


#ifdef __cplusplus
}
#endif

#endif

