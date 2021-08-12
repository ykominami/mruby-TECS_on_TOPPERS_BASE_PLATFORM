/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2009 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: tSample1.c 300 2009-05-08 12:20:30Z ertl-takuya $
 */

/* 
 *  サンプルプログラム(1)の本体
 *
 *  ASPカーネルの基本的な動作を確認するためのサンプルプログラム．
 *
 *  プログラムの概要:
 *
 *  ユーザインタフェースを受け持つメインタスク（タスクID: MAIN_TASK，優
 *  先度: MAIN_PRIORITY）と，3つの並列実行されるタスク（タスクID:
 *  TASK1〜TASK3，初期優先度: MID_PRIORITY）で構成される．また，起動周
 *  期が2秒の周期ハンドラ（周期ハンドラID: CYCHDR1）を用いる．
 *
 *  並列実行されるタスクは，task_loop回空ループを実行する度に，タスクが
 *  実行中であることをあらわすメッセージを表示する．
 *
 *  周期ハンドラは，三つの優先度（HIGH_PRIORITY，MID_PRIORITY，
 *  LOW_PRIORITY）のレディキューを回転させる．プログラムの起動直後は，
 *  周期ハンドラは停止状態になっている．
 *
 *  メインタスクは，シリアルI/Oポートからの文字入力を行い（文字入力を
 *  待っている間は，並列実行されるタスクが実行されている），入力された
 *  文字に対応した処理を実行する．入力された文字と処理の関係は次の通り．
 *  Control-Cまたは'Q'が入力されると，プログラムを終了する．
 *
 *  '1' : 対象タスクをTASK1に切り換える（初期設定）．
 *  '2' : 対象タスクをTASK2に切り換える．
 *  '3' : 対象タスクをTASK3に切り換える．
 *  'a' : 対象タスクをcTask_activateにより起動する．
 *  'A' : 対象タスクに対する起動要求をcTask_cancelActivateによりキャンセルする．
 *  'e' : 対象タスクにexitTaskを呼び出させ，終了させる．
 *  't' : 対象タスクをcTask_terminateにより強制終了する．
 *  '>' : 対象タスクの優先度をHIGH_PRIORITYにする．
 *  '=' : 対象タスクの優先度をMID_PRIORITYにする．
 *  '<' : 対象タスクの優先度をLOW_PRIORITYにする．
 *  'G' : 対象タスクの優先度をcTask_getPriorityで読み出す．
 *  's' : 対象タスクにsleepを呼び出させ，起床待ちにさせる．
 *  'S' : 対象タスクにsleepTimeout10秒)を呼び出させ，起床待ちにさせる．
 *  'w' : 対象タスクをcTask_wakeupにより起床する．
 *  'W' : 対象タスクに対する起床要求をcTask_cancelWakeupによりキャンセルする．
 *  'l' : 対象タスクをcTask_releaseWaitにより強制的に待ち解除にする．
 *  'u' : 対象タスクをcTask_suspendにより強制待ち状態にする．
 *  'm' : 対象タスクの強制待ち状態をcTask_resumeにより解除する．
 *  'd' : 対象タスクにdelay(10秒)を呼び出させ，時間経過待ちにさせる．
 *  'x' : 対象タスクに例外パターン0x0001の例外処理を要求する．
 *  'X' : 対象タスクに例外パターン0x0002の例外処理を要求する．
 *  'y' : 対象タスクにdisableTaskExceptionを呼び出させ，タスク例外を禁止する．
 *  'Y' : 対象タスクにenableTaskExceptionを呼び出させ，タスク例外を許可する．
 *  'r' : 3つの優先度（HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY）のレ
 *        ディキューを回転させる．
 *  'c' : 周期ハンドラを動作開始させる．
 *  'C' : 周期ハンドラを動作停止させる．
 *  'b' : アラームハンドラを5秒後に起動するよう動作開始させる．
 *  'B' : アラームハンドラを動作停止させる．
 *  'z' : 対象タスクにCPU例外を発生させる（タスクを終了させる）．
 *  'Z' : 対象タスクにCPUロック状態でCPU例外を発生させる（プログラムを
 *        終了する）．
 *  'V' : getMicroTimeで性能評価用システム時刻を2回読む．
 *  'v' : 発行したシステムコールを表示する（デフォルト）．
 *  'q' : 発行したシステムコールを表示しない．
 * 呼び口関数 #_TCPF_#
 * require port : signature: sKernel context: task
 *   ER             sleep( );
 *   ER             sleepTimeout( TMO timeout );
 *   ER             delay( RELTIM delay_time );
 *   ER             rotateReadyQueue( PRI priority );
 *   ER             exitTask( );
 *   ER             getTaskId( ID* p_task_id );
 *   ER             getTime( SYSTIM* p_system_time );
 *   ER             getMicroTime( SYSUTM* p_system_micro_time );
 *   ER             lockCpu( );
 *   ER             unlockCpu( );
 *   ER             disableDispatch( );
 *   ER             enableDispatch( );
 *   ER             disableTaskException( );
 *   ER             enableTaskException( );
 *   ER             changeInterruptPriorityMask( PRI interrupt_priority );
 *   ER             getInterruptPriorityMask( PRI* p_interrupt_priority );
 *   ER             exitKernel( );
 *   bool_t         senseContext( );
 *   bool_t         senseLock( );
 *   bool_t         senseDispatch( );
 *   bool_t         senseDispatchPendingState( );
 *   bool_t         senseKernel( );
 * require port : signature: siKernel context: non-task
 *   ER             ciKernel_rotateReadyQueue( PRI priority );
 *   ER             ciKernel_getTaskId( ID* p_task_id );
 *   ER             ciKernel_getMicroTime( SYSUTM* p_system_micro_time );
 *   ER             ciKernel_lockCpu( );
 *   ER             ciKernel_unlockCpu( );
 *   ER             ciKernel_exitKernel( );
 *   bool_t         ciKernel_senseContext( );
 *   bool_t         ciKernel_senseLock( );
 *   bool_t         ciKernel_senseDispatch( );
 *   bool_t         ciKernel_senseDispatchPendingState( );
 *   bool_t         ciKernel_senseKernel( );
 *   bool_t         ciKernel_senseDispatchPendingStateCPU( const void* p_exception_infomation );
 *   bool_t         ciKernel_senseTaskExceptionPendingStateCPU( const void* p_exception_infomation );
 * call port : cTask  signature: sTask context: task
 *   ER             cTask_activate( subscript );
 *   ER_UINT        cTask_cancelActivate( subscript );
 *   ER             cTask_terminate( subscript );
 *   ER             cTask_changePriority( subscript, PRI priority );
 *   ER             cTask_getPriority( subscript, PRI* p_priority );
 *   ER             cTask_refer( subscript, T_RTSK* pk_task_status );
 *   ER             cTask_wakeup( subscript );
 *   ER_UINT        cTask_cancelWakeup( subscript );
 *   ER             cTask_releaseWait( subscript );
 *   ER             cTask_suspend( subscript );
 *   ER             cTask_resume( subscript );
 *   ER             cTask_raiseException( subscript, TEXPTN pattern );
 * call port : cCyclic  signature: sCyclic context: task
 *   ER             cCyclic_start( );
 *   ER             cCyclic_stop( );
 *   ER             cCyclic_refer( T_RCYC* pk_cyclic_handler_status );
 * call port : cAlarm  signature: sAlarm context: task
 *   ER             cAlarm_start( RELTIM alarm_time );
 *   ER             cAlarm_stop( );
 *   ER             cAlarm_refer( T_RALM* pk_alarm_status );
 * call port : cSerialPort  signature: sSerialPort context: task
 *   ER             cSerialPort_open( );
 *   ER             cSerialPort_close( );
 *   ER_UINT        cSerialPort_read( char_t* buf, uint_t len );
 *   ER_UINT        cSerialPort_write( const char_t* buf, uint_t len );
 *   ER             cSerialPort_control( uint_t ioctl );
 *   ER             cSerialPort_refer( T_SERIAL_RPOR* pk_rpor );
 * call port : cSysLog  signature: sSysLog context: task
 *   ER             cSysLog_write( uint_t prio, const SYSLOG* p_syslog );
 *   ER_UINT        cSysLog_read( SYSLOG* p_syslog );
 *   ER             cSysLog_mask( uint_t logmask, uint_t lowmask );
 *   ER             cSysLog_refer( T_SYSLOG_RLOG* pk_rlog );
 *
 * #[</PREAMBLE>]# */ 

#include "tSample1_tecsgen.h"
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "kernel_cfg.h"
#include "tSample1.h"

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

/*
 *  並行実行されるタスクへのメッセージ領域
 */
char_t	message[3];

/*
 *  ループ回数
 */
ulong_t	task_loop;		/* タスク内でのループ回数 */
ulong_t	tex_loop;		/* 例外処理ルーチン内でのループ回数 */

/*
 *  並行実行されるタスク
 */
void eSampleTask_main(int_t subscript)
{
	volatile ulong_t	i;
	int_t		n = 0;
	int_t		tskno = subscript + 1; 
	const char	*graph[] = { "|", "  +", "    *" };
	char_t		c;

	SVC_PERROR(enableTaskException());
	while (1) {
		syslog(LOG_NOTICE, "task%d is running (%03d).   %s",
										tskno, ++n, graph[tskno-1]);
		for (i = 0; i < task_loop; i++);
		c = message[tskno-1];
		message[tskno-1] = 0;
		switch (c) {
		case 'e':
			syslog(LOG_INFO, "#%d#exitTask()", tskno);
			SVC_PERROR(exitTask());
			assert(0);
		case 's':
			syslog(LOG_INFO, "#%d#sleep()", tskno);
			SVC_PERROR(sleep());
			break;
		case 'S':
			syslog(LOG_INFO, "#%d#sleepTimeout(10000)", tskno);
			SVC_PERROR(sleepTimeout(10000));
			break;
		case 'd':
			syslog(LOG_INFO, "#%d#delay(10000)", tskno);
			SVC_PERROR(delay(10000));
			break;
		case 'y':
			syslog(LOG_INFO, "#%d#disableTaskException()", tskno);
			SVC_PERROR(disableTaskException());
			break;
		case 'Y':
			syslog(LOG_INFO, "#%d#enableTaskException()", tskno);
			SVC_PERROR(enableTaskException());
			break;
#ifdef CPUEXC1
		case 'z':
			syslog(LOG_NOTICE, "#%d#raise CPU exception", tskno);
			RAISE_CPU_EXCEPTION;
			break;
		case 'Z':
			SVC_PERROR(lockCpu());
			syslog(LOG_NOTICE, "#%d#raise CPU exception", tskno);
			RAISE_CPU_EXCEPTION;
			SVC_PERROR(unlockCpu());
			break;
#endif /* CPUEXC1 */
		default:
			break;
		}
	}
}

/*
 *  並行して実行されるタスク用のタスク例外処理ルーチン
 */
void
eSampleException_main(int_t subscript, TEXPTN pattern)
{
	volatile ulong_t	i;
	int_t	tskno = (int_t) subscript;

	syslog(LOG_NOTICE, "task%d receives exception 0x%04x.", tskno + 1, pattern);
	for (i = 0; i < tex_loop; i++);

	if ((pattern & 0x8000U) != 0U) {
		syslog(LOG_INFO, "#%d#ext_tsk()", tskno);
		SVC_PERROR(exitTask());
		assert(0);
	}
}

/*
 *  CPU例外ハンドラ
 */
#ifdef CPUEXC1

void
cpuexc_handler(void *p_excinf)
{
	ID		tskid;

	syslog(LOG_NOTICE, "CPU exception handler (p_excinf = %08p).", p_excinf);
	if (ciKernel_senseContext() != true) {
		syslog(LOG_WARNING,
					"ciKernel_senseContext() is not true in CPU exception handler.");
	}
	if (ciKernel_senseDispatchPendingState() != true) {
		syslog(LOG_WARNING,
					"ciKernel_senseDispatchPendingState() is not true in CPU exception handler.");
	}
	syslog(LOG_INFO, "ciKernel_senseLock() = %d ciKernel_senseDispatch() = %d ciKernel_senseContext() = %d",
		   ciKernel_senseLock(), ciKernel_senseDispatch(), ciKernel_senseContext());
	syslog(LOG_INFO, "xsns_dpn = %d xsns_xpn = %d",
		   ciKernel_senseDispatchPendingStateCPU(p_excinf),
		   ciKernel_senseDispatchPendingStateCPU(p_excinf));

	if (xsns_xpn(p_excinf)) {
		syslog(LOG_NOTICE, "Sample program ends with exception.");
		SVC_PERROR(ciKernel_senseKernel());
		assert(0);
	}

	SVC_PERROR(ciKernel_getTaskId(&tskid));
	SVC_PERROR(iras_tex(tskid, 0x8000U));
	
}

#endif /* CPUEXC1 */

/*
 *  周期ハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
/* #[<ENTRY_FUNC>]# eiCyclicHandler_main
 * name:         eiCyclicHandler_main
 * global_name:  tSample1_eiCyclicHandler_main
 * oneway:       
 * #[/ENTRY_FUNC>]# */
void
eiCyclicHandler_main()
{
	SVC_PERROR(ciKernel_rotateReadyQueue(HIGH_PRIORITY));
	SVC_PERROR(ciKernel_rotateReadyQueue(MID_PRIORITY));
	SVC_PERROR(ciKernel_rotateReadyQueue(LOW_PRIORITY));
}

/*
 *  アラームハンドラ
 *
 *  HIGH_PRIORITY，MID_PRIORITY，LOW_PRIORITY の各優先度のレディキュー
 *  を回転させる．
 */
/* #[<ENTRY_FUNC>]# eiAlarmHandler_main
 * name:         eiAlarmHandler_main
 * global_name:  tSample1_eiAlarmHandler_main
 * oneway:       
 * #[/ENTRY_FUNC>]# */
void
eiAlarmHandler_main()
{
	SVC_PERROR(ciKernel_rotateReadyQueue(HIGH_PRIORITY));
	SVC_PERROR(ciKernel_rotateReadyQueue(MID_PRIORITY));
	SVC_PERROR(ciKernel_rotateReadyQueue(LOW_PRIORITY));
}
/*
 *  メインタスク
 */
/* 属性の設定 *//* #[<ENTRY_FUNC>]# eMainTask_main
 * name:         eMainTask_main
 * global_name:  tSample1_eMainTask_main
 * oneway:       
 * #[/ENTRY_FUNC>]# */
void
eMainTask_main()
{
	char_t	c;
	volatile ulong_t	i;
	int_t	tskno = 1;
	ER_UINT	ercd;	
	PRI		tskpri;
	SYSTIM	stime1, stime2;
#ifdef TOPPERS_SUPPORT_GET_UTM
	SYSUTM	utime1, utime2;
#endif /* TOPPERS_SUPPORT_GET_UTM */

	SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts.");
	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = cSerialPort_open();
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `cSerialPort_open'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(cSerialPort_control(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV));

	/*
 	 *  ループ回数の設定
	 *
	 *  TASK_LOOPがマクロ定義されている場合，測定せずに，TASK_LOOPに定
	 *  義された値を，タスク内でのループ回数とする．
	 *
	 *  MEASURE_TWICEがマクロ定義されている場合，1回目の測定結果を捨て
	 *  て，2回目の測定結果を使う．1回目の測定は長めの時間が出るため．
	 */
#ifdef TASK_LOOP
	task_loop = TASK_LOOP;
#else /* TASK_LOOP */

#ifdef MEASURE_TWICE
	task_loop = LOOP_REF;
	SVC_PERROR(getTime(&stime1));
	for (i = 0; i < task_loop; i++);
	SVC_PERROR(getTime(&stime2));
#endif /* MEASURE_TWICE */

	task_loop = LOOP_REF;
	SVC_PERROR(getTime(&stime1));
	for (i = 0; i < task_loop; i++);
	SVC_PERROR(getTime(&stime2));
	task_loop = LOOP_REF * 400UL / (stime2 - stime1);

#endif /* TASK_LOOP */
	tex_loop = task_loop / 5;

	/*
 	 *  タスクの起動
	 */

	SVC_PERROR(cTask_activate( 1 ));
	SVC_PERROR(cTask_activate( 2 ));
	SVC_PERROR(cTask_activate( 3 ));

	/*
 	 *  メインループ
	 */
	do {
		SVC_PERROR(cSerialPort_read(&c, 1));
		switch (c) {
		case 'e':
		case 's':
		case 'S':
		case 'd':
		case 'y':
		case 'Y':
		case 'z':
		case 'Z':
			message[tskno-1] = c;
			break;
		case '1':
			tskno = 1;
			break;
		case '2':
			tskno = 2;
			break;
		case '3':
			tskno = 3;
			break;
		case 'a':
			syslog(LOG_INFO, "#cTask_activate(%d)", tskno);
			SVC_PERROR(cTask_activate(tskno));
			break;
		case 'A':
			syslog(LOG_INFO, "#cTask_cancelActivate(%d)", tskno);
			SVC_PERROR(cTask_cancelActivate(tskno));

			if (ercd >= 0) {
				syslog(LOG_NOTICE, "cTask_cancelActivate(%d) returns %d", tskno, ercd);
			}
			break;
		case 't':
			syslog(LOG_INFO, "#cTask_terminate(%d)", tskno);
			SVC_PERROR(cTask_terminate(tskno));
			break;
		case '>':
			syslog(LOG_INFO, "#cTask_changePriority(%d, HIGH_PRIORITY)", tskno);
			SVC_PERROR(cTask_changePriority(tskno, HIGH_PRIORITY));
			break;
		case '=':
			syslog(LOG_INFO, "#cTask_changePriority(%d, MID_PRIORITY)", tskno);
			SVC_PERROR(cTask_changePriority(tskno, MID_PRIORITY));
			break;
		case '<':
			syslog(LOG_INFO, "#(cTask_changePriority(%d, LOW_PRIORITY)", tskno);
			SVC_PERROR(cTask_changePriority(tskno, LOW_PRIORITY));
			break;
		case 'G':
			syslog(LOG_INFO, "#cTask_getPriority(%d, &tskpri)", tskno);
			SVC_PERROR(ercd = cTask_getPriority(tskno, &tskpri));
			if (ercd >= 0) {
				syslog(LOG_NOTICE, "priority of task %d is %d", tskno, tskpri);
			}
			break;
		case 'w':
			syslog(LOG_INFO, "#cTask_wakeup(%d)", tskno);
			SVC_PERROR(cTask_wakeup(tskno));
			break;
		case 'W':
			syslog(LOG_INFO, "#cTask_cancelWakeup(%d)", tskno);
			SVC_PERROR(ercd = cTask_cancelWakeup(tskno));
			if (ercd >= 0) {
				syslog(LOG_NOTICE, "cTask_cancelWakeup(%d) returns %d", tskno, ercd);
			}
			break;
		case 'l':
			syslog(LOG_INFO, "#cTask_releaseWait(%d)", tskno);
			SVC_PERROR(cTask_releaseWait(tskno));
			break;
		case 'u':
			syslog(LOG_INFO, "#cTask_suspend(%d)", tskno);
			SVC_PERROR(cTask_suspend(tskno));
			break;
		case 'm':
			syslog(LOG_INFO, "#cTask_resume(%d)", tskno);
			SVC_PERROR(cTask_resume(tskno));
			break;
		case 'x':
			syslog(LOG_INFO, "#cTask_raiseException(%d, 0x0001U)", tskno);
			SVC_PERROR(cTask_raiseException(tskno, 0x0001U));
			break;
		case 'X':
			syslog(LOG_INFO, "#cTask_raiseException(%d, 0x0002U)", tskno);
			SVC_PERROR(cTask_raiseException(tskno, 0x0002U));
			break;
		case 'r':
			syslog(LOG_INFO, "#rotateReadyQueue(three priorities)");
			SVC_PERROR(rotateReadyQueue(HIGH_PRIORITY));
			SVC_PERROR(rotateReadyQueue(MID_PRIORITY));
			SVC_PERROR(rotateReadyQueue(LOW_PRIORITY));
			break;
		case 'c':
			syslog(LOG_INFO, "#cCyclic_start(1)");
			SVC_PERROR(cCyclic_start());
			break;
		case 'C':
			syslog(LOG_INFO, "#cCyclic_stop(1)");
			SVC_PERROR(cCyclic_stop());
			break;
		case 'b':
			syslog(LOG_INFO, "#cAlarm_start(1, 5000)");
			SVC_PERROR(cAlarm_start(5000));
			break;
		case 'B':
			syslog(LOG_INFO, "#cAlarm_stop()(1)");
			SVC_PERROR(cAlarm_stop());
			break;
		case 'V':
#ifdef TOPPERS_SUPPORT_GET_UTM
			SVC_PERROR(getMicroTime(&utime1));
			SVC_PERROR(getMicroTime(&utime2));
			syslog(LOG_NOTICE, "utime1 = %ld, utime2 = %ld",
										(ulong_t) utime1, (ulong_t) utime2);
#else /* TOPPERS_SUPPORT_GET_UTM */
			syslog(LOG_NOTICE, "getMicroTime is not supported.");
#endif /* TOPPERS_SUPPORT_GET_UTM */
			break;
		case 'v':
			SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_INFO),
										LOG_UPTO(LOG_EMERG)));
			break;
		case 'q':
			SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_NOTICE),
										LOG_UPTO(LOG_EMERG)));
			break;
#ifdef BIT_KERNEL
		case ' ':
			SVC_PERROR(lockCpu());
			{
				extern ER	bit_kernel(void);

				SVC_PERROR(ercd = bit_kernel());
				if (ercd >= 0) {
					syslog(LOG_NOTICE, "bit_kernel passed.");
				}
			}
			SVC_PERROR(unlockCpu());
			break;
#endif /* BIT_KERNEL */
		default:
			break;
		}
	} while (c != '\003' && c != 'Q');

	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ciKernel_exitKernel());
	assert(0);
}
