/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id: sample1.h 2416 2012-09-07 08:06:20Z ertl-hiro $
 */

/*
 *		サンプルプログラム(1)のヘッダファイル
 */

/*
 *  ターゲット依存の定義
 */
#include "target_test.h"

#include "device.h"
#include "i2c.h"

/*
 *  各タスクの優先度の定義
 */

#define MAIN_PRIORITY	5		/* メインタスクの優先度 */
								/* HIGH_PRIORITYより高くすること */

#define HIGH_PRIORITY	9		/* 並行実行されるタスクの優先度 */
#define MID_PRIORITY	10
#define LOW_PRIORITY	11

/*
 *  ターゲットに依存する可能性のある定数の定義
 */

#ifndef TASK_PORTID
#define	TASK_PORTID		1			/* 文字入力するシリアルポートID */
#endif /* TASK_PORTID */

#ifndef STACK_SIZE
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

#ifndef LOOP_REF
#define LOOP_REF		ULONG_C(1000000)	/* 速度計測用のループ回数 */
#endif /* LOOP_REF */

#define INHNO_LTDC      IRQ_VECTOR_LTDC
#define INTNO_LTDC      IRQ_VECTOR_LTDC
#define INTPRI_LTDC     -7        /* 割込み優先度 */
#define INTATR_LTDC     0         /* 割込み属性 */

#define INHNO_DSI       IRQ_VECTOR_DSI
#define INTNO_DSI       IRQ_VECTOR_DSI
#define INTPRI_DSI      -7        /* 割込み優先度 */
#define INTATR_DSI      0         /* 割込み属性 */

#define INHNO_I2C1EV    IRQ_VECTOR_I2C1_EV	/* 割込みハンドラ番号 */
#define INTNO_I2C1EV    IRQ_VECTOR_I2C1_EV	/* 割込み番号 */
#define INTPRI_I2C1EV   -5				/* 割込み優先度 */
#define INTATR_I2C1EV   0				/* 割込み属性 */

#define INHNO_I2C1ER    IRQ_VECTOR_I2C1_ER	/* 割込みハンドラ番号 */
#define INTNO_I2C1ER    IRQ_VECTOR_I2C1_ER	/* 割込み番号 */
#define INTPRI_I2C1ER   -5				/* 割込み優先度 */
#define INTATR_I2C1ER   0				/* 割込み属性 */

#define INHNO_I2C4EV    IRQ_VECTOR_I2C4_EV	/* 割込みハンドラ番号 */
#define INTNO_I2C4EV    IRQ_VECTOR_I2C4_EV	/* 割込み番号 */
#define INTPRI_I2C4EV   -5				/* 割込み優先度 */
#define INTATR_I2C4EV   0				/* 割込み属性 */

#define INHNO_I2C4ER    IRQ_VECTOR_I2C4_ER	/* 割込みハンドラ番号 */
#define INTNO_I2C4ER    IRQ_VECTOR_I2C4_ER	/* 割込み番号 */
#define INTPRI_I2C4ER   -5				/* 割込み優先度 */
#define INTATR_I2C4ER   0				/* 割込み属性 */

#define INHNO_DFSDM1F0  IRQ_DFSDM1_FLT0	/* 割込みハンドラ番号 */
#define INTNO_DFSDM1F0  IRQ_DFSDM1_FLT0	/* 割込み番号 */
#define INTPRI_DFSDM1F0 -10				/* 割込み優先度 */
#define INTATR_DFSDM1F0 0				/* 割込み属性 */

#define INHNO_DFSDM1F1  IRQ_DFSDM1_FLT1	/* 割込みハンドラ番号 */
#define INTNO_DFSDM1F1  IRQ_DFSDM1_FLT1	/* 割込み番号 */
#define INTPRI_DFSDM1F1 -10				/* 割込み優先度 */
#define INTATR_DFSDM1F1 0				/* 割込み属性 */

#define INHNO_DFSDM1F2  IRQ_DFSDM1_FLT2	/* 割込みハンドラ番号 */
#define INTNO_DFSDM1F2  IRQ_DFSDM1_FLT2	/* 割込み番号 */
#define INTPRI_DFSDM1F2 -10				/* 割込み優先度 */
#define INTATR_DFSDM1F2 0				/* 割込み属性 */

#define INHNO_DFSDM1F3  IRQ_DFSDM1_FLT3	/* 割込みハンドラ番号 */
#define INTNO_DFSDM1F3  IRQ_DFSDM1_FLT3	/* 割込み番号 */
#define INTPRI_DFSDM1F3 -10				/* 割込み優先度 */
#define INTATR_DFSDM1F3 0				/* 割込み属性 */


#define INHNO_DMAS21    IRQ_VECTOR_DMA2_STREAM1	/* 割込みハンドラ番号 */
#define INTNO_DMAS21    IRQ_VECTOR_DMA2_STREAM1	/* 割込み番号 */
#define INTPRI_DMAS21   -4			/* 割込み優先度 */
#define INTATR_DMAS21   0				/* 割込み属性 */

#define INHNO_DMAS23    IRQ_VECTOR_DMA2_STREAM3	/* 割込みハンドラ番号 */
#define INTNO_DMAS23    IRQ_VECTOR_DMA2_STREAM3	/* 割込み番号 */
#define INTPRI_DMAS23   -4			/* 割込み優先度 */
#define INTATR_DMAS23   0				/* 割込み属性 */

#define INHNO_DMAS24    IRQ_VECTOR_DMA2_STREAM4	/* 割込みハンドラ番号 */
#define INTNO_DMAS24    IRQ_VECTOR_DMA2_STREAM4	/* 割込み番号 */
#define INTPRI_DMAS24   -4			/* 割込み優先度 */
#define INTATR_DMAS24   0				/* 割込み属性 */

#define INHNO_DMAS26    IRQ_VECTOR_DMA2_STREAM6	/* 割込みハンドラ番号 */
#define INTNO_DMAS26    IRQ_VECTOR_DMA2_STREAM6	/* 割込み番号 */
#define INTPRI_DMAS26   -4			/* 割込み優先度 */
#define INTATR_DMAS26   0				/* 割込み属性 */

#define INHNO_DMAS27    IRQ_VECTOR_DMA2_STREAM7	/* 割込みハンドラ番号 */
#define INTNO_DMAS27    IRQ_VECTOR_DMA2_STREAM7	/* 割込み番号 */
#define INTPRI_DMAS27   -4			/* 割込み優先度 */
#define INTATR_DMAS27   0				/* 割込み属性 */


typedef struct
{
  void   (*DemoFunc)(void);
  uint8_t DemoName[50]; 
  uint32_t DemoIndex;
}BSP_DemoTypedef;

typedef enum {
  AUDIO_ERROR_NONE = 0,
  AUDIO_ERROR_NOTREADY,
  AUDIO_ERROR_IO,
  AUDIO_ERROR_EOF,
}AUDIO_ErrorTypeDef;

#define COUNT_OF_EXAMPLE(x)    (sizeof(x)/sizeof(BSP_DemoTypedef))

#ifdef USE_FULL_ASSERT
/* Assert activated */
#define BSP_TEST_APPLI_ASSERT(__error_condition__)    do { if(__error_condition__) \
                                                           {  while(1);  \
                                                           } \
                                                          } while(0)
#else
/* Assert not activated : macro has no effect */
#define BSP_TEST_APPLI_ASSERT(__error_condition__)    do { if(__error_condition__) \
                                                           {  ;  \
                                                           } \
                                                         } while(0)
#endif /* USE_FULL_ASSERT */

/* The Audio file is flashed with ST-Link Utility @ flash address =  AUDIO_SRC_FILE_ADDRESS */
#define AUDIO_SRC_FILE_ADDRESS       0x08080000   /* Audio file address in flash */
#define AUDIO_FILE_SIZE              0x80000

/*
 *  関数のプロトタイプ宣言
 */
#ifndef TOPPERS_MACRO_ONLY

extern void	task(intptr_t exinf);
extern void	main_task(intptr_t exinf);
extern void	tex_routine(TEXPTN texptn, intptr_t exinf);
#ifdef CPUEXC1
extern void	cpuexc_handler(void *p_excinf);
#endif /* CPUEXC1 */
extern void	cyclic_handler(intptr_t exinf);
extern void	alarm_handler(intptr_t exinf);
extern void sw_handle(void);
extern void ltdc_handler(void);
extern void dsi_handler(void);
extern void dfsdm1fl0_handler(void);
extern void dfsdm1fl1_handler(void);
extern void dfsdm1fl2_handler(void);
extern void dfsdm1fl3_handler(void);

/* Exported variables --------------------------------------------------------*/
extern const unsigned char stlogo[];
extern uint32_t SdmmcTest;
extern uint32_t SdramTest;
extern uint8_t  key_value;
extern char     cbuff[260];

/* Exported functions ------------------------------------------------------- */
uint8_t TouchScreen_GetTouchPosition(void);
void Touchscreen_DrawBackground_Circles(uint8_t state);
void     Touchscreen_demo1 (void);
void     Touchscreen_demo2 (void);
void AudioPlay_demo (void);
void AudioLoopback_demo (void);
uint8_t AUDIO_Process(void);
void LCD_demo (void);
void SD_demo (void);
void QSPI_demo (void);
void EEPROM_demo (void);
uint8_t CheckForUserInput(void);
void Toggle_Leds(void);
void Error_Handler(void);
void SDRAM_demo(void);
void SDRAM_DMA_demo (void);
void Log_demo(void);


#endif /* TOPPERS_MACRO_ONLY */
