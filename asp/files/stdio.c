/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の (1)〜(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
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
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: stdio.c,v 1.3 2016/05/08 08:08:32 roi Exp $
 */
/*
 * このプログラムはITRON専用のTYPE3ソフトウェアである。
 * POSIXのファイル関数を供給する。
 * 擬似的なストレージ関数郡であり、標準ライブラリィと分けての使用が望ましい。
 * 1. _stdfile_init            ファイルインデックスの初期化を行う
 * 2. open                     ファイルのオープン(特別な専用FDを発行する)
 * 3. close                    ファイルのクローズ
 * 4. fstat                    ファイルのステートの取り出し
 * 5. fseek                    ファイルのシーク
 * 6. read                     ファイルからのデータ読み出し
 * 7. write                    ファイルからのデータ書き込み
 * 8. mmap                     ファイルデータのマップ化
 * 9. munmap                   ファイルデータのアンマップ化
 */

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "kernel_cfg.h"
#include "fcntl.h"
#include "storagedevice.h"

#ifndef NUM_FILEIDX
#define NUM_FILEIDX	128
#endif


static struct StdFileIndex stdfile[NUM_FILEIDX];


void stdfile_init(intptr_t exinf)
{
	unsigned int idx;

	for(idx = 0 ; idx < NUM_FILEIDX ; idx++){
		stdfile[idx].sdmdev = 0;
		stdfile[idx].sdmfd = -1;
	}
}

/*
 *  FDチェック関数
 */
static struct StdFileIndex *fdcheck(int fd)
{
	struct StdFileIndex *pf;

	if(fd <= 0 || fd > NUM_FILEIDX)
		return 0;
	pf = &stdfile[fd-1];
	if(pf->sdmfd == -1)
		return 0;
	else
		return pf;
}

/*
 *  標準ファイルインターフェイス関数(open)
 */
int open(const char *pathname, int flags)
{
	StorageDevice_t         *psdev;
	StorageDeviceFileFunc_t *pff;
	int             devno, id, no;

	if((devno = SDMGetDeviceNo(&pathname)) < 0)
		return -1;
	if((psdev = SDMGetStorageDevice(devno)) == 0)
		return -1;
	pff = psdev->pdevff;
	if(pff != 0 && pff->_sdevff_open == 0)
		return -1;

	wai_sem(SEM_STDFILE);
	for(no = 0 ; no < NUM_FILEIDX ; no++){
		if(stdfile[no].sdmfd == -1)
			break;
	}
	if(no >= NUM_FILEIDX){
		sig_sem(SEM_STDFILE);
		return -1;
	}

	id = pff->_sdevff_open(devno, pathname, flags);
	if(id < 0){
		sig_sem(SEM_STDFILE);
		return id;
	}
	stdfile[no].sdmfd  = id;
	stdfile[no].sdmdev = psdev;
	sig_sem(SEM_STDFILE);
	return no+1;
}

/*
 *  標準ファイルインターフェイス関数(close)
 */
int close(int fd)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;
	int             result;

	if((ps = fdcheck(fd)) == 0)
		return -1;
	wai_sem(SEM_STDFILE);
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_close != 0)
		result = psdev->pdevff->_sdevff_close(ps->sdmfd);
	else
		result = -1;
	ps->sdmfd = -1;
	sig_sem(SEM_STDFILE);
	return result;
}

/*
 *  標準ファイルインターフェイス関数(fstat)
 */
int fstat(int fd, struct stat *buf)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;

	if((ps = fdcheck(fd)) == 0)
		return -1;
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_fstat != 0)
		return psdev->pdevff->_sdevff_fstat(ps->sdmfd, buf);
	else
		return -1;
}

/*
 *  標準ファイルインターフェイス関数(lseek)
 */
off_t lseek(int fd, off_t offset, int whence)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;
	int                 result;

	if((ps = fdcheck(fd)) == 0)
		return -1;
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_lseek != 0)
		return psdev->pdevff->_sdevff_lseek(ps->sdmfd, offset, whence, &result);
	else
		return -1;
}

/*
 *  標準ファイルインターフェイス関数(read)
 */
long read(int fd, void *buf, long count)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;
	int                 result;

	if((ps = fdcheck(fd)) == 0)
		return -1;
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_read != 0)
		return psdev->pdevff->_sdevff_read(ps->sdmfd, buf, count, &result);
	else
		return 0;
}

/*
 *  標準ファイルインターフェイス関数(write)
 */
long write(int fd, const void *buf, long count)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;
	int                 result;

	if((ps = fdcheck(fd)) == 0)
		return -1;
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_write != 0)
		return psdev->pdevff->_sdevff_write(ps->sdmfd, buf, count, &result);
	else
		return 0;
}

/*
 *  ファイルマップ関数(mmap)
 */
void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	struct StdFileIndex *ps;
	StorageDevice_t     *psdev;

	if((ps = fdcheck(fd)) == 0)
		return (void*)-1;
	psdev = ps->sdmdev;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_mmap != 0)
		return psdev->pdevff->_sdevff_mmap(start, length, prot, flags, ps->sdmfd, offset);
	else
		return (void*)-1;
}

/*
 *  ファイルアンマップ関数(未処理)
 */
int munmap(void *start, size_t length)
{
	return 0;
}

/*
 *  標入出力用１文字入力文
 */
static int local_getc(void *st)
{
	StorageDevice_t     *psdev;
	int  result, res = 0;
	char buf[2];

	if(st == 0 && ((FILE *)st)->_file < 0)
		return -1;
	if((psdev = (StorageDevice_t *)((FILE*)st)->_dev) == 0)
		return -1;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_read != 0){
		result = psdev->pdevff->_sdevff_read(((FILE *)st)->_file, buf, 1, &res);
		if(res < 0)
			((FILE *)st)->_flags |= __SERR;
		if(result == 1)
			return buf[0];
		else
			return -1;
	}
	else
		return -1;
}

/*
 *  標入出力用１文字入力文
 */
static int local_gets(void *st, unsigned int len, char *s)
{
	StorageDevice_t     *psdev;
	int result, res = 0;

	if(st == 0 && ((FILE *)st)->_file < 0)
		return -1;
	if((psdev = (StorageDevice_t *)((FILE*)st)->_dev) == 0)
		return -1;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_read != 0){
		result = psdev->pdevff->_sdevff_read(((FILE *)st)->_file, s, len, &res);
		if(res < 0)
			((FILE *)st)->_flags |= __SERR;
		return result;
	}
	else
		return 0;
}

/*
 *  標入出力用文字出力文
 */
static void local_putc(void *st, int c)
{
	StorageDevice_t     *psdev;
	int  res = 0;
	char buf[2];

	if(st == 0 && ((FILE *)st)->_file < 0)
		return;
	if((psdev = (StorageDevice_t *)((FILE*)st)->_dev) == 0)
		return;
	buf[0] = c;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_write != 0){
		psdev->pdevff->_sdevff_write(((FILE *)st)->_file, buf, 1, &res);
		if(res < 0)
			((FILE *)st)->_flags |= __SERR;
	}
}

/*
 *  標入出力用文字列出力文
 */
static int local_puts(void *st, unsigned int len, char *s)
{
	StorageDevice_t     *psdev;
	int result, res = 0;

	if(st == 0 && ((FILE *)st)->_file < 0)
		return -1;
	if((psdev = (StorageDevice_t *)((FILE*)st)->_dev) == 0)
		return -1;
	if(psdev->pdevff != 0 && psdev->pdevff->_sdevff_write != 0){
		result = psdev->pdevff->_sdevff_write(((FILE *)st)->_file, s, len, &res);
		if(res < 0)
			((FILE *)st)->_flags |= __SERR;
		return result;
	}
	else
		return 0;
}

/*
 *  標入出力用データフラッシュ文
 */
static int local_flush(FILE *st)
{
	return 0;
}

/*
 *  通常ファイルの設定
 */
int _setup_file(FILE *st, StorageDevice_t *pd, int lid)
{
	if(st == NULL)
		return 0;
	st->_file       = lid;
	st->_func_in    = local_getc;
	st->_func_ins   = local_gets;
	st->_func_out   = local_putc;
	st->_func_outs  = local_puts;
	st->_func_flush = local_flush;
	st->_dev        = pd;
	return 1;
}

/*
 *  標準入出力ファイルの設定
 */
int _setupstd_file(FILE *st, int fd)
{
	struct StdFileIndex *ps;

	if(st == NULL)
		return 0;

	st->_func_in    = local_getc;
	st->_func_ins   = local_gets;
	st->_func_out   = local_putc;
	st->_func_outs  = local_puts;
	st->_func_flush = local_flush;

	if((ps = fdcheck(fd)) == 0){
		st->_file = -1;
		st->_dev  = 0;
		return 0;
	}
	else{
		st->_file       = ps->sdmfd;
		st->_dev        = ps->sdmdev;
		return 1;
	}
}

