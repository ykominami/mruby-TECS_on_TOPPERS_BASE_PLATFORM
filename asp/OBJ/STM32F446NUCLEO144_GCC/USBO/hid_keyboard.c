/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: hid_keyboard.c 2416 2017-05-28 11:13:03Z roi $
 */
/* Copyright (c) 2010-2011 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* 
 *  USB HOST KEYBOARDコード生成
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "hid_appli.h"

static const uint8_t keymap[4][60] = {
	{
		0,   0,   0,   0,   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',	/* 000 - 011 */
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',	/* 012 - 023 */
		'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6',	/* 024 - 035 */
		'7', '8', '9', '0', 10,  27,  8,   9,   ' ', '-', '=', '[',	/* 036 - 047 */
		']', '\\','#', ';', '\'', 0,  ',', '.', '/', 0,   0,   0	/* 048 - 059 */
    },

    /* CTRL MODIFIER */
	{
		0,   0,   0,   0,   1,   2,   3,   4,   5,   6,   7,   8,	/* 000 - 011 */
		9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  18,  19,	/* 012 - 023 */
		20,  21,  22,  23,  24,  25,  0,   0,   0,   0,   0,   0,	/* 024 - 035 */
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	/* 036 - 047 */
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0	/* 048 - 059 */
	},

	/* SHIFT MODIFIER */
	{
		0,   0,   0,   0,   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',	/* 000 - 011 */
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',	/* 012 - 023 */
		'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', '#', '$', '%', '^',	/* 024 - 035 */
		'&', '*', '(', ')', 0,   0,   0,   0,   0,   0,   '+', '{',	/* 036 - 047 */
		'}', '|', '~', ':', '"', 0,   '<', '>', '?', 0,   0,   0	/* 048 - 059 */
	},

	/* ALT MODIFIER */
	{
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	/* 000 - 011 */
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	/* 012 - 023 */
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	/* 024 - 035 */
 		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	/* 036 - 047 */
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0	/* 048 - 059 */
	}
};

/*
 *  HID KEYBOARD初期化
 *  parameter1:  phandle  HIDハンドラ
 *  return       TUSBH_ERCODE
 */
TUSBH_ERCODE
HidKeyboardInit(void *phandle)
{
	syslog_0(LOG_NOTICE, "## USBH_HID_KeybdInit ##");
	return TUSBH_E_OK;
}

/*
 *  HID KEYBOARD情報取得
 *  parameter1:  phandle  HIDハンドラ
 *  parameter2:  pinfo    KEYBOARD情報設定タイプへのポインタ
 *  parameter3:  buffer   USB HOSTからのデータ
 *  parameter4:  length   USB HOSTデータ長
 *  return       TUSBH_ERCODE
 */
TUSBH_ERCODE
HidGetKeyboardInfo(void *phandle, KEYBOARD_Info_t *pinfo, uint8_t *buffer, uint16_t length)
{
	uint8_t i;

	if(length > 0){
		pinfo->lctrl = (buffer[0] >> 0) & 1;
		pinfo->lshift = (buffer[0] >> 1) & 1;
		pinfo->lalt = (buffer[0] >> 2) & 1;
		pinfo->lgui = (buffer[0] >> 3) & 1;
		pinfo->rctrl = (buffer[0] >> 4) & 1;
		pinfo->rshift = (buffer[0] >> 5) & 1;
		pinfo->ralt = (buffer[0] >> 6) & 1;
		pinfo->rgui = (buffer[0] >> 7) & 1;

		for(i = 0 ; i < sizeof(pinfo->keys) ; i++)
			pinfo->keys[i]=(uint32_t)buffer[i+2];
		return TUSBH_E_OK;
	}
	else
		return TUSBH_E_TMOUT;
}

/*
 *  HID ASCII-CODE取得
 *  parameter1:  phandle  HIDハンドラ
 *  parameter2:  pinfo    KEYBOARD情報設定タイプへのポインタ
 *  return       ASCII CODE
 */
uint8_t
HidGetASCIICode(KEYBOARD_Info_t *pinfo)
{
	uint8_t   index = 0;

	if(pinfo->lshift == 1 || pinfo->rshift == 1)
		index = 2;
	else if(pinfo->lctrl == 1 || pinfo->rctrl == 1)
		index = 1;
	else if(pinfo->lalt == 1 || pinfo->ralt == 1)
		index = 3;
	if(pinfo->keys[0] < 60)
		return keymap[index][pinfo->keys[0]];
	else
		return 0;
}

