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
 *  @(#) $Id: tDataqueue_inline.h 308 2009-05-09 12:33:04Z ertl-takuya $
 */

/*
 * 属性アクセスマクロ #_CAAM_#
 * id               ID               ATTR_id         
 */

/* 受け口関数 #_TEPF_# */
/* #[<ENTRY_PORT>]# eDataqueue
 * entry port: eDataqueue
 * signature:  sDataqueue
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eDataqueue_send
 * name:         eDataqueue_send
 * global_name:  tDataqueue_eDataqueue_send
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_send(CELLIDX idx, intptr_t data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(snd_dtq(ATTR_id, data));
}

/* #[<ENTRY_FUNC>]# eDataqueue_sendPolling
 * name:         eDataqueue_sendPolling
 * global_name:  tDataqueue_eDataqueue_sendPolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_sendPolling(CELLIDX idx, intptr_t data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(psnd_dtq(ATTR_id, data));
}
/* #[<ENTRY_FUNC>]# eDataqueue_sendTimeout
 * name:         eDataqueue_sendTimeout
 * global_name:  tDataqueue_eDataqueue_sendTimeout
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_sendTimeout(CELLIDX idx, intptr_t data, TMO timeout)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(tsnd_dtq(ATTR_id, data, timeout));
}

/* #[<ENTRY_FUNC>]# eDataqueue_sendForce
 * name:         eDataqueue_sendForce
 * global_name:  tDataqueue_eDataqueue_sendForce
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_sendForce(CELLIDX idx, intptr_t data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(fsnd_dtq(ATTR_id, data));
}

/* #[<ENTRY_FUNC>]# eDataqueue_receive
 * name:         eDataqueue_receive
 * global_name:  tDataqueue_eDataqueue_receive
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_receive(CELLIDX idx, intptr_t* p_data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(rcv_dtq(ATTR_id, p_data));
}

/* #[<ENTRY_FUNC>]# eDataqueue_receivePolling
 * name:         eDataqueue_receivePolling
 * global_name:  tDataqueue_eDataqueue_receivePolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_receivePolling(CELLIDX idx, intptr_t* p_data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(prcv_dtq(ATTR_id, p_data));
}

/* #[<ENTRY_FUNC>]# eDataqueue_receiveTimeout
 * name:         eDataqueue_receiveTimeout
 * global_name:  tDataqueue_eDataqueue_receiveTimeout
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_receiveTimeout(CELLIDX idx, intptr_t* p_data, TMO timeout)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(trcv_dtq(ATTR_id, p_data, timeout));
}

/* #[<ENTRY_FUNC>]# eDataqueue_initialize
 * name:         eDataqueue_initialize
 * global_name:  tDataqueue_eDataqueue_initialize
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_initialize(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ini_dtq(ATTR_id));
}

/* #[<ENTRY_FUNC>]# eDataqueue_refer
 * name:         eDataqueue_refer
 * global_name:  tDataqueue_eDataqueue_refer
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eDataqueue_refer(CELLIDX idx, T_RDTQ* pk_dataqueueStatus)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ref_dtq(ATTR_id, pk_dataqueueStatus));
}

/* #[<ENTRY_PORT>]# eiDataqueue
 * entry port: eiDataqueue
 * signature:  siDataqueue
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eiDataqueue_sendPolling
 * name:         eiDataqueue_sendPolling
 * global_name:  tDataqueue_eiDataqueue_sendPolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eiDataqueue_sendPolling(CELLIDX idx, intptr_t data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ipsnd_dtq(ATTR_id, data));
}

/* #[<ENTRY_FUNC>]# eiDataqueue_sendForce
 * name:         eiDataqueue_sendForce
 * global_name:  tDataqueue_eiDataqueue_sendForce
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eiDataqueue_sendForce(CELLIDX idx, intptr_t data)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ifsnd_dtq(ATTR_id,  data));
}

