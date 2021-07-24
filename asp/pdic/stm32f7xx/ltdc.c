/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: ltdc.c 698 2017-02-24 18:26:43Z roi $
 */
/*
 * STM32F746-LTDC用デバイスドライバ
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "kernel_cfg.h"
#include "device.h"
#include "ltdc.h"


/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  AF 9セクション定義
 */
#define GPIO_AF9_CAN1           0x09	/* CAN1 Alternate Function mapping    */
#define GPIO_AF9_CAN2           0x09	/* CAN2 Alternate Function mapping    */
#define GPIO_AF9_TIM12          0x09	/* TIM12 Alternate Function mapping   */
#define GPIO_AF9_TIM13          0x09	/* TIM13 Alternate Function mapping   */
#define GPIO_AF9_TIM14          0x09	/* TIM14 Alternate Function mapping   */
#define GPIO_AF9_QUADSPI        0x09	/* QUADSPI Alternate Function mapping */
#define GPIO_AF9_LTDC           0x09	/* LCD-TFT Alternate Function mapping */

/*
 *  AF 14セクション定義
 */
#define GPIO_AF14_LTDC          0x0E	/* LCD-TFT alternate functionマップ値 */

#define PLLSAI_TIMEOUT_VALUE   (100*1000)

#define RCC_PLLSAICFGR_PLLSAIQ_POS    24
#define RCC_PLLSAICFGR_PLLSAIP_POS    16

#ifdef TOPPERS_STM32F7_DISCOVERY
static const GPIO_Init_Table ltdc_gpio_table[] = {
	{TADR_GPIOG_BASE, (GPIO_PIN_12) },
	{TADR_GPIOE_BASE, (GPIO_PIN_4)  },
	{TADR_GPIOI_BASE, (GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 ) },
	{TADR_GPIOJ_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | \
			  GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
			  GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOK_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7) }
};

#define NUM_LTDC_GPIO_ITEM (sizeof(ltdc_gpio_table)/sizeof(GPIO_Init_Table))

/*
 *  LTDC GPIOの初期化
 */
static void
ltdc_gpio_init(void)
{
	GPIO_Init_t GPIO_Init_Data;
	int i, pin;
	volatile uint32_t tmp;

	/* LTDCクロック許可 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_LTDCEN);
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_APB2ENR));

	/* LTDC用GPIOのクロック設定 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),
			  RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOGEN
			| RCC_AHB1ENR_GPIOIEN | RCC_AHB1ENR_GPIOJEN
			| RCC_AHB1ENR_GPIOKEN);
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));
	(void)(tmp);

	/* LTDC用GPIOの初期化パラメータ設定 */
	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	GPIO_Init_Data.speed     = GPIO_SPEED_FAST;
	GPIO_Init_Data.alternate = GPIO_AF9_LTDC;

	for(i = 0 ; i < NUM_LTDC_GPIO_ITEM ; i++){
		for(pin = 0 ; pin < 16 ; pin++){
			if((ltdc_gpio_table[i].pinmap & 1<<pin) != 0)
				gpio_setup(ltdc_gpio_table[i].base, &GPIO_Init_Data, pin);
		}
		GPIO_Init_Data.alternate = GPIO_AF14_LTDC;
	}

	/* LCD表示用GPIOの設定 */
	GPIO_Init_Data.mode      = GPIO_MODE_OUTPUT;
	gpio_setup(TADR_GPIOI_BASE, &GPIO_Init_Data, PINPOSITION12);
	gpio_setup(TADR_GPIOK_BASE, &GPIO_Init_Data, PINPOSITION3);

	/* 表示許可LCD_DISPピンをアサート */
	sil_wrw_mem((uint32_t *)(TADR_GPIOI_BASE+TOFF_GPIO_BSRR), GPIO_PIN_12);

	/* バックライトLCD_BL_CTRLピンをアサート */
	sil_wrw_mem((uint32_t *)(TADR_GPIOK_BASE+TOFF_GPIO_BSRR), GPIO_PIN_3);
}
#endif

/*
 *  LTDCのレイヤーのハードウェア初期設定する
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 pLayerCfg レイヤーの初期設定構造体のポインタ
 *  parameter3 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
static void
LTDC_SetConfig(LTDC_Handle_t *phandle, LTDC_LayerCfg_t *pLayerCfg, uint32_t LayerIdx)
{
	uint32_t layer = TADR_LTDC_LAYER1+LayerIdx*LTDC_WINDOW_SIZE;
	uint32_t bpcr  = sil_rew_mem((uint32_t *)(phandle->base+TOFF_LTDC_BPCR));
	uint32_t tmp = 0;

	/* 水平方向スタートストップ位置設定 */
	tmp  = (pLayerCfg->WindowX1 + ((bpcr & LTDC_BPCR_AHBP) >> 16)) << 16;
	tmp |= (pLayerCfg->WindowX0 + ((bpcr & LTDC_BPCR_AHBP) >> 16) + 1);
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_WHPCR), tmp);

	/* 垂直方向スタートストップ位置設定 */
	tmp  = (pLayerCfg->WindowY1 + (bpcr & LTDC_BPCR_AVBP)) << 16;
	tmp |= (pLayerCfg->WindowY0 + (bpcr & LTDC_BPCR_AVBP) + 1);
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_WVPCR), tmp);

	/* ピクセルフォーマットの指定 */
	sil_modw_mem((uint32_t *)(layer+TOFF_LTDCW_PFCR), LTDC_PFCR_PF, pLayerCfg->PixelFormat);

	/* デフォルトのカラー値を設定 */
	tmp  = pLayerCfg->Backcolor.Blue;
	tmp |= pLayerCfg->Backcolor.Green << 8;
	tmp |= pLayerCfg->Backcolor.Red << 16;
	tmp |= pLayerCfg->Alpha0 << 24;
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_DCCR), tmp);

	/* アルファ値を設定 */
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_CACR), pLayerCfg->Alpha);

	/* ブランディングファクターを設定 */
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_BFCR), (pLayerCfg->BlendingFactor1 | pLayerCfg->BlendingFactor2));

	/* フレームバッファアドレスを設定 */
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_CFBAR), pLayerCfg->FBStartAdress);

	/* バイト中のカラーフレームピッチの設定 */
	if(pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
		tmp = 4;
	else if (pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_RGB888)
		tmp = 3;
	else if((pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) ||
		(pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_RGB565)   ||
		  (pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_ARGB1555) ||
		    (pLayerCfg->PixelFormat == LTDC_PIXEL_FORMAT_AL88))
		tmp = 2;
	else
		tmp = 1;
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_CFBLR), (((pLayerCfg->ImageWidth * tmp) << 16) | (((pLayerCfg->WindowX1 - pLayerCfg->WindowX0) * tmp)  + 3)));

	/* フレームバッファライン数を設定 */
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_CFBLNR), pLayerCfg->ImageHeight);

	/* LTDCレイヤーを許可 */
	sil_orw_mem((uint32_t *)(layer+TOFF_LTDCW_CR), LTDC_CR_LEN);
}

/*
 *  LTDCのウインドウのアルファ値を設定する
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 Alpha     ウインドウアルファ値
 *  parameter3 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_setalpha(LTDC_Handle_t *phandle, uint32_t Alpha, uint32_t LayerIdx)
{
	LTDC_LayerCfg_t *pLayerCfg;

	if(LayerIdx >= MAX_LAYER)
		return E_PAR;
	if(Alpha > 511)
		return E_PAR;

	/* レイヤーコンフィギュレーション領域のポインタ取得 */
	pLayerCfg = &phandle->LayerCfg[LayerIdx];
	/* ウインドウアルファ値設定 */
	pLayerCfg->Alpha = Alpha;
	/* LTDCレイヤー設定 */
	LTDC_SetConfig(phandle, pLayerCfg, LayerIdx);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  LTDCのフレームバッファアドレスを設定する
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 Alpha     アドレス値
 *  parameter3 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_setaddress(LTDC_Handle_t *phandle, uint32_t Address, uint32_t LayerIdx)
{
	LTDC_LayerCfg_t *pLayerCfg;

	if(LayerIdx >= MAX_LAYER)
		return E_PAR;

	/* レイヤーコンフィギュレーション領域のポインタ取得 */
	pLayerCfg = &phandle->LayerCfg[LayerIdx];
	/* フレームバッファアドレス設定 */
	pLayerCfg->FBStartAdress = Address;
	/* LTDCレイヤー設定 */
	LTDC_SetConfig(phandle, pLayerCfg, LayerIdx);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  LTDCのウインドウサイズ位置を設定する
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 XSize     ウインドウXサイズ
 *  parameter3 YSize     ウインドウYサイズ
 *  parameter4 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_setwindowsize(LTDC_Handle_t *phandle, uint32_t XSize, uint32_t YSize, uint32_t LayerIdx)
{
	LTDC_LayerCfg_t *pLayerCfg;

	/* レイヤーコンフィギュレーション領域のポインタ取得 */
	pLayerCfg = &phandle->LayerCfg[LayerIdx];
	if(LayerIdx >= MAX_LAYER)
		return E_PAR;
	if(XSize > 8191 || YSize > 8191)
		return E_PAR;

	/* ウインドウX位置設定 */
	pLayerCfg->WindowX0 = 0;
	pLayerCfg->WindowX1 = XSize + pLayerCfg->WindowX0;
	/* ウインドウY位置設定 */
	pLayerCfg->WindowY0 = 0;
	pLayerCfg->WindowY1 = YSize + pLayerCfg->WindowY0;
	/* ウインドウXサイズ設定 */
	pLayerCfg->ImageWidth = XSize;
	/* ウインドウYサイズ */
	pLayerCfg->ImageHeight = YSize;
	/* LTDCレイヤー設定 */
	LTDC_SetConfig(phandle, pLayerCfg, LayerIdx);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  LTDCのウインドウ位置を設定する
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 X0        ウインドウX位置
 *  parameter3 Y0        ウインドウY位置
 *  parameter4 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_setwindowposition(LTDC_Handle_t *phandle, uint32_t X0, uint32_t Y0, uint32_t LayerIdx)
{
	LTDC_LayerCfg_t *pLayerCfg;

	/* レイヤーコンフィギュレーション領域のポインタ取得 */
	pLayerCfg = &phandle->LayerCfg[LayerIdx];
	if(LayerIdx >= MAX_LAYER)
		return E_PAR;
	if(X0 > 4095 || (X0+pLayerCfg->ImageWidth) > 65535)
		return E_PAR;
	if(Y0 > 4095 || (Y0+pLayerCfg->ImageHeight) > 65535)
		return E_PAR;

	/* ウインドウX位置設定 */
	pLayerCfg->WindowX0 = X0;
	pLayerCfg->WindowX1 = X0 + pLayerCfg->ImageWidth;
	/* ウインドウY位置設定 */
	pLayerCfg->WindowY0 = Y0;
	pLayerCfg->WindowY1 = Y0 + pLayerCfg->ImageHeight;
	/* LTDCレイヤー設定 */
	LTDC_SetConfig(phandle, pLayerCfg, LayerIdx);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  カラーキーングの初期設定を行う
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 RGBvalue  カラーキーイング番号
 *  parameter3 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_configcolorkeying(LTDC_Handle_t *phandle, uint32_t RGBValue, uint32_t LayerIdx)
{
	uint32_t layer = TADR_LTDC_LAYER1+LayerIdx*LTDC_WINDOW_SIZE;

	if(LayerIdx >= MAX_LAYER)
		return E_PAR;

	/* カラーキーイング番号設定 */
	sil_andw_mem((uint32_t *)(layer+TOFF_LTDCW_CKCR), (LTDC_CKCR_CKBLUE | LTDC_CKCR_CKGREEN | LTDC_CKCR_CKRED));
	sil_wrw_mem((uint32_t *)(layer+TOFF_LTDCW_CKCR), RGBValue);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  カラーキーングを有効にする
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_enablecolorkeying(LTDC_Handle_t *phandle, uint32_t LayerIdx)
{
	if(LayerIdx >= MAX_LAYER)
		return E_PAR;

	/* COLKENビットをオン */
	sil_orw_mem((uint32_t *)(TADR_LTDC_LAYER1+LayerIdx*LTDC_WINDOW_SIZE+TOFF_LTDCW_CR), LTDC_CR_COLKEN);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  カラーキーングを無効にする
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  parameter2 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_disablecolorkeying(LTDC_Handle_t *phandle, uint32_t LayerIdx)
{
	if(LayerIdx >= MAX_LAYER)
		return E_PAR;

	/* COLKENビットをオフ */
	sil_andw_mem((uint32_t *)(TADR_LTDC_LAYER1+LayerIdx*LTDC_WINDOW_SIZE+TOFF_LTDCW_CR), LTDC_CR_COLKEN);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  LTDCのレイヤー設定をおこなう
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  patameter2 pLayerCfg レイヤー設定構造体へのポインタ
 *  parameter3 LaerIdx   レイヤー番号
 *  return               正常ならばE_OK
 */
ER
ltdc_configlayer(LTDC_Handle_t *phandle, LTDC_LayerCfg_t *pLayerCfg, uint32_t LayerIdx)
{
	if(LayerIdx >= MAX_LAYER)
		return E_PAR;
	/* レイヤーコンフィグレーション設定をコピー */
	phandle->LayerCfg[LayerIdx] = *pLayerCfg;
	/* LTDCレイヤー設定 */
	LTDC_SetConfig(phandle, pLayerCfg, LayerIdx);
	/* リロード */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), LTDC_SRCR_IMR);
	return E_OK;
}

/*
 *  LTDCラインイベント設定
 *  parameter1 phandle   LTDCの初期設定構造体へのポインタ
 *  patameter2 Line      ライン位置
 *  return               正常ならばE_OK
 */
ER
ltdc_lineevent(LTDC_Handle_t *phandle, uint32_t Line)
{
	/* Change LTDC peripheral state */
	phandle->state = LTDC_STATE_BUSY;

	/* Enable the Line interrupt */
	sil_orw_mem((uint32_t *)(phandle->base+TOFF_LTDC_IER), LTDC_IER_LIE);

	/* Sets the Line Interrupt position */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_LIPCR), Line);

	/* Change the LTDC state*/
	phandle->state = LTDC_STATE_READY;
	return E_OK;
}

/*
 *  LTDCレジスタリロード設定
 *  parameter1 phandle    LTDCの初期設定構造体へのポインタ
 *  patameter2 ReloadType リロードタイプ
 *  return                正常ならばE_OK
 */
ER
ltdc_reload(LTDC_Handle_t *phandle, uint32_t ReloadType)
{
	/* Change LTDC peripheral state */
	phandle->state = LTDC_STATE_BUSY;

	/* Enable the Reload interrupt */  
	sil_orw_mem((uint32_t *)(phandle->base+TOFF_LTDC_IER), LTDC_IER_RRIE);

	/* Apply Reload type */
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SRCR), ReloadType);

	/* Change the LTDC state*/
	phandle->state = LTDC_STATE_READY;
	return E_OK;
}

/*
 *  LTDCの初期設定を行う
 *  parameter1 phandle  LTDCのハンドラへのポインタ
 *  return              正常ならE_OK
 */
ER
ltdc_init(LTDC_Handle_t *phandle)
{
	uint32_t timecount = 0;
	uint32_t tmpreg0 = 0;
	uint32_t tmpreg1 = 0;

	/*
	 * PLLSAIクロック停止
     */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLSAION);

    timecount = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & (RCC_CR_PLLSAIRDY)) == RCC_CR_PLLSAIRDY){
		if( timecount > PLLSAI_TIMEOUT_VALUE){
			return E_TMOUT;	/* タイムアウト発生 */
		}
		sil_dly_nse(1000);
		timecount++;
	}

	/*
	 *  PLLSAIP/PLLSAIQ設定
	 */
	tmpreg0 = ((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLSAICFGR)) & RCC_PLLSAICFGR_PLLSAIQ) >> RCC_PLLSAICFGR_PLLSAIQ_POS);
	tmpreg1 = ((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLSAICFGR)) & RCC_PLLSAICFGR_PLLSAIP) >> RCC_PLLSAICFGR_PLLSAIP_POS);
	sil_wrw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLSAICFGR), (phandle->Init.pllsain << 6) | (tmpreg1 << 16) | (tmpreg0 << 24) | (phandle->Init.pllsair << 28));
	sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_PLLSAIDIVR, phandle->Init.saidivr);

    /*
	 *  PLLSAIクロック許可
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLSAION);

    /*
	 *  PLLSAIレディ待ち
	 */
	timecount = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & (RCC_CR_PLLSAIRDY)) != RCC_CR_PLLSAIRDY){
		if( timecount > PLLSAI_TIMEOUT_VALUE){
			return E_TMOUT;	/* タイムアウト発生 */
		}
		sil_dly_nse(1000);
		timecount++;
	}

#ifdef TOPPERS_STM32F7_DISCOVERY
	/*
	 *  LTDCの初期設定
	 */
	/* GPIO設定 */
	ltdc_gpio_init();
#endif

	/* HS/VS/DE/PCの極性設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_GCR), (LTDC_GCR_HSPOL | LTDC_GCR_VSPOL | LTDC_GCR_DEPOL | LTDC_GCR_PCPOL),
				(phandle->Init.HSPolarity | phandle->Init.VSPolarity | phandle->Init.DEPolarity | phandle->Init.PCPolarity));
	/* 同調サイズ設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_SSCR), (LTDC_SSCR_VSH | LTDC_SSCR_HSW),
				((phandle->Init.HorizontalSync << 16) | phandle->Init.VerticalSync));
	/* バックポーチ設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_BPCR), (LTDC_BPCR_AVBP | LTDC_BPCR_AHBP),
				((phandle->Init.AccumulatedHBP << 16) | phandle->Init.AccumulatedVBP));
	/* アクティブ幅設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_AWCR), (LTDC_AWCR_AAH | LTDC_AWCR_AAW),
				((phandle->Init.AccumulatedActiveW << 16) | phandle->Init.AccumulatedActiveH));
	/* トータル幅設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_TWCR), (LTDC_TWCR_TOTALH | LTDC_TWCR_TOTALW),
				((phandle->Init.TotalWidth << 16) | phandle->Init.TotalHeigh));
	/* バックグラウンドカラー設定 */
	sil_modw_mem((uint32_t *)(phandle->base+TOFF_LTDC_BCCR), (LTDC_BCCR_BCBLUE | LTDC_BCCR_BCGREEN | LTDC_BCCR_BCRED),
				((phandle->Init.Backcolor.Red << 16) | (phandle->Init.Backcolor.Green << 8) | phandle->Init.Backcolor.Blue));
	/* エラーリセット */
	sil_orw_mem((uint32_t *)(phandle->base+TOFF_LTDC_IER), (LTDC_IER_TERRIE | LTDC_IER_FUIE));
	/* LTDC実行設定 */
	sil_orw_mem((uint32_t *)(phandle->base+TOFF_LTDC_GCR), LTDC_GCR_LTDCEN);
	phandle->state = LTDC_STATE_READY;
	return E_OK;
}

/*
 *  LTDC割込みハンドラ
 */
void
ltdc_irqhandler(LTDC_Handle_t *phandle)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(phandle->base+TOFF_LTDC_ISR));

	/*
	 *  割込みマスクとクリア
	 */
	sil_andw_mem((uint32_t *)(phandle->base+TOFF_LTDC_IER), isr);
	sil_wrw_mem((uint32_t *)(phandle->base+TOFF_LTDC_ICR), isr);

	/*
	 *  転送エラーチェック
	 */
	if((isr & LTDC_ISR_TERRIF) != 0){
		/* Update error code */
		phandle->errorcode |= LTDC_ERROR_TE;
		phandle->state      = LTDC_STATE_ERROR;
		if(phandle->errorcallback != NULL)
			phandle->errorcallback(phandle);
	}

	/*
	 *  FIFOアンダーランエラーチェック
	 */
	if((isr & LTDC_ISR_FUIF) != 0){
		/* Update error code */
		phandle->errorcode |= LTDC_ERROR_FU;
		phandle->state      = LTDC_STATE_ERROR;
		if(phandle->errorcallback != NULL)
			phandle->errorcallback(phandle);
	}

	/*
	 *  ラインイベントチェック
	 */
	if((isr & LTDC_IER_LIE) != 0){
		/* Change LTDC state */
		phandle->state = LTDC_STATE_READY;
		if(phandle->linecallback != NULL)
			phandle->linecallback(phandle);
	}

	/*
	 * リロードイベントチェック
	 */
	if((isr & LTDC_ISR_RRIF) != 0){
		/* Change LTDC state */
		phandle->state = LTDC_STATE_READY;
		if(phandle->reloadcallback != NULL)
			phandle->reloadcallback(phandle);
	}
}

