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
 *  @(#) $Id: ltdc.h 698 2017-02-24 18:25:32Z roi $
 */
/*
 * STM32F746 LTDC用デバイスドライバの外部宣言
 */

#ifndef _LTDC_H_
#define _LTDC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define MAX_LAYER  2

/*
 *  LTDC_HS_POLARITY LTDC HS POLARITY
 */
#define LTDC_HSPOLARITY_AL       0x00000000		/* Horizontal Synchronization is active low. */
#define LTDC_HSPOLARITY_AH       LTDC_GCR_HSPOL	/* Horizontal Synchronization is active high. */

/*
 *  LTDC_VS_POLARITY LTDC VS POLARITY
 */
#define LTDC_VSPOLARITY_AL       0x00000000		/* Vertical Synchronization is active low. */
#define LTDC_VSPOLARITY_AH       LTDC_GCR_VSPOL	/* Vertical Synchronization is active high. */

/*
 *  LTDC_DE_POLARITY LTDC DE POLARITY
 */
#define LTDC_DEPOLARITY_AL       0x00000000		/* Data Enable, is active low. */
#define LTDC_DEPOLARITY_AH       LTDC_GCR_DEPOL	/* Data Enable, is active high. */

/*
 *  LTDC_BlendingFactor1 LTDC Blending Factor1
 */
#define LTDC_BLENDING_FACTOR1_CA    0x00000400	/* Blending factor : Cte Alpha */
#define LTDC_BLENDING_FACTOR1_PAxCA 0x00000600	/* Blending factor : Cte Alpha x Pixel Alpha*/

/*
 *  LTDC_BlendingFactor2 LTDC Blending Factor2
 */
#define LTDC_BLENDING_FACTOR2_CA    0x00000005	/* Blending factor : Cte Alpha */
#define LTDC_BLENDING_FACTOR2_PAxCA 0x00000007	/* Blending factor : Cte Alpha x Pixel Alpha*/

/*
 *  LTDC_PC_POLARITY LTDC PC POLARITY
 */
#define LTDC_PCPOLARITY_IPC      0x00000000		/* input pixel clock. */
#define LTDC_PCPOLARITY_IIPC     LTDC_GCR_PCPOL	/* inverted input pixel clock. */

/*
 *  LTDCピクセルフォーマット定義
 */
#define LTDC_PIXEL_FORMAT_ARGB8888  0x00000000	/* ARGB8888 LTDC pixel format */
#define LTDC_PIXEL_FORMAT_RGB888    0x00000001	/* RGB888 LTDC pixel format   */
#define LTDC_PIXEL_FORMAT_RGB565    0x00000002	/* RGB565 LTDC pixel format   */
#define LTDC_PIXEL_FORMAT_ARGB1555  0x00000003	/* ARGB1555 LTDC pixel format */
#define LTDC_PIXEL_FORMAT_ARGB4444  0x00000004	/* ARGB4444 LTDC pixel format */
#define LTDC_PIXEL_FORMAT_L8        0x00000005	/* L8 LTDC pixel format       */
#define LTDC_PIXEL_FORMAT_AL44      0x00000006	/* AL44 LTDC pixel format     */
#define LTDC_PIXEL_FORMAT_AL88      0x00000007	/* AL88 LTDC pixel format     */

/*
 *  LTDC状態定義
 */
#define LTDC_STATE_RESET         0x00000000		/* LTDC リセット状態 */
#define LTDC_STATE_READY         0x00000001		/* LTDC レディ状態 */
#define LTDC_STATE_BUSY          0x00000002		/* LTDC ビジィ状態 */
#define LTDC_STATE_ERROR         0x00010000		/* LTDC エラー状態 */

/*
 *  LTDCエラー定義
 */
#define LTDC_ERROR_NONE          0x00000000		/* LTDC エラーなし */
#define LTDC_ERROR_TE            0x00000001		/* LTDC 転送エラー */
#define LTDC_ERROR_FU            0x00000002		/* LTDC FIFOアンダーラン */


/*
 *  LTDCカラー構造体定義
 */
typedef struct
{
	uint8_t	    Blue;			/* Configures the blue value. */
	uint8_t     Green;			/* Configures the green value. */
	uint8_t     Red;			/* Configures the red value. */
	uint8_t     Reserved;		/* Reserved 0xFF */
} Color_t;

/*
 *  LTDC初期化構造体定義
 */
typedef struct
{
	uint32_t    HSPolarity;		/* configures the horizontal synchronization polarity. */
	uint32_t    VSPolarity;		/* configures the vertical synchronization polarity. */
	uint32_t    DEPolarity;		/* configures the data enable polarity. */
	uint32_t    PCPolarity;		/* configures the pixel clock polarity. */
	uint32_t    HorizontalSync;	/* configures the number of Horizontal synchronization width. */
	uint32_t    VerticalSync;	/* configures the number of Vertical synchronization height. */
	uint32_t    AccumulatedHBP;	/* configures the accumulated horizontal back porch width. */
	uint32_t    AccumulatedVBP;	/* configures the accumulated vertical back porch height. */
	uint32_t    AccumulatedActiveW;	/* configures the accumulated active width. */
	uint32_t    AccumulatedActiveH;	/* configures the accumulated active height. */
	uint32_t    TotalWidth;		/* configures the total width. */
	uint32_t    TotalHeigh;		/* configures the total height. */
	Color_t     Backcolor;		/* configures the background color. */
	uint32_t    pllsain;		/* configures the PLLSAIN */
	uint32_t    pllsair;		/* configures the PLLSAIR */
	uint32_t    saidivr;		/* configures the PLLSAIDIVR */
} LTDC_Init_t;

/*
 *  LTDCレイヤー構造体定義
 */
typedef struct
{
	uint32_t    WindowX0;		/* Configures the Window Horizontal Start Position. */
	uint32_t    WindowX1;		/* Configures the Window Horizontal Stop Position. */
	uint32_t    WindowY0;		/* Configures the Window vertical Start Position. */
	uint32_t    WindowY1;		/* Configures the Window vertical Stop Position. */
	uint32_t    PixelFormat;	/* Specifies the pixel format. */
	uint32_t    Alpha;			/* Specifies the constant alpha used for blending. */
	uint32_t    Alpha0;			/* Configures the default alpha value. */
	uint32_t    BlendingFactor1;/* Select the blending factor 1. */
	uint32_t    BlendingFactor2;/* Select the blending factor 2. */
	uint32_t    FBStartAdress;	/* Configures the color frame buffer address */
	uint32_t    ImageWidth;		/* Configures the color frame buffer line length. */
	uint32_t    ImageHeight;	/* Specifies the number of line in frame buffer. */
	Color_t     Backcolor;		/* Configures the layer background color. */
} LTDC_LayerCfg_t;

/*
 *  LTDCハンドラ構造体定義
 */
typedef struct _LTDC_Handle_t LTDC_Handle_t;
struct _LTDC_Handle_t {
	uint32_t           base;				/* LTDC ベースレジスタ */
	LTDC_Init_t        Init;				/* LTDC parameters */
	LTDC_LayerCfg_t LayerCfg[MAX_LAYER];	/* LTDC Layers parameters */
	void               (*linecallback)(LTDC_Handle_t * hltdc);	/* ラインコールバック関数 */
	void               (*reloadcallback)(LTDC_Handle_t * hltdc);/* リロードコールバック関数 */
	void               (*errorcallback)(LTDC_Handle_t * hltdc);	/* エラーコールバック関数 */
	volatile uint32_t  state;				/* LTDC 実行状態 */
	volatile uint32_t  errorcode;			/* LTDC エラーコード */
};

extern ER ltdc_init(LTDC_Handle_t *phandle);
extern ER ltdc_configlayer(LTDC_Handle_t *hltdc, LTDC_LayerCfg_t *pLayerCfg, uint32_t LayerIdx);
extern ER ltdc_setwindowsize(LTDC_Handle_t *hltdc, uint32_t XSize, uint32_t YSize, uint32_t LayerIdx);
extern ER ltdc_setwindowposition(LTDC_Handle_t *hltdc, uint32_t X0, uint32_t Y0, uint32_t LayerIdx);
extern ER ltdc_setalpha(LTDC_Handle_t *hltdc, uint32_t Alpha, uint32_t LayerIdx);
extern ER ltdc_setaddress(LTDC_Handle_t *hltdc, uint32_t Address, uint32_t LayerIdx);
extern ER ltdc_configcolorkeying(LTDC_Handle_t *hltdc, uint32_t RGBValue, uint32_t LayerIdx);
extern ER ltdc_enablecolorkeying(LTDC_Handle_t *hltdc, uint32_t LayerIdx);
extern ER ltdc_disablecolorkeying(LTDC_Handle_t *hltdc, uint32_t LayerIdx);
extern ER ltdc_lineevent(LTDC_Handle_t *phandle, uint32_t Line);
extern ER ltdc_reload(LTDC_Handle_t *phandle, uint32_t ReloadType);
extern void ltdc_irqhandler(LTDC_Handle_t *phandle);

#ifdef __cplusplus
}
#endif

#endif

