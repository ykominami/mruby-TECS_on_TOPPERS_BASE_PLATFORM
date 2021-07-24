/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2012 by Ryosuke Takeuchi
 *                     GJ Business Division RICOH COMPANY,LTD. JAPAN
 * 
 *  上記著作権者は，Free Software Foundation によって公表されている 
 *  GNU General Public License の Version 2 に記述されている条件か，以
 *  下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェア（本ソフトウェ
 *  アを改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを再利用可能なバイナリコード（リロケータブルオブ
 *      ジェクトファイルやライブラリなど）の形で利用する場合には，利用
 *      に伴うドキュメント（利用者マニュアルなど）に，上記の著作権表示，
 *      この利用条件および下記の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを再利用不可能なバイナリコードの形または機器に組
 *      み込んだ形で利用する場合には，次のいずれかの条件を満たすこと．
 *    (a) 利用に伴うドキュメント（利用者マニュアルなど）に，上記の著作
 *        権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 利用の形態を，別に定める方法によって，上記著作権者に報告する
 *        こと．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者を免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者は，
 *  本ソフトウェアに関して，その適用可能性も含めて，いかなる保証も行わ
 *  ない．また，本ソフトウェアの利用により直接的または間接的に生じたい
 *  かなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: task_expansion.c,v 1.3 2015/11/07 14:46:20 roi Exp $
 */

/*
 *	TOPPERS/ASP用タスク管理拡張機能
 */

#include "kernel_impl.h"
#include "check.h"
#include "task.h"
#include "task_expansion.h"
#include "kernel_cfg.h"
#include "target_timer.h"

/*
 *  コンテクスト中からの強制待ち状態への移行
 */
ER
isus_tsk(ID tskid)
{
	TCB	*tcb;
	UINT	tstat;
	ER	ercd;

	CHECK_INTCTX_UNL();
	CHECK_TSKID(tskid);
	tcb = get_tcb(tskid);

	i_lock_cpu();
	if (TSTAT_DORMANT(tstat = tcb->tstat)) {
		ercd = E_OBJ;
	}
	else if (TSTAT_RUNNABLE(tstat)) {
		/*
		 *  実行できる状態から強制待ち状態への遷移
		 */
		tcb->tstat = TS_SUSPENDED;
		if (make_non_runnable(tcb)) {
			reqflg = TRUE;
		}
		ercd = E_OK;
	}
	else if (TSTAT_SUSPENDED(tstat)) {
		ercd = E_QOVR;
	}
	else {
		/*
		 *  待ち状態から二重待ち状態への遷移
		 */
		tcb->tstat |= TS_SUSPENDED;
		ercd = E_OK;
	}
	i_unlock_cpu();

    error_exit:
	return(ercd);
}

/*
 *  タスク状態参照（簡易版）
 */
ER
ref_tst(ID tskid, T_RTST *pk_rtst)
{
	TCB	*tcb;
	UB	tstat;
	ER	ercd;

	CHECK_TSKCTX_UNL();
	CHECK_TSKID_SELF(tskid);
	tcb = get_tcb_self(tskid);

	t_lock_cpu();
	pk_rtst->tskwait  = 0;
	pk_rtst->tskpri   = EXT_TSKPRI(tcb->priority);
#ifdef USE_TSKINICTXB
	pk_rtst->inistk   = (VP)tcb->p_tinib->tskinictxb.stk_bottom;
	pk_rtst->inistksz = tcb->p_tinib->tskinictxb.stksz;
#else
	pk_rtst->inistk   = (VP)tcb->p_tinib->stk;
	pk_rtst->inistksz = tcb->p_tinib->stksz;
#endif

	tstat = tcb->tstat;
	if (TSTAT_RUNNABLE(tstat)) {
		if (tcb == p_runtsk) {
			pk_rtst->tskstat = TTS_RUN;
		}
		else {
			pk_rtst->tskstat = TTS_RDY;
		}
	}
	else if (TSTAT_WAITING(tstat)) {
		if (TSTAT_SUSPENDED(tstat)) {
			pk_rtst->tskstat = TTS_WAS;
		}
		else {
			pk_rtst->tskstat = TTS_WAI;
		}
		if ((tstat & TS_WAIT_MASK) == TS_WAIT_DLY) {
			pk_rtst->tskwait = TTW_DLY;
		}
		else if ((tstat & TS_WAIT_MASK) == TS_WAIT_SLP) {
			pk_rtst->tskwait = TTW_SLP;
		}
		else if ((tstat & TS_WAIT_MASK) == TS_WAIT_RDTQ) {
			pk_rtst->tskwait = TTW_RDTQ;
		}
		else {
			pk_rtst->tskwait = TTW_OTHR;
		}
	}
	else if (TSTAT_SUSPENDED(tstat)) {
		pk_rtst->tskstat = TTS_SUS;
	}
	else {
		pk_rtst->tskstat = TTS_DMT;
	}
	if (pk_rtst->tskstat == TTS_DMT)
		pk_rtst->tskpc = (FP)tcb->p_tinib->task;
	else
		pk_rtst->tskpc = (FP)tcb->tskctxb.pc;
	pk_rtst->tsksp   = (VP)tcb->tskctxb.sp;
	ercd = E_OK;
	t_unlock_cpu();

    error_exit:
	return(ercd);
}

/*
 *  タスク状態のロギング格納領域
 */
static T_TLOG   tsk_log[MAX_TASK_LOG+1];
static uint32_t check_tskid;
static SYSTIM   check_time;
static SYSTIM   pervious_time;

/*
 *  システム時間取り出し関数
 *  割込み中でも使用でき、共通化できるように
 *  インライン関数とする．
 */
Inline SYSTIM get_systime(void)
{
	return next_time;
}

/*
 *  タスクの実測時間測定用の時間
 *  取り出し関数、CPUロック状態で呼び出す．
 */
Inline SYSTIM get_anatime(void)
{
	SYSTIM  time;
#if TIC_DENO != 1
	INT     subtime;
#endif /* TIC_DENO != 1 */
	CLOCK   clock1, clock2;
	BOOL    ireq;

	/*
	 * 時間の取り出しはコンテキスト中でも行われるので
	 * get_utmと同等の記述をここにおく。
	 */
#if TIC_DENO != 1
	subtime = (INT) next_subtime;
#endif /* TIC_DENO != 1 */
	clock1 = target_timer_get_current();
	ireq = target_timer_probe_int();
	clock2 = target_timer_get_current();

	time = get_systime() * ANA_STIC;
#if TIC_DENO != 1
	time += subtime * ANA_STIC / TIC_DENO;
#endif /* TIC_DENO != 1 */
	if (!(ireq) || clock1 > clock2) {
		time -= TIC_NUME * ANA_STIC / TIC_DENO;
	}
	time += clock1 * ANA_STIC / TIMER_CLOCK;
	return time;
}

/*
 *  タスク実行領域設定関数
 *  この関数はdispatchの開始時、CPUロック状態で呼び出す．
 *  STM32F746はこの関数を複雑にするとハングアップ時間の方はあきらめる．
 */
void log_dsp_enter(TCB *runtsk)
{
#if !defined(TOPPERS_ARM_M) || defined(TOPPERS_STM32F4_DISCOVERY)
	SYSTIM time = get_systime();
#endif	/* ROI DEBUG */
	T_TLOG  *t;

	if(check_tskid < MAX_TASK_LOG)
		t = &tsk_log[check_tskid];
	else
		t = &tsk_log[MAX_TASK_LOG];
#if !defined(TOPPERS_ARM_M) || defined(TOPPERS_STM32F4_DISCOVERY)
	t->runtimes += time - check_time;
	check_time = time;
#endif
	if(p_schedtsk == 0){
		check_tskid = 0;
#if defined(TOPPERS_ARM_M) && !defined(TOPPERS_STM32F4_DISCOVERY)
		t->runtimes += 2;
//		check_time  += 2;
#endif
	}
}

/*
 *  タスク実行領域設定関数
 *  この関数はdispatchの終了時時、CPUロック状態で呼び出す．
 */
void log_dsp_leave(TCB *runtsk)
{
	SYSTIM time = get_anatime();
	T_TLOG  *t;

	if(check_tskid < MAX_TASK_LOG)
		t = &tsk_log[check_tskid];
	else
		t = &tsk_log[MAX_TASK_LOG];
	t->runtimes += time - check_time;

	if(runtsk){
		check_tskid = TSKID(runtsk);
		t = &tsk_log[check_tskid];
		t->runcount++;
	}
	else
		check_tskid = 0;
	check_time = time;
}

/*
 *  タスクログ状態取り出し関数
 */
int_t get_tsklog(T_TPRM * pprm)
{
	INT no;
	INT num_item=0;

	get_tim(&pprm->currtime);
	t_lock_cpu();
	pprm->pervtime = pervious_time;
	pervious_time  = pprm->currtime;
	for(no = 0 ; no <= tmax_tskid && no <= MAX_TASK_LOG ; no++){
		if(no < NUM_LDSP){
			pprm->tlog[no] = tsk_log[no];
			num_item = no;
		}
		else{
			pprm->tlog[1].runcount += tsk_log[no].runcount;
			pprm->tlog[1].runtimes += tsk_log[no].runtimes;
			num_item = NUM_LDSP-1;
		}
		tsk_log[no].runcount = 0;
		tsk_log[no].runtimes = 0;
	}
	t_unlock_cpu();
	return num_item;
}


/*
 *  デバイス・ポートのログアウト要求判定用データ領域
 */

static T_PCHK port_log[NUM_PCHK];

/*
 *  デバイスログを行うかどうかの判定
 */
static uint_t check_device_log(ulong_t address, ulong_t size)
{
	int    i;
	T_PCHK *p;

	for(i = 0 ; i < NUM_PCHK ; i++){
		p = &port_log[i];
		if(p->portaddress >= address && p->portaddress < (address+size) && p->logtype)
			return (UINT)p->logtype;
	}
	return 0;
}

/*
 *  デバイス・ポートのログ判定データの取り出し
 */
T_PCHK *get_device_log(ulong_t no){
	if(no < NUM_PCHK)
		return &port_log[no];
	else
		return 0;
}

/*
 *  デバイス読み出し表示判定とログアウト関数
 */
void
ana_rdv(ulong_t address, ulong_t data, int_t size)
{
	SYSTIM time = get_systime();
	ID     id;
	uint_t logtype = check_device_log(address, size);

	if(logtype == 0)
		return;
	if(sense_context() || !p_runtsk)
		syslog_3(logtype, "Device Read  time=%09d interrupt port=%08x data=0x%x", time, address, data);
	else{
		id = TSKID(p_runtsk);
		switch(size){
		case 4:
			syslog_4(logtype, "Device Read  time=%09d task=%04d port=%08x data=%08x", time, id, address, data);
			break;
		case 2:
			syslog_4(logtype, "Device Read  time=%09d task=%04d port=%08x data=%04x", time, id, address, data);
			break;
		default:
			syslog_4(logtype, "Device Read  time=%09d task=%04d port=%08x data=%02x", time, id, address, data);
			break;
		}
	}
}

/*
 *  デバイス書き込み表示判定とログアウト関数
 */
void
ana_wdv(ulong_t address, ulong_t data, int_t size)
{
	SYSTIM time = get_systime();
	ID     id;
	uint_t logtype = check_device_log(address, size);

	if(logtype == 0)
		return;
	if(sense_context() || !p_runtsk)
		syslog_3(logtype, "Device Write  time=%09d interrupt port=%08x data=0x%x", time, address, data);
	else{
		id = TSKID(p_runtsk);
		switch(size){
		case 4:
			syslog_4(logtype, "Device Write time=%09d task=%04d port=%08x data=%08x", time, id, address, data);
			break;
		case 2:
			syslog_4(logtype, "Device Write time=%09d task=%04d port=%08x data=%04x", time, id, address, data);
			break;
		default:
			syslog_4(logtype, "Device Write time=%09d task=%04d port=%08x data=%02x", time, id, address, data);
			break;
		}
	}
}

