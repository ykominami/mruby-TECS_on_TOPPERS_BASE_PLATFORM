/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
 * 
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
 *  $Id: sample1.c 2416 2012-09-07 08:06:20Z ertl-hiro $
 */

/* 
 *  サンプルプログラム(1)の本体
 *
 *  STM32F769I-Discoveryのデバイスデモプログラム．
 *
 *  プログラムの概要:
 *
 *  ユーザーキーを押すごとに、メニュー中のデモプログラムを実行する．
 *  デモプログラムは、6つ用意している．SDカードのデモ以外は
 *  STマイクロ社のSTM-Cubeで用意されているものと同等である．各プログラ
 *  ムはaspカーネル＋TOPPERS Base Platform(STM)V1.2上で動作する．
 *
 *  1.タッチスクリーン・デモ1
 *  2.タッチスクリーン・デモ2
 *  3.オーディオ演奏・デモ(雀の鳴き声)
 *  4.マイク入力とオーディオ出力(ROM実行でないと正常動作しない)
 *  5.SDファイル表示・デモ
 *  6.LCD表示・デモ
 *
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "sample1.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_audio.h"

#include "stlogo.h"
#include "topamelogo.h"

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))


BSP_DemoTypedef  BSP_examples[] =
  {
  {Touchscreen_demo1, "TOUCHSCREEN DEMO1", 0},
  {Touchscreen_demo2, "TOUCHSCREEN DEMO2", 0},
  {AudioPlay_demo, "AUDIO PLAY", 0},
  {AudioLoopback_demo, "AUDIO RECORD AND PLAY", 0},
  {SD_demo, "SD", 0},
/*  {QSPI_demo, "QSPI", 0}, */
 {LCD_demo, "LCD", 0},
/*  {SDRAM_demo, "SDRAM", 0},
  {SDRAM_DMA_demo, "SDRAM DMA", 0}, 
  {Log_demo, "LCD LOG", 0}, */
  };

static uint8_t DemoIndex = 0;
uint8_t NbLoop = 1;
uint8_t toggle_led = 0;
char    cbuff[260];

#if 1	/* ROI DEBUG */

uint32_t HAL_GetTick(void)
{
	SYSTIM tick;
	get_tim(&tick);
	return tick;
}

/**
  * @brief Toggle Leds
  * @param  None
  * @retval None
  */
void Toggle_Leds(void)
{
	static uint8_t ticks = 0;

	if(ticks++ > 100){
		toggle_led = 1;   /* Toggle LED performed in main loop */
		ticks = 0;
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* Turn LED REDon */
	BSP_LED_On(LED_RED);
	syslog_0(LOG_ERROR, "## audio loop Error ##");
	slp_tsk();
}


/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback13(void)
{
	syslog_0(LOG_NOTICE, "## HAL_GPIO_EXTI_Callback13 ##");
//  HAL_GPIO_EXTI_Callback(TS_INT_PIN);
}

#endif	/* ROI DEBUG */

/*
 *  CPU例外ハンドラ
 */
#ifdef CPUEXC1

void
cpuexc_handler(void *p_excinf)
{
	ID		tskid;

	syslog(LOG_NOTICE, "CPU exception handler (p_excinf = %08p).", p_excinf);
	if (sns_ctx() != true) {
		syslog(LOG_WARNING,
					"sns_ctx() is not true in CPU exception handler.");
	}
	if (sns_dpn() != true) {
		syslog(LOG_WARNING,
					"sns_dpn() is not true in CPU exception handler.");
	}
	syslog(LOG_INFO, "sns_loc = %d sns_dsp = %d sns_tex = %d",
									sns_loc(), sns_dsp(), sns_tex());
	syslog(LOG_INFO, "xsns_dpn = %d xsns_xpn = %d",
									xsns_dpn(p_excinf), xsns_xpn(p_excinf));

	if (xsns_xpn(p_excinf)) {
		syslog(LOG_NOTICE, "Sample program ends with exception.");
		SVC_PERROR(ext_ker());
		assert(0);
	}

	SVC_PERROR(iget_tid(&tskid));
	SVC_PERROR(iras_tex(tskid, 0x8000U));
}

#endif /* CPUEXC1 */

/*
 *  周期ハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void cyclic_handler(intptr_t exinf)
{
	Toggle_Leds();
	/* Check periodically the buffer state and fill played buffer with new data
	   following the state that has been updated by the BSP_AUDIO_OUT_TransferComplete_CallBack()
	   and BSP_AUDIO_OUT_HalfTransfer_CallBack() */
	AUDIO_Process();
}

/*
 *  アラームハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
void alarm_handler(intptr_t exinf)
{
	SVC_PERROR(irot_rdq(HIGH_PRIORITY));
	SVC_PERROR(irot_rdq(MID_PRIORITY));
	SVC_PERROR(irot_rdq(LOW_PRIORITY));
}

/*
 *  デモ用メイン画面
 */
static void Display_DemoDescription(void)
{
	/*
	 *  LCD初期化
	 */
	BSP_LCD_SelectLayer(0);
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);  

	/*
	 *  LCDメッセージ
	 */
	BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"STM32F769I BSP", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 35, (uint8_t *)"TOPPERS ASP2/BASE PLATFORM", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)"demo & driver examples", CENTER_MODE);

	/*
	 *  ロゴ表示
	 */
	BSP_LCD_DrawBitmap(BSP_LCD_GetXSize()/2 - 80, 90, (uint8_t *)stlogo);
	BSP_LCD_DrawBitmap(BSP_LCD_GetXSize()/2 + 5, 90, (uint8_t *)topamelogo);

	/*
	 *  Copy Right表示
	 */
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 40, (uint8_t *)"Copyright (c) STMicroelectronics 2016", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 20, (uint8_t *)"Copyright (c) TOPPERS PROJECT 2017", CENTER_MODE);

	/*
	 *  メニュー表示
	 */
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, BSP_LCD_GetYSize()/2 + 15, BSP_LCD_GetXSize(), 60);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE); 
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 30, (uint8_t *)"Press User button to start :", CENTER_MODE);
	sprintf(cbuff,"%s example", BSP_examples[DemoIndex].DemoName);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 + 45, (uint8_t *)cbuff, CENTER_MODE);
}


/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	uint8_t  lcd_status = LCD_OK;
	ER_UINT	ercd;

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	sta_cyc(CYCHDR1);
	exti15_func[13-EXTI15_BASENO] = HAL_GPIO_EXTI_Callback13;

	/*
	 *  LCD初期化
	 */
	lcd_status = BSP_LCD_Init();
	if(lcd_status != LCD_OK){
		syslog_1(LOG_ERROR, "## lcd_status(%d) ##", lcd_status);
		slp_tsk();
	}

	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

	Display_DemoDescription();

	/*
	 *  ユーザーメニュー処理
	 */
	while(1){
		if(toggle_led == 1){
			BSP_LED_Toggle(LED_GREEN);
			BSP_LED_Toggle(LED_RED);
			toggle_led = 0;
		}

		if(BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET){

			BSP_examples[DemoIndex++].DemoFunc();

			if(DemoIndex >= COUNT_OF_EXAMPLE(BSP_examples)){
				/* Increment number of loops which be used by EEPROM example */
				NbLoop++;
				DemoIndex = 0;
			}
			Display_DemoDescription();
		}
		dly_tsk(10);
	}

	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();

	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}

/*
 *  ユーザースイッチチェック
 *  return  switch state (1 : active / 0 : Inactive)
 */
uint8_t CheckForUserInput(void)
{
	if(BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET){
	    dly_tsk(10);
    	return 1 ;
	}
	return 0;
}


