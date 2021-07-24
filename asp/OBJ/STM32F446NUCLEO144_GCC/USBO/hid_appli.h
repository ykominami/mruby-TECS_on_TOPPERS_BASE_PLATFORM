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
 *  $Id: hid_appli.h 2416 2017-05-28 11:17:20Z roi $
 */

/*
 *	USB HOST HIDアプリのヘッダファイル
 */
#include "tusb_types.h"

#ifndef TOPPERS_MACRO_ONLY

typedef struct
{
	uint8_t              button_left:1;
	uint8_t              button_right:1;
	uint8_t              button_middle:1;
	int8_t               x;
	int8_t               y;
	int8_t               z;
} MOUSE_Info_t;

typedef struct
{
	uint8_t lctrl:1;
	uint8_t lshift:1;
	uint8_t lalt:1;
	uint8_t lgui:1;
	uint8_t rctrl:1;
	uint8_t rshift:1;
	uint8_t ralt:1;
	uint8_t rgui:1;
	uint8_t keys[6];
} KEYBOARD_Info_t;

TUSBH_ERCODE HidMouseInit(void *phandle);
TUSBH_ERCODE HidMouseInfo(void *phandle, MOUSE_Info_t *p, uint8_t *buffer, uint16_t length);

TUSBH_ERCODE HidKeyboardInit(void *phandle);
TUSBH_ERCODE HidGetKeyboardInfo(void *phandle, KEYBOARD_Info_t *pinfo, uint8_t *buffer, uint16_t length);
uint8_t HidGetASCIICode(KEYBOARD_Info_t *info);

#endif /* TOPPERS_MACRO_ONLY */
