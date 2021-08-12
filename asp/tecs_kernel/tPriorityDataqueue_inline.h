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
 *  @(#) $Id: tPriorityDataqueue_inline.h 308 2009-05-09 12:33:04Z ertl-takuya $
 */

/*
 * 属性アクセスマクロ #_CAAM_#
 * id               ID               ATTR_id
 */

/* 受け口関数 #_TEPF_# */
/* #[<ENTRY_PORT>]# ePriorityDataqueue
 * entry port: ePriorityDataqueue
 * signature:  sPriorityDataqueue
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_send
 * name:         ePriorityDataqueue_send
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_send
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_send(CELLIDX idx, intptr_t data, PRI dataPriority)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(snd_pdq(ATTR_id, data, dataPriority));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_sendPolling
 * name:         ePriorityDataqueue_sendPolling
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_sendPolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_sendPolling(CELLIDX idx, intptr_t data, PRI dataPriority)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(psnd_pdq(ATTR_id, data, dataPriority));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_sendTimeout
 * name:         ePriorityDataqueue_sendTimeout
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_sendTimeout
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_sendTimeout(CELLIDX idx, intptr_t data, PRI dataPriority, TMO timeout)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(tsnd_pdq(ATTR_id, data, dataPriority, timeout));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_receive
 * name:         ePriorityDataqueue_receive
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_receive
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_receive(CELLIDX idx, intptr_t* p_data, PRI* p_dataPriority)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(rcv_pdq(ATTR_id, p_data, p_dataPriority));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_receivePolling
 * name:         ePriorityDataqueue_receivePolling
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_receivePolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_receivePolling(CELLIDX idx, intptr_t* p_data, PRI* p_dataPriority)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(prcv_pdq(ATTR_id, p_data, p_dataPriority));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_receiveTimeout
 * name:         ePriorityDataqueue_receiveTimeout
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_receiveTimeout
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_receiveTimeout(CELLIDX idx, intptr_t* p_data, PRI* p_dataPriority, TMO timeout)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(trcv_pdq(ATTR_id, p_data, p_dataPriority, timeout));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_initialize
 * name:         ePriorityDataqueue_initialize
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_initialize
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_initialize(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ini_pdq(ATTR_id));
}

/* #[<ENTRY_FUNC>]# ePriorityDataqueue_refer
 * name:         ePriorityDataqueue_refer
 * global_name:  tPriorityDataqueue_ePriorityDataqueue_refer
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
ePriorityDataqueue_refer(CELLIDX idx, T_RPDQ* pk_priorityDataqueueStatus)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ref_pdq(ATTR_id, pk_priorityDataqueueStatus));
}

/* #[<ENTRY_PORT>]# eiPriorityDataqueue
 * entry port: eiPriorityDataqueue
 * signature:  siPriorityDataqueue
 * context:    task
 * params: 
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eiPriorityDataqueue_sendPolling
 * name:         eiPriorityDataqueue_sendPolling
 * global_name:  tPriorityDataqueue_eiPriorityDataqueue_sendPolling
 * oneway:       
 * #[/ENTRY_FUNC>]# */
Inline ER
eiPriorityDataqueue_sendPolling(CELLIDX idx, intptr_t data, PRI dataPriority)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	return(ipsnd_pdq(ATTR_id, data, dataPriority));
}

