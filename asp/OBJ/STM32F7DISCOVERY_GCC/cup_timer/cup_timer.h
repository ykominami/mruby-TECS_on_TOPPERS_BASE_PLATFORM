/*
 * カップラーメンタイマのヘッダファイル
 */

#define STACK_SIZE      2048  /* タスクのスタックサイズ */
#define DEFAULT_PRIORITY   8  /* タスクの優先度 */

#define I2C_PORTID    I2C3_PORTID

#define INHNO_I2CEV   IRQ_VECTOR_I2C3_EV	/* 割込みハンドラ番号 */
#define INTNO_I2CEV   IRQ_VECTOR_I2C3_EV	/* 割込み番号 */
#define INTPRI_I2CEV  -5			/* 割込み優先度 */
#define INTATR_I2CEV  0				/* 割込み属性 */

#define INHNO_I2CER   IRQ_VECTOR_I2C3_ER	/* 割込みハンドラ番号 */
#define INTNO_I2CER   IRQ_VECTOR_I2C3_ER	/* 割込み番号 */
#define INTPRI_I2CER  -5			/* 割込み優先度 */
#define INTATR_I2CER  0				/* 割込み属性 */


#define LED1_BLINK_INTERVAL 1000     /* LED1の点滅間隔   */
#define LED4_BLINK_INTERVAL  250     /* LED4の点滅間隔   */
#define SW_SCAN_INTERVAL      10     /* SWのスキャン間隔 */
#define BASE_TIME_INTERVAL   250     /* 基本時間間隔     */
#define TO_SEC               (1000/BASE_TIME_INTERVAL)

#define INIT_TIME      30     /* スタート時のタイムアウト時間 */
#define EXTRA_UNIT     30     /* 時間延長単位 */
#define TIMEOUT_BLINK  60     /* タイムアウト時の点滅回数 */
#define ACT_INTERVAL   10     /* タイマ動作通知LEDのインターバル */
#define ACT_BLINK_TIME  4     /* タイマ動作通知LEDの点滅回数 */


#ifndef TOPPERS_MACRO_ONLY
extern void timer_task(intptr_t exinf);
extern void blink_task(intptr_t exinf);

extern void led1_blink_handler(void);
extern void led4_blink_time_handler(void);
extern void sw_scan_handler(void);
extern void base_time_handler(void);

extern void device_info_init(intptr_t exinf);
extern void sw_handle(void);


#endif /* TOPPERS_MACRO_ONLY */

