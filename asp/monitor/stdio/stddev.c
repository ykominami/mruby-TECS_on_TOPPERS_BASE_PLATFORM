/*
 *  TOPPERS/JSP/ASP/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Just Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: stddev.c,v 1.4 2017/05/27 18:34:04 roi Exp $
 */

/* 
 *  TOPPERS用タスクモニタCPU依存プログラム．
 *
 */

#include <itron.h> 
#include <stdio.h>
#include <string.h>
#include "syssvc/serial.h"
#include "monitor.h"


#ifdef _SMALL_STDIO_
/******************************************************************************
 * 縮小版標準入出力の設定
 ******************************************************************************/

FILE _iob[4];
static ID   dummyid = CONSOLE_PORTID;
static ID * stdport = &dummyid;

/*
 *  モニタの１文字入力文
 */
static int
local_getc(FILE *st)
{
	char buf[2];

	serial_rea_dat(*stdport, buf, 1);
	return (int)buf[0];
}

/*
 *  モニタの文字列入力文
 */
static int
local_gets(FILE *st, unsigned int len, char *s)
{
	return serial_rea_dat(*stdport, s, len);
}

/*
 *  モニタの１文字出力文
 */
static void
local_putc(FILE *st, int c)
{
	char buf[2];

	buf[0] = c;
	serial_wri_dat(*stdport, buf, 1);
	if(c == '\n'){
		buf[0] = '\r';
		serial_wri_dat(*stdport, buf, 1);
	}
}

/*
 *  モニタの文字列出力文
 */
static int
local_puts(FILE *st, unsigned int len, char *s)
{
	return serial_wri_dat(*stdport, s, len);
}

/*
 *  モニタのデータフラッシュ文
 */
static int
local_flush(FILE *st)
{
	return 0;
}

/*
 *  ローカルなstrlen
 */
static int
local_datacount(const char *s)
{
	int i;

	for(i = 0 ; *s != 0 ; i++)
		s++;
	return i;
}

/*
 *  標準出力関数
 */
int fputc(int c, FILE *st)
{
	if(st == NULL)
		st = stdout;
	st->_func_out(st, c);
	return 1;
}

/*
 *  標準文字列出力関数
 */
int fputs(const char *s, FILE *st)
{
	int status;

	if(st == NULL)
		st = stdout;
	status = st->_func_outs(st, local_datacount(s), (char *)s);
	st->_func_out(st, '\n');
	return status;
}

/*
 *  標準入力関数
 */
int fgetc(FILE *st)
{
	if(st == NULL)
		st = stdin;
	return st->_func_in(st);
}

/*
 *  標準文字列入力関数
 */
int fgets(char *buf, int size, FILE *st)
{
	if(st == NULL)
		st = stdin;
	return st->_func_ins(st, size, buf);
}

/*
 *  標準データ読み出し関数
 */
size_t fread(void *buf, size_t size, size_t count, FILE *st)
{
	int result;
	if(st == NULL)
		st = stdin;
	result = st->_func_ins(st, size*count, buf);
	return result / size;
}

/*
 *  標準データ書き込み関数
 */
size_t fwrite(const void *buf, size_t size, size_t count, FILE *st)
{
	int result;
	if(st == NULL)
		st = stdout;
	result = st->_func_outs(st, size*count, (char *)buf);
	return result / size;
}

/*
 *  標準文字列出力関数
 */
int puts(const char *s)
{
	int status;

	status = stdout->_func_outs(stdout, local_datacount(s), (char *)s);
	stdout->_func_out(stdout, '\n');
	return status;
}

/*
 *  一文字出力プログラム
 */
int putchar(int c)
{
	return fputc(c,stdout);
}

/*
 *  バッファのフラッシュ関数
 */
int fflush(FILE *st)
{
	if(st == NULL)
		st = stdout;
	return st->_func_flush(st);
}

/*
 *  標準入出力用ストリーム取り出し関数
 */
FILE *_get_stdio(int no)
{
	return &_iob[no];
}

/*
 *  標準入出力ストリーム初期化関数
 */
int _set_stdio(int no)
{
	if(no >= 0 && no < 3){
		_iob[no]._file       = -1;
		_iob[no]._func_in    = (int (*)(void*))local_getc;
		_iob[no]._func_ins   = (int (*)(void*, unsigned int, char *))local_gets;
		_iob[no]._func_out   = (void (*)(void*, int))local_putc;
		_iob[no]._func_outs  = (int  (*)(void*, unsigned int, char *))local_puts;
		_iob[no]._func_flush = local_flush;
		_iob[no]._dev        = 0;
		return no;
	}
	else
		return -1;
}

/*
 *  標準入出力初期化関数
 */
void _setup_stdio(signed int *pport)
{
	int i;

	stdport = pport;
	for(i = 0 ; i < 3 ; i++){
		_set_stdio(i);
	}
}

#endif /* _SMALL_STDIO_ */

