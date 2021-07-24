/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2015 by Ryosuke Takeuchi
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
 *  @(#) $Id: monitor.c,v 2.1 2016/08/02 17:12:20 roi Exp $
 */

/* 
 *  TOPPERS/ASP用タスクモニタプログラム．
 *
 */

#include <kernel.h>
#include <stdio.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "task_expansion.h"
#include "task.h"
#include "kernel_cfg.h"
#include "monitor.h"

#ifndef NUM_ARGC
#define NUM_ARGC    16
#endif

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
 *  モニタ内定義
 */
#define	HEX_VALUE			16			/* 16進指定 */
#define	DEC_VALUE			10			/* 10進指定 */

#define	MONDISPLAY_BYTE	DATA_BYTE	/* バイト単位メモリ表示 */
#define	MONDISPLAY_HALF	DATA_HALF	/* ハーフ単位メモリ表示 */
#define	MONDISPLAY_WORD	DATA_WORD	/* ワード単位メモリ表示 */
#define	MONDISPLAY_TASK	3			/* タスク状態表示 */
#define	MONDISPLAY_REG	5			/* レジスタの表示 */
#define	MONDISPLAY_ASM	6

#define	MONSET_BYTE		DATA_BYTE	/* バイト単位メモリ設定 */
#define	MONSET_HALF		DATA_HALF	/* ハーフ単位メモリ設定 */
#define	MONSET_WORD		DATA_WORD	/* ワード単位メモリ設定 */
#define	MONSET_COMMAND	3			/* コマンドモード設定 */
#define	MONSET_SERIAL	5			/* シリアル設定 */
#define	MONSET_TASK		6			/* タスク設定 */
#define MONSET_IN       7			/* 入力設定 */

struct SUBCOMMAND_TABLE {
	const char *subcommand;			/* サブコマンド文 */
	const char type;				/* 実行タイプ */
};

/*
 *  プロトタイプ宣言
 *
 *  このモジュール内で使用している関数
 */
static void    prompt(ID tskid);
static int_t   dispatch_command(int argc, char **argv);
static int_t   display_command(int argc, char **argv);
static int_t   set_command(int argc, char **argv);
static int_t   help_command(int argc, char **argv);
static int_t   task_activate(int argc, char **argv);
static int_t   task_terminate(int argc, char **argv);
static int_t   task_suspend(int argc, char **argv);
static int_t   task_resume(int argc, char **argv);
static int_t   task_release(int argc, char **argv);
static int_t   task_wakeup(int argc, char **argv);
static int_t   task_priority(int argc, char **argv);
static int_t   log_mode(int argc, char **argv);
static int_t   log_task(int argc, char **argv);
static int_t   log_port(int argc, char **argv);
static uint_t  get_taskstate(STAT tskstat);
static char    monitor_getstring(char *s, int *len);

extern const char	*itron_strerror(ER ercd);

/*
 *  モニタバナー表示
 */
static char const banner[] = "\n"
"ASP TASK Monitor Release %d.%d.%d for " TARGET_NAME
" (" __DATE__ ", " __TIME__ ")\n"
"Copyright (C) 2003-2017 by RICOH COMPANY,LTD. JAPAN\n";

/*
 *  ヘルプメッセージ
 */
static char const display_help[] =
"  Display BYTE     [start address] [end address]\n"
"          HALF     [start address] [end address]\n"
"          WORD     [start address] [end address]\n"
"          TASK\n"
"          REGISTER\n";

static char const set_help[] =
"  Set     BYTE     [start address]\n"
"          HALF     [start address]\n"
"          WORD     [start address]\n"
"          COMMAND  [mode]  mode=1 or (2)\n"
"          SERIAL   [port id]\n"
"          TASK     [task id]\n";

static char const task_help[] =
"  Task    ACTIVATE          (act_tsk)\n"
"          TERMINATE         (ter_tsk)\n"
"          SUSPEND           (sus_tsk)\n"
"          RESUME            (rsm_tsk)\n"
"          RELEASE           (rel_tsk)\n"
"          WAKEUP            (wup_tsk)\n"
"          PRIORITY [pri]    (chg_pri)\n";

static char const log_help[] =
"  Log     MODE     [logmask] [lowmask]\n"
"          TASK     [cycle time(ms)] [count]\n"
"          PORT     [no] [logno] [portaddress]\n";

static char const help_help[] =
"  Help    [arg1]\n";

/*
 *  タスクコマンドサブテーブル
 */
static const COMMAND_INFO task_command_info[] = {
	{"ACTIVATE",	task_activate},
	{"TERMINATE",	task_terminate},
	{"SUSPEND",		task_suspend},
	{"RESUME",		task_resume},
	{"RELEASE",		task_release},
	{"WAKEUP",		task_wakeup},
	{"PRIORITY",	task_priority}
};

/*
 *  ログコマンドサブテーブル
 */
static const COMMAND_INFO log_command_info[] = {
	{"MODE",		log_mode},
	{"TASK",		log_task},
	{"PORT",		log_port}
};

/*
 *  メインコマンドテーブル
 */
static COMMAND_LINK const standard_command[] = {
	{NULL,	0,	"DISPLAY",	display_command,	display_help,	NULL },
	{NULL,	0,	"SET",		set_command,		set_help,		NULL },
	{NULL,	7,	"TASK",		NULL,				task_help,		task_command_info },
	{NULL,	3,	"LOG",		NULL,				log_help,		log_command_info },
	{NULL,	0,	"HELP",		help_command,		help_help,		NULL }
};

/*
 *  表示コマンドテーブル
 */
static struct SUBCOMMAND_TABLE const mon_display[] = {
	{"BYTE",		MONDISPLAY_BYTE },	/* バイト単位メモリ表示 */
	{"HALF",		MONDISPLAY_HALF },	/* ハーフ単位メモリ表示 */
	{"WORD",		MONDISPLAY_WORD },	/* ワード単位メモリ表示 */
	{"TASK",		MONDISPLAY_TASK },	/* タスク状態表示 */
	{"REGISTER",	MONDISPLAY_REG  },	/* タスクレジスター表示 */
	{"ASSEMBLER",	MONDISPLAY_ASM  }	/* アセンブラー表示 */
};

/*
 *  設定コマンドテーブル
 */
static struct SUBCOMMAND_TABLE const mon_set[] = {
	{"BYTE",		MONSET_BYTE     },	/* バイト単位メモリ設定 */
	{"HALF",		MONSET_HALF     },	/* ハーフ単位メモリ設定 */
	{"WORD",		MONSET_WORD     },	/* ワード単位メモリ設定 */
	{"COMMAND",		MONSET_COMMAND  },	/* コマンドモード設定 */
	{"SERIAL",		MONSET_SERIAL   },	/* シリアル設定 */
	{"TASK",		MONSET_TASK     },	/* タスク選択 */
	{"IN",			MONSET_IN       }	/* 入力設定 */
};

/*
 *  バックスペース
 */
static const char backspace[] = "\b \b";

/*
 *  タスクモード表示テーブル
 */
#define	TSTATE_LEN		11

static char const task_state[6][TSTATE_LEN] = {
	"RUNNING",						/* 実行中 */
	"RUNNABLE",						/* 実行できる状態 */
	"WAITING",						/* 待ち状態 */
	"SUSPENDED",					/* 強制待ち状態 */
	"WSUSPENDED",					/* 二重待ち状態 */
	"DORMANT"						/* 休止状態 */
};

/*
 *  ロギングモード表示テーブル
 */
static char const log_mode_name[8][12] = {
	"LOG_EMERG",					/* シャットダウンに値するエラー */
	"LOG_ALERT",
	"LOG_CRIT",
	"LOG_ERROR",					/* システムエラー */
	"LOG_WARNING",					/* 警告メッセージ */
	"LOG_NOTICE",
	"LOG_INFO",
	"LOG_DEBUG"						/* デバッグ用メッセージ */
};

/*
 * モニタで使用されるデータ領域
 */
static ID      current_tskid;			/* モニタが対象とするタスク */
static char    mon_command[MAX_COMMAND_LENGTH+1];
static char    mon_datatype;			/* モニタが指定するデータタイプのデフォルト値 */
static ulong_t mon_address;				/* モニタが指定するアドレスのデフォルト値 */
static int_t   mon_mode;				/* モニタが指定するコマンドモード */
static uint_t  mon_logmask;				/* モニタが指定する記録／出力すべき重要度 */
static uint_t  mon_lowmask;				/* モニタが指定する低レベル出力すべき重要度 */
static ID      mon_default_portid;		/* モニタのデフォルトポートID */
static FILE    *mon_infile;				/* モニタの入力ファイル */
static COMMAND_LINK *pheadcmd;

ID             mon_portid;				/* モニタのデバイスポートID */

/*
 *  モニタタスク
 */
void monitor(intptr_t exinf)
{
	int    no, argc, cno, len;
	char   *argv[NUM_ARGC], c;

	/* モニタで使用するデータの初期化 */

	mon_portid   = mon_default_portid = (ID)exinf;
	mon_datatype = DATA_BYTE;
	mon_logmask  = LOG_NOTICE;
	mon_lowmask  = LOG_EMERG;
	current_tskid = MONTASK;
	mon_infile   = stdin;

	if(mon_portid != CONSOLE_PORTID)
		SVC_PERROR(serial_opn_por(mon_portid));
#ifdef NEED_MONITOR
	if(!need_monitor())
		ext_tsk();
#endif	/* NEED_MONITOR */
#ifndef NOINIT_STDIO_MONITOR
	_setup_stdio(&mon_portid);
#endif	/* NOTSET_STDIO_MONITOR */
#ifdef MONITOR_DELAY
	dly_tsk(MONITOR_DELAY);
#endif	/* MONITOR_DELAY */
	printf(banner,
		(TMONITOR_PRVER >> 12) & 0x0f,
		(TMONITOR_PRVER >> 4) & 0xff,
		TMONITOR_PRVER & 0x0f);
	SVC_PERROR(syslog_msk_log(LOG_UPTO(mon_logmask), LOG_UPTO(mon_lowmask)));
	SVC_PERROR(serial_ctl_por(mon_portid, (IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	/* モニタのメインのデスパッチ */

	do{
		prompt(current_tskid);
		c = monitor_getstring(mon_command, &len);
		/*
		 *  引数の設定
		 */
		for(no = argc = cno = 0 ; argc < 16 && no < len ; no++){
			if(test_next_char(mon_command[no])){
				mon_command[no] = 0;
				cno = 0;
			}
			else if(no < len){
				if(cno == 0)
					argv[argc++] = &mon_command[no];
				cno++;
			}
		}
		mon_command[no] = 0;

		putecho('\n');
		no = dispatch_command(argc, argv);
		fflush(NULL);
	}while(c != KEY_EXT);
	printf("exit monitor !!\n");
}

/*
 *  モニタのプロンプトの表示
 *
 *  指定のタスクIDがモニタ以外の場合は
 *  タスクの状態によってプロンプトの表示内容を変更する．
 */
static void
prompt(ID tskid)
{
	T_RTSK rtsk;

	if(tskid == MONTASK)
		printf("mon");
	else{
		ref_tsk(tskid, &rtsk);
		printf("%03d(%s)", tskid, task_state[get_taskstate(rtsk.tskstat)]);
	}
	putchar('>');
	fflush(NULL);
}

/*
 *  コマンドのデスパッチ
 */
static int_t
dispatch_command(int argc, char **argv)
{
	COMMAND_LINK *pcmd = (COMMAND_LINK *)standard_command;
	const COMMAND_INFO *pscmd;
	int no, sno, count;

	if(argc > 0){
		count = sizeof(standard_command) / sizeof(COMMAND_LINK);
		for(no = 0 ; no < count ; no++, pcmd++){
			if(compare_word(pcmd->command, argv[0], mon_mode)){
				if(pcmd->num_command == 0){
					if(pcmd->func != NULL){
						pcmd->func(argc-1, &argv[1]);
						return no;
					}
					else
						return -1;
				}
				for(sno = 0, pscmd = pcmd->pcinfo ; sno < pcmd->num_command; sno++, pscmd++){
					if(compare_word(pscmd->command, argv[1], mon_mode)){
						pscmd->func(argc-1, &argv[1]);
						return sno;
					}
				}
			}
		}
		pcmd = pheadcmd;
		while(pcmd){
			if(compare_word(pcmd->command, argv[0], mon_mode)){
				for(sno = 0, pscmd = pcmd->pcinfo ; sno < pcmd->num_command; sno++, pscmd++){
					if(compare_word(pscmd->command, argv[1], mon_mode)){
						pscmd->func(argc-1, &argv[1]);
						return sno;
					}
				}
			}
			pcmd = pcmd->pcnext;
		}
	}
	return -1;
}

/*
 *  コマンドセットアップ
 */
int
setup_command(COMMAND_LINK *pcmd)
{
	COMMAND_LINK *pbcmd = pheadcmd;

	if(pcmd == NULL)
		return -1;
	pcmd->pcnext = NULL;
	if(pbcmd == NULL){
		pheadcmd = pcmd;
		return 0;
	}
	while(pbcmd != NULL){
		if(pbcmd == pcmd)
			return -1;
		else if(pbcmd->pcnext == NULL)
			break;
		pbcmd = pbcmd->pcnext;
	}
	pbcmd->pcnext = pcmd;
	return 0;
}

/*
 *  表示コマンド
 */
static int_t
display_command(int argc, char **argv)
{
	char     cmd=0;
	T_RTST   rtst;
	char     b;
	uint16_t h;
	ulong_t  w, value1, value2;
	int      no, count;

	count = sizeof(mon_display) / sizeof(struct SUBCOMMAND_TABLE);
	if(argc > 0){
		for(no = 0 ; no < count ; no++){
			if(compare_word(mon_display[no].subcommand, argv[0], 0)){
				cmd = mon_display[no].type;
				break;
			}
		}
	}
	switch(cmd){
	default:						/* デフォルト */
		cmd = mon_datatype;
	case MONDISPLAY_BYTE:			/* バイト単位メモリ表示 */
	case MONDISPLAY_HALF:			/* ハーフ単位メモリ表示 */
	case MONDISPLAY_WORD:			/* ワード単位メモリ表示 */
		value2 = 128;
		if(argc < 2 || !get_value(argv[1], &value1, HEX_VALUE))
			value1 = mon_address;
		value1 = MonAlignAddress(value1);
		if(argc > 2){
			if(*argv[2] == '+'){
				if(!get_value(argv[2]+1, &value2, HEX_VALUE))
					value2 = 128;
			}
			else{
				if(!get_value(argv[2], &value2, HEX_VALUE))
					value2 = value1 + 128;
				value2 -= value1;
			}
		}
		mon_datatype = cmd;
		while((W)value2 > 0){
			printf("%08lx  ", (long)value1);
			for(no = 0 ; no < 16 ; no += mon_datatype){
				if(no == 8)
					putchar(' ');
				switch(mon_datatype){
				case DATA_HALF:
					if(MemoryRead(value1+no, (intptr_t)&h, 2) == 0)
						h = -1;
					printf("%04x  ", h);
					break;
				case DATA_WORD:
					if(MemoryRead(value1+no, (intptr_t)&w, 4) == 0)
						w = -1;
					printf("%08lx    ", (long)w);
					break;
				default:
					if(MemoryRead(value1+no, (intptr_t)&b, 1) == 0)
						b = -1;
					printf("%02x ", b);
					break;
				}
			}
			if(getMemoryType(value1+no, 0) == MEMORY_AREA){
				for(no = 0 ; no < 16 ; no++){
					if(MemoryRead(value1+no, (intptr_t)&b, 1)){
						if(b < ' ' || b >= 0xE0)
							b = '.';
						else if(b >= 127 && b < 0xA0)
							b = '.';
					}
					else
						b = '.';
					putchar(b);
				}
			}
			putchar('\n');
			value1 += 16;
			value2 -= 16;
			tslp_tsk(50);
			if(monitor_break())
				value2 = 0;
		}
		mon_address = value1;
		break;
	case MONDISPLAY_TASK:			/* タスク状態表示 */
		printf("cur id  pri state      pc       stack    inistack inisize\n");
		for(no = 0 ; no < tmax_tskid ; no++){
			ref_tst(no+TMIN_TSKID, &rtst);
			if(current_tskid == (no+TMIN_TSKID))
				printf(" * ");
			else
				printf("   ");
			if(MONTASK == (no+TMIN_TSKID))
				printf(" mon");
			else
				printf(" %03d", no+TMIN_TSKID);
			printf(" %3d ", rtst.tskpri);
			value1 = get_taskstate(rtst.tskstat);
			for(count = 0 ; count < TSTATE_LEN-1 ; count++){
				if(task_state[value1][count] == 0)
					putchar(' ');
				else
					putchar(task_state[value1][count]);
			}
			if(rtst.tskstat == TTS_RUN)
				printf("         ");
			else
				printf(" %08lx", (unsigned long)rtst.tskpc);
			printf(" %08lx %08lx %5ld\n", (unsigned long)rtst.tsksp, (unsigned long)rtst.inistk, (unsigned long)rtst.inistksz);
		}
		putchar('\n');
		break;
	case MONDISPLAY_REG:			/* レジスタの表示 */
		if(current_tskid != MONTASK)
			display_registers(current_tskid);
		break;
	}
	return 0;
}

/*
 *  設定コマンド
 */
static int_t
set_command(int argc, char **argv)
{
	char     cmd=0;
	char     b;
	uint16_t h;
	ulong_t  w, value1, value2;
	int      no, count;
	bool_t   cont;

	count = sizeof(mon_set) / sizeof(struct SUBCOMMAND_TABLE);
	if(argc > 0){
		for(no = 0 ; no < count ; no++){
			if(compare_word(mon_set[no].subcommand, argv[0], 0)){
				cmd = mon_set[no].type;
				break;
			}
		}
	}
	switch(cmd){
	default:						/* デフォルト */
		cmd = mon_datatype;
	case MONSET_BYTE:				/* バイト単位メモリ設定 */
	case MONSET_HALF:				/* ハーフ単位メモリ設定 */
	case MONSET_WORD:				/* ワード単位メモリ設定 */
		if(argc < 2 || !get_value(argv[1], &value1, HEX_VALUE))
			value1 = mon_address;
		value1 = MonAlignAddress(value1);
		mon_datatype = cmd;
		do{
			printf("  %08lx", (unsigned long)value1);
			switch(mon_datatype){
			case DATA_HALF:
				if(MemoryRead(value1, (intptr_t)&h, 2) == 0)
					h = -1;
				printf(" %04x =", h);
				break;
			case DATA_WORD:
				if(MemoryRead(value1, (intptr_t)&w, 4) == 0)
					w = -1;
				printf(" %08lx =", (unsigned long)w);
				break;
			default:
				if(MemoryRead(value1, (intptr_t)&b, 1) == 0)
					b = -1;
				printf(" %02x =", b);
				break;
			}
			monitor_getstring(mon_command, &count);
			if(get_value(mon_command, &value2, HEX_VALUE)){
				switch(mon_datatype){
				case DATA_HALF:
					h = value2;
					MemoryWrite(value1, (intptr_t)&h, 2);
					if(MemoryRead(value1, (intptr_t)&h, 2) == 0)
						h = -1;
					printf(" %04x\n", h);
					break;
				case DATA_WORD:
					w = value2;
					MemoryWrite(value1, (intptr_t)&w, 4);
					if(MemoryRead(value1, (intptr_t)&w, 4) == 0)
						w = -1;
					printf(" %08lx\n", (unsigned long)w);
					break;
				default:
					b = value2;
					MemoryWrite(value1, (intptr_t)&b, 1);
					if(MemoryRead(value1, (intptr_t)&b, 1) == 0)
						b = -1;
					printf(" %02x\n", b);
					break;
				}
				value1 += mon_datatype;
				cont = true;
			}
			else
				cont = false;
		}while(cont);
		putchar('\n');
		mon_address = value1;
		break;
	case MONSET_COMMAND:			/* コマンドモード設定 */
		if(argc > 1 && get_value(argv[1], &value1, DEC_VALUE)){
			if(value1 == 1 || value1 == 2){
				printf("  set %d command(s) mode !\n", (int)value1);
				mon_mode = value1;
			}
		}
		printf("  set command mode=%d word(s) !\n", mon_mode);
		break;
	case MONSET_SERIAL:				/* シリアル設定 */
		if(argc > 1 && get_value(argv[1], &value1, DEC_VALUE)){
			if(value1 > 0 && mon_portid != value1){
				if(mon_portid != mon_default_portid && mon_portid != CONSOLE_PORTID)
					SVC_PERROR(serial_cls_por(mon_portid));
				mon_portid = value1;
				if(mon_portid != mon_default_portid && mon_portid != CONSOLE_PORTID)
					SVC_PERROR(serial_opn_por(mon_portid));
				SVC_PERROR(serial_ctl_por(mon_portid, (IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));
				printf(banner,
					(TMONITOR_PRVER >> 12) & 0x0f,
					(TMONITOR_PRVER >> 4) & 0xff,
					TMONITOR_PRVER & 0x0f);
			}
		}
		printf("  set serial port id=%d !\n", mon_portid);
		break;
	case MONSET_TASK:				/* タスク選択 */
		if(argc > 1 && get_value(argv[1], &value1, DEC_VALUE)){
			if(value1 >= TMIN_TSKID && value1 < (TMIN_TSKID+tmax_tskid))
				current_tskid = value1;
			else
				current_tskid = MONTASK;
		}
		else
			current_tskid = MONTASK;
		break;
	case MONSET_IN:					/* 入力設定 */
		value1 = 0;
		if(argc > 1)
			get_value(argv[1], &value1, DEC_VALUE);
		if(value1 == 1)
			mon_infile = stdout;
		else if(value1 == 2)
			mon_infile = stderr;
		else
			mon_infile = stdin;
		break;
	}
	return 0;
}

/*
 *  ヘルプコマンド
 */
static int_t
help_command(int argc, char **argv)
{
	COMMAND_LINK *pcmd = (COMMAND_LINK *)standard_command;
	int no, count;

	count = sizeof(standard_command) / sizeof(COMMAND_LINK);
	for(no = 0 ; no < count ; no++, pcmd++){
		if(argc > 0){
			if(compare_word(pcmd->command, argv[0], 0)){
				printf(pcmd->help);
				return no;
			}
		}
		else
			printf(pcmd->help);
	}
	pcmd = pheadcmd;
	while(pcmd){
		if(argc > 0){
			if(compare_word(pcmd->command, argv[0], 0)){
				printf(pcmd->help);
				return no;
			}
		}
		else
			printf(pcmd->help);
		pcmd = pcmd->pcnext;
	}
	return 0;
}

/*
 *  タスクコマンド、起動要求(act_tsk)
 */
static int_t
task_activate(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = act_tsk(current_tskid);
	printf("  execute act_tsk(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、強制終了(ter_tsk)
 */
static int_t
task_terminate(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = ter_tsk(current_tskid);
	printf("  execute ter_tsk(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、待ち要求(sus_tsk)
 */
static int_t
task_suspend(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = sus_tsk(current_tskid);
	printf("  execute sus_tsk(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、待ち再開(rsm_tsk)
 */
static int_t
task_resume(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = rsm_tsk(current_tskid);
	printf("  execute rsm_tsk(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、待ち解除(rel_wai)
 */
static int_t
task_release(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = rel_wai(current_tskid);
	printf("  execute rel_wai(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、タスク起床(wup_tsk)
 */
static int_t
task_wakeup(int argc, char **argv)
{
	ER      ercd = E_OK;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	ercd = wup_tsk(current_tskid);
	printf("  execute wup_tsk(%d) :: result = %s !\n", current_tskid, itron_strerror(ercd));
	tslp_tsk(100);				/* スイッチング */
	return 0;
}

/*
 *  タスクコマンド、優先度変更(chg_pri)
 */
static int_t
task_priority(int argc, char **argv)
{
	ER      ercd = E_OK;
	ulong_t w;

	if(current_tskid == MONTASK){	/* モニタ自体の制御はできない */
		printf(" Can't control the monitor!!\n");
		return 0;
	}
	if(argc > 1 && get_value(argv[1], &w, DEC_VALUE)){
		ercd = chg_pri(current_tskid, w);
		printf("  execute chg_pri(%d, %d) :: result = %s !\n", current_tskid, (int)w, itron_strerror(ercd));
		tslp_tsk(100);				/* スイッチング */
		return 0;
	}
	return -1;
}

/*
 * ログコマンド、ロギングモード設定
 */
static int_t
log_mode(int argc, char **argv)
{
	ulong_t	value1, value2;

	if(argc < 2 || !get_value(argv[1], &value1, DEC_VALUE))
		value1 = mon_logmask;
	if(argc < 3 || !get_value(argv[2], &value2, DEC_VALUE))
		value2 = mon_lowmask;
	if(value1 <= LOG_DEBUG)
		mon_logmask = value1;
	if(value2 <= LOG_DEBUG)
		mon_lowmask = value2;
	syslog_msk_log(LOG_UPTO(mon_logmask), LOG_UPTO(mon_lowmask));
	printf("  set logmask=%s lowmask=%s !\n", log_mode_name[mon_logmask], log_mode_name[mon_lowmask]);
	return 0;
}

/*
 * ログコマンド、タスクモニタ設定
 */
static int_t
log_task(int argc, char **argv)
{
	T_TPRM  loglist;
	ulong_t value1, value2, value3 = 0;
	ulong_t tcount, ttime;
	ulong_t sno = MONTASK;
	ulong_t xno = MONTASK+NUM_LOG_DISP;
	int     no, count;

	if(argc < 2 || !get_value(argv[1], &value1, DEC_VALUE))
		value1 = 0;
	else if(argc < 3 || !get_value(argv[2], &value3, DEC_VALUE))
		value3 = 0;
	if(value1 < 100)
		value1 = 0;
	value2 = 0;
	do{
		count = get_tsklog(&loglist);
		if(value2 == 0){		/* はじめの１回 */
			value2 =loglist.currtime;
			if(xno > count){
				int i = xno - count;
				if(i > MONTASK)
					i = MONTASK;
				sno -= xno - count;
				xno -= xno - count;
			}
			printf("-- time --");
			for(no = 1 ; no <= xno ; no++){
				if(no > sno)
					printf(" --id=%04u --", no);
			}
			printf(" -- others --  -vacancy-\n");
		}
		value2 += value1;
		if(value1 == 0)
			printf("%010lu\n", (unsigned long)loglist.pervtime);
		printf("%010lu", (unsigned long)loglist.currtime);
		tcount = ttime = 0;
		for(no = 1 ; no <= count ; no++){
			if(no > sno && no <= xno){
				printf(" %04u %05lu.%01u", loglist.tlog[no].runcount, (unsigned long)(loglist.tlog[no].runtimes/10),
						(int)(loglist.tlog[no].runtimes%10));
			}
			else{
				tcount += loglist.tlog[no].runcount;
				ttime += loglist.tlog[no].runtimes;
			}
		}
		printf(" %04u %05lu.%01u", (int)tcount, (unsigned long)(ttime/10), (int)(ttime%10));
		printf("  %07lu.%01u\n", (unsigned long)(loglist.tlog[0].runtimes/10), (int)(loglist.tlog[0].runtimes%10));
		if(value1){
			tslp_tsk(value2 - loglist.currtime);
			if(monitor_break())
				value1 = 0;
		}
		value3--;
	}while(value1 && value3 != 0);
	return 0;
}

/*
 * ログコマンド、ポートモニタ設定
 */
static int_t
log_port(int argc, char **argv)
{
	T_PCHK  *p;
	ulong_t type, address;
	ulong_t value1, value2;

	value2 = NUM_PCHK-1;
	if(argc > 1 && get_value(argv[1], &value1, DEC_VALUE)){
		if((p = get_device_log(value1)) != 0){
			value2 = value1;
			if(argc < 3 || !get_value(argv[2], &type, DEC_VALUE)){
				type = p->logtype;
			}
			if(type >= LOG_DEBUG)
				type = 0;
			p->logtype = type;
			if(argc < 4 || !get_value(argv[4], &address, HEX_VALUE)){
				address = p->portaddress;
			}
			p->portaddress = address;
		}
	}
	else
		value1 = 0;
	while(value1 <= value2){
		if((p = get_device_log(value1)) != 0){
			if(p->logtype > 0)
				printf("  %02d   %s port=%08lx\n", (int)value1, log_mode_name[p->logtype], (unsigned long)p->portaddress);
			else
				printf("  %02d   OFF      port=%08lx\n", (int)value1, (unsigned long)p->portaddress);
		}
		else
			break;
		value1++;
	}
	return 0;
}


/*
 *  キャラクタの比較
 */
bool_t
compare_word(const char *s, char *d, int_t mode)
{
	char c1, c2;

	if(*d == 0)
		return false;
	while(*s != 0 && *d != 0 && *d != ' ' && *d != ','){
		c2 = *d++;
		if(c2 >= 'a' && c2 <= 'z')
			c2 -= 0x20;
		c1 = *s++;
		if(c1 >= 'a' && c1 <= 'z')
			c1 -= 0x20;
		if(c1 != c2)
			return false;
		if(mode == 1)
			break;
	}
	return true;
}

/*
 *  タスク状態番号を取り出す
 */
static uint_t
get_taskstate(STAT tskstat)
{
	uint_t no;

	if(tskstat == TTS_RUN)
		no = 0;
	else if(tskstat == TTS_RDY)
		no = 1;
	else if(tskstat == TTS_WAI)
		no = 2;
	else if(tskstat == TTS_SUS)
		no = 3;
	else if(tskstat == TTS_DMT)
		no = 5;
	else
		no = 4;
	return no;
}

/*
 *  モニタのコマンド読み込み文
 */
char
monitor_getstring(char *s, int *len)
{
	char c;
	int  no = 0;

	*len = 0;
	do{
		c = getc(mon_infile);
		if(c >= ' ' && no < MAX_COMMAND_LENGTH){
			putecho((int)c);
			s[no++] = c;
		}
		else if((c == KEY_BS || c == KEY_DEL) && no > 0){
			printecho(backspace);
			no--;
		}
	}while(c != KEY_NL && c != KEY_CR && c != KEY_EXT);
	s[no] = 0;
	*len = no;
	return c;
}

/*
 *  モニタコマンドブレークの判定
 */
bool_t
monitor_break(void)
{
	T_SERIAL_RPOR k_rpor;

	if(mon_infile != stdin)
		return false;
	serial_ref_por(mon_portid, &k_rpor);
	if(k_rpor.reacnt > 0){
		getc(mon_infile);
		return true;
	}
	else
		return false;
}

/*
 *  モニタ入力ファイルの切り替え
 */
void *
monitor_infile(void *file)
{
	void *ofile = mon_infile;

	if(file != NULL)
		mon_infile = file;
	return ofile;
}

