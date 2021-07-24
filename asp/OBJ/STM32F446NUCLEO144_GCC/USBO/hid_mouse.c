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
 *  $Id: hid_mouse.c 2416 2017-05-28 11:13:03Z roi $
 */

/* 
 *  USB HOST MOUSEデータ生成
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "hid_appli.h"

/*
 *  HID MOUSE初期化
 *  parameter1:  phandle  HIDハンドラ
 *  return       TUSBH_ERCODE
 */
TUSBH_ERCODE
HidMouseInit(void *phandle)
{
	syslog_0(LOG_NOTICE, "## HidMouseInit ##");
	return TUSBH_E_OK;
}

/*
 *  HID MOUSE情報取得
 *  parameter1:  phandle  HIDハンドラ
 *  parameter2:  pinfo    MOUSE情報設定タイプへのポインタ
 *  parameter3:  buffer   USB HOSTからのデータ
 *  parameter4:  length   USB HOSTデータ長
 *  return       TUSBH_ERCODE
 */
TUSBH_ERCODE
HidMouseInfo(void *phandle, MOUSE_Info_t *pinfo, uint8_t *buffer, uint16_t length)
{
	if(length > 0){
		pinfo->x = (int8_t )buffer[1];
		pinfo->y = (int8_t )buffer[2];
		pinfo->z = (int8_t )buffer[3];
		pinfo->button_left=(uint8_t)(buffer[0] & 1);
		pinfo->button_right=(uint8_t)((buffer[0] >> 1) & 1);
		pinfo->button_middle=(uint8_t)((buffer[0] >> 2) & 1);
		return TUSBH_E_OK;
	}
	else
		return TUSBH_E_TMOUT;
}

