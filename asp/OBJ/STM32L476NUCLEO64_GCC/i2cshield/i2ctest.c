/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: i2ctest.c 2416 2017-07-19 08:06:20Z ertl-hiro $
 */

/* 
 *  I2Cテストの本体
 *  PORTID=1  PORT1
 *  PORTID=3  PORT3
 *  PORT2はArduinoコネクタに対応していない
 *  時間表示を行うには、RTCコマンド日時の入力が必要
 *   (RTCのバッテリバックアップはない)
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include <target_syssvc.h>
#include <string.h>
#include "device.h"
#include "rtc.h"
#include "i2ctest.h"

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


/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 80 MHz */
/* This example use TIMING to 0x00D00E28 to reach 1 MHz speed (Rise time = 120ns, Fall time = 25ns) */
#define I2C_TIMING      0x00D00E28

/* Size of Reception buffer */
#define RXBUFFERSIZE    8

/*
 *  スレーブアドレス定義
 */
#define LCD_ADDR        (0x3E<<1)
#define EEPROM_ADDR     (0x50<<1)
#define	ADT7410_ADDR    (0x48<<1)

#define ADT7410_TEMP_H	0x00
#define ADT7410_TEMP_L	0x01
#define ADT7410_STATUS	0x02

#define LCD_locate(h, x, y) aqm0802_set_command((h), (0x80 + ((y) << 6) + (x)))
#define LCD_puts(h, b)      aqm0802_set_data((h), (b), strlen((const char *)(b)))


/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

uint8_t lcd_buf[2][12];

/*
 *  SW1割込み
 */
void sw_int(int arg)
{
	syslog_1(LOG_NOTICE, "## sw_int(%d) ##", arg);
}

/*
 *  I2C SEND CALLBACK FUNCTION
 */
static void
I2C_TxCpltCallback(I2C_Handle_t *hi2c)
{
	syslog_1(LOG_INFO, "## I2C_TxCpltCallback(%08x) ##", hi2c);
}

/*
 *  I2C RECEIVE CALLBACK
 */
static void
I2C_RxCpltCallback(I2C_Handle_t *hi2c)
{
	syslog_1(LOG_INFO, "## I2C_RxCpltCallback(%08x) ##", hi2c);
}

/*
 *  I2C ERROR CALLBACK
 */
static void
I2C_ErrorCallback(I2C_Handle_t *hi2c)
{
	syslog_1(LOG_ERROR, "## I2C_ErrorCallback(%08x) ##", hi2c);
}

static void
set_value(uint8_t *buf, int value)
{
    buf[1] = (value % 10) + '0';
    buf[0] = (value / 10) + '0';
}

static void
set_time(uint8_t *buf, long time)
{
	struct tm2 timedate;

	rtc_get_time(&timedate);
    set_value(&buf[6], timedate.tm_sec);
    buf[5] = ':';
    set_value(&buf[3], timedate.tm_min);
    buf[2] = ':';
    set_value(&buf[0], timedate.tm_hour);
}

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	I2C_Init_t i2c_initd;
	I2C_Handle_t *hi2c;
	CLCD_Handler_t  CLcdHandle;
	ER_UINT	ercd;
	SYSTIM  tim;
	uint32_t temp;
	uint32_t count = 0;

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

	syslog_0(LOG_NOTICE, "I2C SEND/RECV START");

	/*
	 *  I2C初期化ポートはコンパイルスイッチに従う
	 */
	i2c_initd.Timing          = I2C_TIMING;
//  i2c_initd.ClockSpeed      = 400000;
//	i2c_initd.DutyCycle       = I2C_DUTYCYCLE_16_9;
	i2c_initd.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	i2c_initd.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2c_initd.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2c_initd.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
	i2c_initd.OwnAddress1     = 0;
	i2c_initd.OwnAddress2     = 0;
	i2c_initd.OwnAddress2Masks = I2C_OA2_NOMASK;
	i2c_initd.semid           = I2CTRS_SEM;
	i2c_initd.semlock         = I2CLOC_SEM;

	if((hi2c = i2c_init(I2C_PORTID, &i2c_initd)) == NULL){
		/* Initialization Error */
		syslog_0(LOG_ERROR, "## I2C ERROR(1) ##");
	}
	syslog_0(LOG_NOTICE, "LCD INITAILIZE !");

	hi2c->writecallback = I2C_TxCpltCallback;
	hi2c->readcallback  = I2C_RxCpltCallback;
	hi2c->errorcallback = I2C_ErrorCallback;

	memset(&lcd_buf[0][0], ' ', 8);
	memset(&lcd_buf[1][0], ' ', 8);

	CLcdHandle.hi2c  = hi2c;
	CLcdHandle.saddr = LCD_ADDR;
	if((ercd = aqm0802_init(&CLcdHandle)) != E_OK){
		syslog_2(LOG_ERROR, "## LCD INIT ERROR(%d)[%08x] ##", ercd, hi2c->ErrorCode);
		goto stop_task;
	}

	get_tim(&tim);
	set_time(&lcd_buf[0][0], tim);
	LCD_locate(&CLcdHandle, 0, 0);
	LCD_puts(&CLcdHandle, &lcd_buf[0][0]);

#ifdef EEPROM_ADDR
	syslog_1(LOG_NOTICE, "READ EEPROM [%08x] !", aRxBuffer);
	if(i2c_memread(hi2c, (uint16_t)EEPROM_ADDR, 0, I2C_MEMADD_SIZE_16BIT, (uint8_t*)aRxBuffer, RXBUFFERSIZE)!= E_OK){
	    /* I2C RECEIVE ERROR */
		syslog_1(LOG_ERROR, "## I2C ERROR(12)[%08x] ##", hi2c->ErrorCode);
		goto stop_task;
	}
#endif

	syslog_0(LOG_NOTICE, "TEMP SENSER !");
	do{
		/* STATUS READ */
		if(i2c_memread(hi2c, (uint16_t)ADT7410_ADDR, ADT7410_STATUS, I2C_MEMADD_SIZE_8BIT, (uint8_t*)aRxBuffer, 1)!= E_OK){
			/* I2C RECEIVE ERROR */
			syslog_1(LOG_ERROR, "## I2C ERROR(13)[%08x] ##", hi2c->ErrorCode);
			goto stop_task;
		}
		temp = aRxBuffer[0];
	}while((temp & 0x80) != 0);

	while(count < 1000){
		if(i2c_memread(hi2c, (uint16_t)ADT7410_ADDR, ADT7410_TEMP_H, I2C_MEMADD_SIZE_8BIT, (uint8_t*)aRxBuffer, 1)!= E_OK){
			/* I2C RECEIVE ERROR */
			syslog_1(LOG_ERROR, "## I2C ERROR(14)[%08x] ##", hi2c->ErrorCode);
			goto stop_task;
		}
		temp = aRxBuffer[0] << 8;
		if(i2c_memread(hi2c, (uint16_t)ADT7410_ADDR, ADT7410_TEMP_L, I2C_MEMADD_SIZE_8BIT, (uint8_t*)aRxBuffer, 1)!= E_OK){
			/* I2C RECEIVE ERROR */
			syslog_1(LOG_ERROR, "## I2C ERROR(15)[%08x] ##", hi2c->ErrorCode);
			goto stop_task;
		}
		temp += aRxBuffer[0];
		temp /= 13;

		get_tim(&tim);
		set_time(&lcd_buf[0][0], tim);
		LCD_locate(&CLcdHandle, 0, 0);
		LCD_puts(&CLcdHandle, &lcd_buf[0][0]);
		set_value(&lcd_buf[1][0], temp/10);
		lcd_buf[1][2] = '.';
		lcd_buf[1][3] = (temp % 10) + '0';
		LCD_locate(&CLcdHandle, 0, 1);
		LCD_puts(&CLcdHandle, &lcd_buf[1][0]);
		if((count % 10) == 0)
			syslog_2(LOG_NOTICE, "count(%d) temp(%d)", count, temp);
		dly_tsk(1000);
		count++;
	}

stop_task:
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}
