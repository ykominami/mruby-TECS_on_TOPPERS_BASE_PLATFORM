/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2012 by GT Development Center RICOH COMPANY,LTD. JAPAN
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
 *  @(#) $Id: fffcntl.c,v 1.2 2016/05/06 08:07:10 roi Exp $
 */

/*
 * FatFS用POSIX変換用関数
 * 1. ff_opendir               FatFSファイルのオープンディレクトリ
 * 2. ff_closedir              FatFSファイルのクローズディレクトリ
 * 3. ff_readdir               FatFSファイルのディレクトリエントリ読み出し
 * 4. ff_mkdir                 FatFSファイルのディレクトリ作成
 * 5. ff_unlink                FatFSファイルのディレクトリまたはファイル削除
 * 6. ff_rename                FatFSファイルのリネーム
 * 7. ff_chmod                 FatFSファイルのチェンジモード
 * 8. ff_stat                  FatFSファイルのダイレクトステートの取り出し
 * 9. ff_statfs                FatFSファイルのシステムステータスの取り出し
 * 10.ff_open                  FatFSファイルのオープン
 * 11.ff_close                 FatFSファイルのクローズ
 * 12.ff_fstat                 FatFSファイルのステートの取り出し
 * 13.ff_fseek                 FatFSファイルのシーク
 * 14.ff_read                  FatFSファイルからのデータ読み出し
 * 15.ff_write                 FatFSファイルへのデータ書き込み
 *
 */

#include <stdlib.h>
#include <string.h>
#include "storagedevice.h"
#include "ff.h"

#ifndef NUM_DIRNEST
#define NUM_DIRNEST 4
#endif
#ifndef NUM_FIL
#define NUM_FIL     16
#endif

#define	S_IRUSR	0000400	/* read permission, owner */
#define	S_IWUSR	0000200	/* write permission, owner */
#define	S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRGRP	0000040	/* read permission, group */
#define	S_IWGRP	0000020	/* write permission, grougroup */
#define	S_IXGRP 0000010/* execute/search permission, group */
#define	S_IROTH	0000004	/* read permission, other */
#define	S_IWOTH	0000002	/* write permission, other */
#define	S_IXOTH 0000001/* execute/search permission, other */

#define	FF_EINVAL 22	/* Invalid argument */

static void *ff_opendir(const char *pathname);
static int ff_closedir(void *dir);
static int ff_readdir(void *dir, void *pdirent);
static int ff_mkdir(const char *pathname);
static int ff_unlink(const char *pathname);
static int ff_rename(const char *oname, const char *nname);
static int ff_chmod(const char *name, int mode);
static int ff_stat(const char *name, struct stat *buf);
static int ff_statfs(const char *name, void *status);
static int ff_open(int devno, const char *pathname, int flags);
static int ff_close(int fd);
static int ff_fstat(int fd, struct stat *buf);
static off_t ff_lseek(int fd, off_t offset, int whence, int *res);
static long ff_read(int fd, void *buf, long count, int *res);
static long ff_write(int fd, const void *buf, long count, int *res);


const StorageDeviceFileFunc_t fatfsSDeviceFileFunc = {
	ff_opendir,
	ff_closedir,
	ff_readdir,
	ff_mkdir,
	ff_unlink,
	ff_unlink,
	ff_rename,
	ff_chmod,
	ff_stat,
	ff_statfs,
	ff_open,
	ff_close,
	ff_fstat,
	ff_lseek,
	ff_read,
	ff_write,
	0
};

typedef struct DIRPACK {
	DIR      dir;
	FILINFO  finfo;
} DIRP;

static FIL   FileObj[NUM_FIL];
static DIRP  dirobj[NUM_DIRNEST];
static UH    long_file_name[128];
static int   opendircount = 0;
static char  sddevname[256];
static int   ff_errno;

/*
 *  FatFsファイルディレクトリオープン
 */
static void *ff_opendir(const char *pathname)
{
	DIRP *dirp;
	FRESULT result;

	if(opendircount >= NUM_DIRNEST)
		return NULL;
	dirp = &dirobj[opendircount++];
	if((result = f_opendir(&dirp->dir, pathname)) != FR_OK){
		ff_errno = result;
		opendircount--;
		return NULL;
	}
	else
		return dirp;
}

/*
 *  FatFsファイルディレクトリクローズ
 */
static int ff_closedir(void *dir)
{
	if(opendircount > 0)
		opendircount--;
	return 0;
}

/*
 *  FatFsファイルディレクトリ読み込み
 */
static int ff_readdir(void *dir, void *pdirent)
{
	struct dirent2 *pd = pdirent;
	DIRP   *dirp = dir;
	FRESULT result;

#if _USE_LFN != 0
	dirp->dir.lfn = long_file_name;
#endif
	if((result = f_readdir(&dirp->dir, &dirp->finfo)) != FR_OK){
		ff_errno = result;
		return 0;
	}
	if(dirp->dir.sect == 0)
		return 0;
	pd->d_fsize = dirp->finfo.fsize;
	pd->d_date  = dirp->finfo.fdate;
	pd->d_time  = dirp->finfo.ftime;
	pd->d_type  = dirp->finfo.fattrib;
#if _USE_LFN != 0
	if(dirp->dir.lfn_idx != 0xffff){
		UH *pw, data;
		UB *p, *pe;
		pw = dirp->dir.lfn;
		p  = pd->d_name;
		pe = p+254;
		while(*pw != 0 && p < pe){
			data = ff_convert(*pw, 0);
			if(data <= 0x80)
				*p++ = (UB)data;
			else{
				*p++ = data >> 8;
				*p++ = (UB)data;
			}
			pw++;
		}
		*p = 0;
	}
	else
#endif
	{
		int     i;
		for(i = 0 ; i < (8+1+3) && dirp->finfo.fname[i] != 0 ; i++)
			pd->d_name[i] = dirp->finfo.fname[i];
		pd->d_name[i] = 0;
	}
	return 1;
}

/*
 *  FatFsディレクトリ作成
 */
static int ff_mkdir(const char *pathname)
{
	int result = f_mkdir(pathname);
	if(result == FR_OK)
		return 0;
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsディレクトリ/ファイル削除
 */
static int ff_unlink(const char *pathname)
{
	int result = f_unlink(pathname);
	if(result == FR_OK)
		return 0;
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsリネーム
 */
static int ff_rename(const char *oname, const char *nname)
{
	int result = f_rename(oname, nname);
	if(result == FR_OK)
		return 0;
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsモード変更
 */
static int ff_chmod(const char *name, int mode)
{
	FRESULT result;
	BYTE value = 0;
	BYTE mask  = AM_RDO;

	if((mode & (S_IWUSR|S_IWGRP|S_IWOTH)) == 0){
		value |= AM_RDO;
	}
	result = f_chmod(name, value, mask);
	if(result == FR_OK)
		return 0;
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFs直接ファイルステート取得
 */
static int ff_stat(const char *name, struct stat *buf)
{
	FILINFO finfo;
	FRESULT result;

	memset(buf, 0, sizeof(struct stat));
	finfo.lfname = NULL;
	result = f_stat(name, &finfo);
	if(result == FR_OK){
		buf->st_mode = S_IRUSR|S_IRGRP|S_IROTH;
		if((finfo.fattrib & AM_RDO) == 0)
			buf->st_mode |= S_IWUSR|S_IWGRP|S_IWOTH;
		buf->st_blksize = 512;
		buf->st_blocks  = (finfo.fsize+buf->st_blksize-1)/buf->st_blksize;
		buf->st_size = finfo.fsize;
		return 0;
	}
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsシステムステータスの取り出し
 */
static int ff_statfs(const char *name, void *status)
{
	struct statfs2 *p = status;
	FATFS   *fatfs;
	DWORD   nclust;
	FRESULT result;

	result = f_getfree(name, &nclust, &fatfs);
	if(result == FR_OK){
		memset(p, 0, sizeof(struct statfs2));
		p->f_bsize  = fatfs->csize * _MAX_SS;
		p->f_blocks = fatfs->max_clust-1;
		p->f_bfree  = nclust;
		return 0;
	}
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsファイルオープン
 */
static int ff_open(int devno, const char *pathname, int flags)
{
	int result, i;
	unsigned char mode;

	if(devno < 0 || devno >= MAXIMUM_DEVNO){
		ff_errno = FF_EINVAL;
		return -1;
	}
	sddevname[0] = devno + '0';
	sddevname[1] = ':';
	strcpy(&sddevname[2], pathname);
	mode = (flags+1) & 3;
	if(flags & O_CREAT)
		mode |= FA_OPEN_ALWAYS;
	if(flags & O_EXCL)
		mode |= FA_CREATE_NEW;
	result = -1;
	for(i = 0 ; i < NUM_FIL ; i++){
		if(FileObj[i].fs == 0){
			result = f_open(&FileObj[i], sddevname, mode);
			if(result == 0)
				return (i);
			else
				FileObj[i].fs = 0;
			break;
		}
	}
	ff_errno = result;
	return -1;
}

/*
 *  FatFsファイルクローズ
 */
static int ff_close(int fd)
{
	FRESULT result;

	if(fd >= NUM_FIL){
		ff_errno = FF_EINVAL;
		return -1;
	}
	result = f_close(&FileObj[fd]);
	if(result == FR_OK)
		return 0;
	else{
		ff_errno = result;
		return -1;
	}
}

/*
 *  FatFsファイルステート取得
 */
static int ff_fstat(int fd, struct stat *buf)
{
	if(fd >= NUM_FIL){
		ff_errno = FF_EINVAL;
		return -1;
	}
	memset(buf, 0, sizeof(struct stat));
	buf->st_size = FileObj[fd].fsize;
#ifdef _EXT_RTOS
	buf->st_mtime = (FileObj[fd.fdate<<16)
                   | FileObj[fd].ftime;
#endif
	buf->st_atime = buf->st_ctime
                  = buf->st_mtime;
	buf->st_mode = S_IRUSR|S_IRGRP|S_IROTH;
	if((FileObj[fd].flag & FA_WRITE) != 0)
		buf->st_mode |= S_IWUSR|S_IWGRP|S_IWOTH;
	return 0;
}

/*
 *  FatFsファイルシーク
 */
static off_t ff_lseek(int fd, off_t offset, int whence, int *res)
{
	off_t off = 0;
	FRESULT result;

	*res = -1;
	if(fd >= NUM_FIL){
		ff_errno = FF_EINVAL;
		return -1;
	}
	switch (whence) {
	case SEEK_SET:
		off = offset;
		break;
	case SEEK_CUR:
		off = FileObj[fd].fptr + offset;
		break;
	case SEEK_END:
		off = FileObj[fd].fsize + offset;
		break;
	default:
		ff_errno =  FF_EINVAL;
		return -1;
	}
	result = f_lseek(&FileObj[fd], off);
	if(result != FR_OK){
		ff_errno = result;
		off = -1;
	}
	else
		*res = 0;
	return off;
}

/*
 *  FatFsファイル読みだし
 */
static long ff_read(int fd, void *buf, long count, int *res)
{
	long n;
	FRESULT result;

	*res = -1;
	if(fd >= NUM_FIL){
		ff_errno = FF_EINVAL;
		return 0;
	}
	result =  f_read(&FileObj[fd], buf, count, (unsigned int *)&n);
	if(result != FR_OK){
		ff_errno = result;
		n = 0;
	}
	else
		*res = 0;
	return n;
}

/*
 *  FatFsファイル書き込み
 */
static long ff_write(int fd, const void *buf, long count, int *res)
{
	long n = 0;
	FRESULT result;

	*res = -1;
	if(fd >= NUM_FIL){
		ff_errno = FF_EINVAL;
		return 0;
	}
	result = f_write(&FileObj[fd], buf, count, (unsigned int *)&n);
	if(result != FR_OK){
		ff_errno = result;
		n = 0;
	}
	else
		*res = 0;
	return n;
}

/* This function is called each time errno is evaluated. */
int *__errno(void)
{
	return (int *)&ff_errno;
}

