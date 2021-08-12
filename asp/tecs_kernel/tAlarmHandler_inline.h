/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2009 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: tAlarmHandler_inline.h 308 2009-05-09 12:33:04Z ertl-takuya $
 */
/*
 * id               ID               ATTR_id
 */

/* 受け口関数 #_TEPF_# */
/* #[<ENTRY_PORT>]# eAlarm
 * entry port: eAlarm
 * signature:  sAlarm
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eAlarm_start
 * name:         eAlarm_start
 * global_name:  tAlarmHandler_eAlarm_start
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eAlarm_start(CELLIDX idx, RELTIM alarmTime)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(sta_alm(ATTR_id, alarmTime));
}

/* #[<ENTRY_FUNC>]# eAlarm_stop
 * name:         eAlarm_stop
 * global_name:  tAlarmHandler_eAlarm_stop
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eAlarm_stop(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(stp_alm(ATTR_id));
}

/* #[<ENTRY_FUNC>]# eAlarm_refer
 * name:         eAlarm_refer
 * global_name:  tAlarmHandler_eAlarm_refer
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eAlarm_refer(CELLIDX idx, T_RALM* pk_alarmStatus)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ref_alm(ATTR_id, pk_alarmStatus));
}

/* #[<ENTRY_PORT>]# eiAlarm
 * entry port: eiAlarm
 * signature:  siAlarm
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eiAlarm_start
 * name:         eiAlarm_start
 * global_name:  tAlarmHandler_eiAlarm_start
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eiAlarm_start(CELLIDX idx, RELTIM alarmTime)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ista_alm(ATTR_id, alarmTime));
}

/* #[<ENTRY_FUNC>]# eiAlarm_stop
 * name:         eiAlarm_stop
 * global_name:  tAlarmHandler_eiAlarm_stop
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eiAlarm_stop(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(istp_alm(ATTR_id));
}

