/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
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
 *  $Id: usb_otg.h 2416 2016-07-12 18:11:28Z roi $
 */

/*
 *	USB OTGデバイスドライバのヘッダファイル
 */

#ifndef _USB_OTG_H_
#define _USB_OTG_H_


/*
 *  USBポート定義
 */
#define USB1_PORTID             1
#define USB2_PORTID             2
#define NUM_USBPORT             2

/*
 *  USB OTGモード
 */
#define USB_OTG_MODE_DEVICE     0
#define USB_OTG_MODE_HOST       1
#define USB_OTG_MODE_DRD        2

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


#define HC_PID_DATA0            0
#define HC_PID_DATA2            1
#define HC_PID_DATA1            2
#define HC_PID_SETUP            3

/*
 *  URB状態定義
 */ 
#define URB_IDLE                0
#define URB_DONE                1
#define URB_NOTREADY            2
#define URB_NYET                3
#define URB_ERROR               4
#define URB_STALL               5

/*
 *  HOSTチャンネル状態定義
 */
#define USBO_HC_IDLE            0
#define USBO_HC_XFRC            1
#define USBO_HC_HALTED          2
#define USBO_HC_NAK             3
#define USBO_HC_NYET            4
#define USBO_HC_STALL           5
#define USBO_HC_XACTERR         6
#define USBO_HC_BBLERR          7
#define USBO_HC_DATATGLERR      8
#define USBO_HC_FAIL            16

/*
 *  LPMアクティブパラメータ
 */
#define PCD_LPM_L0_ACTIVE       0x00		/* on */
#define PCD_LPM_L1_ACTIVE       0x01		/* LPM L1 sleep */
#define PCD_LPM_L2              0x02		/* suspend */
#define PCD_LPM_L3              0x03		/* off */

/*
 *  エンドポイント・ホストチャネルの最大数
 */
#define MAX_EPS_CHANNELS        16

/*
 *  デバイスステート
 */
#define DWC2_L0                 0	/* On state */
#define DWC2_L1                 1	/* LPM sleep state */
#define DWC2_L2                 2	/* USB suspend state */
#define DWC2_L3                 3	/* Off state */


/*
 *  USB OTGハードウェア情報
 */
typedef struct
{
	unsigned en_multiple_tx_fifo:1;
	unsigned host_rx_fifo_size:16;
	unsigned max_transfer_size:26;
	unsigned max_packet_count:11;
	unsigned total_fifo_size:16;
}USB_OTG_HwParam_t;

/*
 *  USB OTG初期設定定義
 */
typedef struct
{
    uint32_t                usb_otg_mode;			/* USB OTGモード */
    uint32_t                dev_endpoints;			/* デバイスエンドポイント数(1-15) */
    uint32_t                host_channels;			/* ホストチャネル数(1-15) */
    uint32_t                speed;					/* USBコアスピード */
    uint32_t                dma_enable;				/* USB-DMA機能の有効無効設定(0 or 1) */
    uint32_t                phy_itface;				/* PHYインターフェイス選択 */
    uint32_t                sof_enable;				/* SOF割込みの有効無効設定(0 or 1) */
    uint32_t                low_power_enable;		/* LOW POWERモード有効無効設定(0 or 1) */
    uint32_t                lpm_enable;				/* Link Power Management有効無効設定(0 or 1) */
    uint32_t                vbus_sensing_enable;	/* VBUSセンス有効無効設定(0 or 1) */
    uint32_t                use_dedicated_ep1;		/* エンドポイント1専用割込み有効無効設定(0 or 1) */
    uint32_t                use_external_vbus;		/* 外部VBUS有効無効設定(0 or 1) */
}USB_OTG_Init_t;

/*
 *  エンドポイント定義
 */
typedef struct
{
    uint8_t                 num;					/* エンドポイント番号(0-14) */
    uint8_t                 is_in : 1;				/* エンドポイントOUT/IN(0 or 1) */
    uint8_t                 is_stall : 1;			/* エンドポイントスチール状態(0 or 1) */
    uint8_t                 type : 2;				/* エンドポイントタイプ */
    uint8_t                 data_pid_start : 1;		/* 初期データPID(0 or 1) */
    uint8_t                 even_odd_frame : 1;		/* フレームパリティ(0 or 1) */
    uint16_t                tx_fifo_num;			/* 転送FIFO番号(1-15) */
    uint16_t                maxpacket;				/* エンドポイント最大パケットサイズ(0-64KB) */
    uint8_t                 *xfer_buff;				/* 転送バッファポインター */
    uint32_t                dma_addr;				/* DMA転送アドレス(32bitアライン) */
    uint32_t                xfer_len;				/* 現在の転送数 */
    uint32_t                xfer_count;				/* 指定転送カウント */
}USB_OTG_EPTypeDef;

/*
 *  ホストクラス定義
 */
typedef struct
{
    uint8_t                 dev_addr;				/* USBデバイスアドレス(1-255) */
    uint8_t                 ch_num;					/* Hostチャネル番号(1-15) */
    uint8_t                 ep_num;					/* エンドポイント番号(1-15) */
	uint8_t                 ep_is_in;				/* エンドポイントのディレクション(0 or 1) */
	uint8_t                 speed;					/* USB Hostスピード */
	uint8_t                 do_ping;				/* HSモードのPINGプロトコルの有効無効設定(0 or 1) */
	uint8_t                 process_ping;			/* PINGプロトコル実行中フラグ(1で実行中) */
	uint8_t                 ep_type;				/* エンドポイントタイプ(0-3) */
	uint16_t                max_packet;				/* 最大パケットサイズ(64KB以内) */
	uint8_t                 data_pid;				/* 初期設定データPID. */
	uint8_t                 *xfer_buff;				/* 転送バッファへのポインタ */
	uint32_t                xfer_len;				/* 現在の転送数 */
	uint32_t                xfer_count;				/* 指定転送サイズ */
	uint8_t                 toggle_in;				/* IN transfer current toggle flag. */
	uint8_t                 toggle_out;				/* OUT transfer current toggle flag */
	uint32_t                dma_addr;				/* DMA転送アドレス(4バイトアライン) */
	uint32_t                err_count;				/* Hostチャネルエラーカウンター */
	volatile uint8_t        urb_state;				/* URBステート */
	volatile uint8_t        state;					/* Hostチャネルステート */
}USB_OTG_HCTypeDef;


#ifndef USBD_HS_TRDT_VALUE
 #define USBD_HS_TRDT_VALUE           9U
#endif /* USBD_HS_TRDT_VALUE */


#define USBOTG_BASE(a)    ((uint32_t)(a))
#define USBPGC_BASE(a)    ((uint32_t)(a)+USB_OTG_PCGCCTL_BASE)
#define USBPRT_BASE(a)    ((uint32_t)(a)+USB_OTG_HOST_PORT_BASE)
#define USBD_BASE(a)      ((uint32_t)(a)+USB_OTG_DEVICE_BASE)
#define USBIEP_BASE(a, i) ((uint32_t)(a)+USB_OTG_IN_ENDPOINT_BASE+((i)*USBO_EP_REG_SIZE))
#define USBOEP_BASE(a, i) ((uint32_t)(a)+USB_OTG_OUT_ENDPOINT_BASE+((i)*USBO_EP_REG_SIZE))
#define USBD_FIFO(a, i)   ((uint32_t)(a)+USB_OTG_FIFO_BASE+((i)*USB_OTG_FIFO_SIZE))
#define USBH_BASE(a)      ((uint32_t)(a)+USB_OTG_HOST_BASE)
#define USBHC_BASE(a, i)  ((uint32_t)(a)+USB_OTG_HOST_CHANNEL_BASE+((i)*USBO_HOST_CHANNEL_REG_SIZE))


#ifndef TOPPERS_MACRO_ONLY


/*
 *   USB_OTGデバイスハンドラ定義
 */
typedef struct _USB_OTG_Handle_t USB_OTG_Handle_t;
struct _USB_OTG_Handle_t
{
	uint32_t              base;			/* USB OTG レジスタベースアドレス */
	USB_OTG_Init_t        Init;			/* USB OTG 初期設定 */
	USB_OTG_HCTypeDef     hc[MAX_EPS_CHANNELS];			/* HOST チャネルデータ */
	USB_OTG_EPTypeDef     IN_ep[MAX_EPS_CHANNELS-1];	/* IN エンドポイントデータ */
	USB_OTG_EPTypeDef     OUT_ep[MAX_EPS_CHANNELS-1];	/* OUT エンドポイントデータ */
	volatile uint32_t     Setup[12];					/* Setup パケットバッファ */
	uint8_t               lx_state;						/* lx state */
	uint8_t               enabled;						/* device enabled */
	uint8_t               connected;					/* device 接続中 */
	uint8_t               dedicated_fifos;				/* device dedicated fifo mode */
	uint32_t              resetbit;						/* usb reset bit */
	uint32_t              fifo_mem;						/* fifo memoryサイズ */
	uint32_t              pcd_rx_fifo_sz;				/* pcd rx fifoサイズ */
	uint32_t              pcd_np_g_tx_fifo_sz;			/* pcd g tx fifoサイズ */
	uint32_t              pcd_tx_fifo_sz[MAX_EPS_CHANNELS];	/* pcd tx fifoテーブル */
#ifdef USB_OTG_GLPMCFG_LPMEN
	volatile uint32_t     BESL;			/* BESL保存領域 */
	volatile uint8_t      lpm_state;	/* LPM 状態 */
	uint8_t               lpm_active;	/* Link Power Management有効無効設定(0 or 1) */
#endif /* USB_OTG_GLPMCFG_LPMEN */
	USB_OTG_HwParam_t     hw_params;					/* ハードウェア設定値 */
	void                  (*hostsofcallback)(USB_OTG_Handle_t * hhcd);		/* sofコールバック関数 */
 	void                  (*hostconnectcallback)(USB_OTG_Handle_t * hhcd);	/* connectコールバック関数 */
 	void                  (*hostdisconnectcallback)(USB_OTG_Handle_t * hhcd);	/* disconnectコールバック関数 */
 	void                  (*hostchangeurbcallback)(USB_OTG_Handle_t * hhcd, uint8_t num, uint8_t state);	/* changeurbコールバック関数 */
	void                  (*devsetupstagecallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devdataoutstagecallback)(USB_OTG_Handle_t * hhcd, uint8_t epnum);
	void                  (*devdatainstagecallback)(USB_OTG_Handle_t * hhcd, uint8_t epnum);
	void                  (*devsofcallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devresetcallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devsuspendcallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devresumecallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devisooutcallback)(USB_OTG_Handle_t * hhcd, uint8_t epnum);
	void                  (*devisoincallback)(USB_OTG_Handle_t * hhcd, uint8_t epnum);
	void                  (*devconnectcallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devdisconnectcallback)(USB_OTG_Handle_t * hhcd);
	void                  (*devlpmcallback)(USB_OTG_Handle_t * hhcd, uint8_t msg);
	void                  *pHost;		/* Pointer Host Stack Handler    */
	void                  *pDev;		/* Pointer Device Stack Handler */
};

extern USB_OTG_Handle_t *usbo_init(ID portid, USB_OTG_Init_t *pini);
extern ER usbo_deinit(USB_OTG_Handle_t *husb);
extern ER usbo_devinit(USB_OTG_Handle_t *husb);
extern ER usbo_coreinit(USB_OTG_Handle_t *husb);
extern ER usbo_setupint(USB_OTG_Handle_t *husb);
extern ER usbo_enableglobalint(USB_OTG_Handle_t *husb);
extern ER usbo_disableglobalint(USB_OTG_Handle_t *husb);
extern ER usbo_setcurrentmode(USB_OTG_Handle_t *husb);
extern ER usbo_flushRxFifo(USB_OTG_Handle_t *husb);
extern ER usbo_flushTxFifo(USB_OTG_Handle_t *husb, uint32_t num);
extern ER usbo_initFiFo(USB_OTG_Handle_t *husb);
extern ER usbo_activateEndpoint(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_deactivateEndpoint(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_epstartxfer(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_ep0startxfer(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_epsetStall(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_epclearStall(USB_OTG_Handle_t *husb, USB_OTG_EPTypeDef *ep);
extern ER usbo_setDevAddress(USB_OTG_Handle_t *husb, uint8_t address);
extern ER usbo_devconnect(USB_OTG_Handle_t *husb);
extern ER usbo_devdisconnect(USB_OTG_Handle_t *husb);
extern ER usbo_stopdevice(USB_OTG_Handle_t *husb);
extern ER usbo_ep0_outstart(USB_OTG_Handle_t *husb, uint8_t *psetup);
extern uint8_t usbo_getDevSpeed(USB_OTG_Handle_t *husb);
extern ER usbo_init_lpm(USB_OTG_Handle_t *husb);

extern ER usbo_hostinit(USB_OTG_Handle_t *husb);
extern ER usbo_starthost(USB_OTG_Handle_t *husb);
extern ER usbo_resetport(USB_OTG_Handle_t *husb);
extern ER usbo_drivevbus(USB_OTG_Handle_t *husb, uint8_t state);
extern void usbo_driveextvbus(USB_OTG_Handle_t *husb, uint8_t state);
extern uint32_t usbo_gethostspeed(USB_OTG_Handle_t *husb);
extern uint32_t usbo_getcurrentframe(USB_OTG_Handle_t *husb);
extern ER usbo_hc_init(USB_OTG_Handle_t *husb, uint8_t ch_num, uint8_t epnum);
extern ER usbo_hc_startxfer(USB_OTG_Handle_t *husb, uint8_t ch_num);
extern ER usbo_hc_halt(USB_OTG_Handle_t *husb, uint8_t ch_num);
extern ER usbo_stophost(USB_OTG_Handle_t *husb);

extern void usbo_hcd_irqhandler(USB_OTG_Handle_t *husb);
extern void usbo_pcd_irqhandler(USB_OTG_Handle_t *husb);

extern void usb_otg_isr(intptr_t exinf);
extern void usb_otg_wkup_isr(intptr_t exinf);

extern void	usbh_task(intptr_t exinf);

#endif /* TOPPERS_MACRO_ONLY */

#endif	/* _USB_OTG_H_ */
