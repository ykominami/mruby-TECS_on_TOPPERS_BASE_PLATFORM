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
 *  @(#) $Id: monitor.h,v 2.0 2015/05/29 17:43:21 roi Exp $
 */

#ifndef _MONITOR_H_
#define	_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 
 *  モニタ・サンプルプログラムのヘッダファイル
 */
#include <kernel.h>
#include "syssvc/logtask.h"

/*
 *  タスク優先度の設定
 */
#define MONITOR_PRIORITY	4

/*
 *  ターゲットに依存する可能性のある定数の定義
 */

#ifndef CONSOLE_PORTID
#ifdef LOGTASK_PORTID
#define	CONSOLE_PORTID		LOGTASK_PORTID	/* 文字入力するシリアルポートID */
#else  /* LOGTASK_PORTID */
#define	CONSOLE_PORTID		1				/* 文字入力するシリアルポートID */
#endif /* LOGTASK_PORTID */
#endif /* TASK_PORTID */

/*
 *  モニタのサイズの設定
 */
#if defined(M16C)
#define	MONITOR_STACK_SIZE	512		/* タスクのスタックサイズ */
#elif defined(H8) && !defined(SUPPORT_VLOUME)
#define	MONITOR_STACK_SIZE	768		/* タスクのスタックサイズ */
#else
#define	MONITOR_STACK_SIZE	2046	/* タスクのスタックサイズ */
#endif
#define	MAX_COMMAND_LENGTH	80		/* モニタの最大コマンド長 */
#define	NUM_LOG_DISP		3		/* 最大表示アイテム数 */
#ifndef MONITOR_PORTID
#define MONITOR_PORTID      CONSOLE_PORTID
#endif

/*
 *  バージョン情報
 */
#define	TMONITOR_PRVER	0x1020		/* カーネルのバージョン番号 */

/*
 *  キー割り当ての定義
 */
#define	KEY_BS			(8)			/* バックスペース */
#define	KEY_DEL			(127)		/* 削除 */
#define KEY_NL			(10)		/* 改行1 */
#define	KEY_CR			(13)		/* 改行2 */
#define	KEY_EXT			(1)			/* 終了 */

/*
 *  データタイプ定義
 */
#define	DATA_BYTE		1			/* バイトデータ（１バイト）*/
#define	DATA_HALF		2			/* ハーフデータ（２バイト）*/
#define	DATA_WORD		4			/* ワードデータ（４バイト）*/

/*
 *  領域属性の定義
 */
#define	NONE_AREA		0			/* 領域の割り当てのない領域 */
#define	PORT_AREA		1			/* ハードウェアのポート領域 */
#define	MEMORY_AREA		2			/* メモリ領域　*/

#define	MREAD_ONLY		1			/* 読み込み専用 */
#define	MWRITE_ONLY		2			/* 書き込み専用 */
#define	MREAD_WRITE		(MREAD_ONLY+MWRITE_ONLY)

#ifndef TOPPERS_MACRO_ONLY

/*
 *  コマンドデスパッチ用の構造体定義
 */
typedef struct _COMMAND_INFO {
	const char   *command;			/* コマンド文 */
	int_t        (*func)(int argc, char **argv);	/* 実行関数 */
} COMMAND_INFO;

typedef struct _COMMAND_LINK COMMAND_LINK;
struct _COMMAND_LINK {
	COMMAND_LINK *pcnext;
	int          num_command;
	const char   *command;			/* 主コマンド文 */
	int_t        (*func)(int argc, char **argv);	/* 実行関数 */
	const char   *help;				/* ヘルプ文 */
    const COMMAND_INFO *pcinfo;
};

/*
 *  エコーの設定
 */
#define putecho(a)      putchar(a)
#define printecho(a)    printf(a)

/*
 *  関数のプロトタイプ宣言
 */

extern bool_t  need_monitor(void);
extern void	   monitor(intptr_t exinf);
extern int     setup_command(COMMAND_LINK *pcmd);
extern bool_t  monitor_break(void);
extern bool_t  compare_word(const char *s, char *d, int_t mode);
extern char    getMemoryType(ulong_t address, int_t mode);
extern int_t   MemoryRead(ulong_t address, intptr_t p, int_t type);
extern int_t   MemoryWrite(ulong_t address, intptr_t p, int_t type);
extern ulong_t MonAlignAddress(ulong_t address);
extern int_t   MemoryRead(ulong_t address, intptr_t p, int_t type);
extern int_t   MemoryWrite(ulong_t address, intptr_t p, int_t type);
extern ulong_t get_exception_pc(void * p_excinf);
extern void    display_registers(ID tskid);
extern ulong_t display_assembler(ulong_t pc);

/*
 *  次のキャラクタの判定
 */
Inline bool_t
test_next_char(char c)
{
	if(c == ' ' || c == '\t')
		return true;
	else
		return false;
}

/*
 *  文字列から数字を取り出す
 *  戻り値がFALSEなら値が未設定であることを示す
 */
Inline bool_t
get_value(char *s, ulong_t *v, int_t card)
{
	char c;
	int  no = 0;

	*v = 0;
	if(s[no] == 0)
		return false;
	else if(s[no] == ' ' || s[no] == '\t'
               || s[no] == ',' || s[no] == '.'){
		no++;
		return false;
	}
	else{
		while(s[no]){
			c = s[no];
			no++;
			if(c >= '0' && c <= '9')
				c -= '0';
			else if(c >= 'A' && c <= 'Z')
				c -= 'A' - 10;
			else if(c >= 'a' && c <= 'z')
				c -= 'a' - 10;
			else
				break;
			*v = *v * card + c;
		}
		return true;
	}
}

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif

#endif /* _MONITOR_H_ */

