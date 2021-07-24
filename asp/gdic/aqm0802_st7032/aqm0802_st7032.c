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
 *  $Id: aqm0802_st7032.c 2416 2017-11-23 11:40:56Z roi $
 */

/* 
 *  AQM0802 ST7032 LCD制御プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "aqm0802_st7032.h"

#define TXBUFFERSIZE  2

#if defined(TOPPERS_STM32F746_NUCLEO144) || defined(TOPPERS_STM32F767_NUCLEO144) || defined(TOPPERS_STM32F769_DISCOVERY)
#define i2c_send(h, a, b, l)    i2c_memwrite((h), (a), 0, 0, (b), (l), 500)
#else
#define i2c_send(h, a, b, l)    i2c_memwrite((h), (a), 0, 0, (b), (l))
#endif

static uint8_t aTxBuffer[TXBUFFERSIZE];

/*
 *  LCDの初期化
 */
ER
aqm0802_init(CLCD_Handler_t *hlcd)
{
	I2C_Handle_t *hi2c = hlcd->hi2c;
	ER ercd;

	hlcd->max_col = 8;
	hlcd->max_raw = 2;

	/*
	 *  LED COMMAND 0x38
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x38;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 70us
    sil_dly_nse(LCD_INTERVAL1*1000);
    /*
	 *  LCD COMMAND 0x39
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x39;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 *  LCD COMMAND 0x14
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x14;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 *  LCD COMMAND 0x70
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x70;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 *  LCD COMMAND 0x56
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x56;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 *  LCD COMMAND 0x6C
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x6C;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 200ms
	dly_tsk(200);
	/*
	 *  LCD COMMAND 0x38
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x38;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 * LCD COMMAND 0x0C Display ON/OFF control
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x0C;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 26.3us
    sil_dly_nse(LCD_INTERVAL2*1000);
	/*
	 *  LCD COMMAND 0x01 Clear Display
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x01;
	if((ercd = i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE)) != E_OK)
		return ercd;

    // Wait 1.08ms
	dly_tsk(2);
	/*
	 *  LCD COMMAND 0x80
	 */
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = 0x80;
	i2c_send(hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE);
	return ercd;
}

/*
 *  LCDへのコマンド送信関数
 */
ER
aqm0802_set_command(CLCD_Handler_t *hlcd, uint8_t c)
{
    aTxBuffer[0] = ST7032_CMD;
    aTxBuffer[1] = c;
	return i2c_send(hlcd->hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE);
}

/*
 *  LCDへのデータ送信関数
 */
ER
aqm0802_set_data(CLCD_Handler_t *hlcd, uint8_t *p, uint8_t len)
{
	ER  ercd = E_OK;
	int i;

	for(i = 0 ; i < len ; i++){
	    aTxBuffer[0] = ST7032_DAT;
    	aTxBuffer[1] = *p++;
		ercd = i2c_send(hlcd->hi2c, hlcd->saddr, aTxBuffer, TXBUFFERSIZE);
		if(ercd != E_OK)
			break;
    }
	return ercd;
}

