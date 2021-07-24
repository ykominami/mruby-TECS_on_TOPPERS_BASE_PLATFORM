/*	$NetBSD: fcntl.h,v 1.10.2.1 1998/01/29 10:38:31 mellon Exp $	*/

/*-
 * Copyright (c) 1983, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)fcntl.h	8.3 (Berkeley) 1/21/94
 */

/*
 * このプログラムはNetBSD用ヘッダーに
 * ＲＯＭファイル専用ファイルストレージ専用機能を追加したインクルードファイルである。
 *
 * $Modifier:  roi takeuchi
 * $Date:    2007/01/11
 * $Version: 1.0.0.0
 *
 * COPYRIGHT (C) 2007 RICOH COMPANY LTD.
 */
#ifndef _SYS_FCNTL_H_
#define	_SYS_FCNTL_H_

/*
 * This file includes the definitions for open and fcntl
 * described by POSIX for <fcntl.h>; it also includes
 * related kernel definitions.
 */
#ifndef NOTUSE_TYPES_H	/* types.hは呼ばない */
#include <sys/types.h>
#elif defined(__SHC__)	/* SHCの場合off_tは未定義 */
typedef long long off_t;
#endif

/*
 * File status flags: these are used by open(2), fcntl(2).
 * They are also used (indirectly) in the kernel file structure f_flags,
 * which is a superset of the open/fcntl flags.  Open flags and f_flags
 * are inter-convertible using OFLAGS(fflags) and FFLAGS(oflags).
 * Open/fcntl flags begin with O_; kernel-internal flags begin with F.
 */
/* open-only flags */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

/*
 * Kernel encoding of open mode; separate read and write bits that are
 * independently testable: 1 greater than the above.
 *
 * XXX
 * FREAD and FWRITE are excluded from the #ifdef _KERNEL so that TIOCFLUSH,
 * which was documented to use FREAD/FWRITE, continues to work.
 */
#ifndef _POSIX_SOURCE
#define	FREAD		0x0001
#define	FWRITE		0x0002
#endif
#define	O_NONBLOCK	0x0004		/* no delay */
#define	O_APPEND	0x0008		/* set append mode */
#ifndef _POSIX_SOURCE
#define	O_SHLOCK	0x0010		/* open with shared file lock */
#define	O_EXLOCK	0x0020		/* open with exclusive file lock */
#define	O_ASYNC		0x0040		/* signal pgrp when data ready */
#endif
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#define	O_SYNC		0x0080		/* synchronous writes */
#endif
#define	O_CREAT		0x0200		/* create if nonexistant */
#define	O_TRUNC		0x0400		/* truncate to zero length */
#define	O_EXCL		0x0800		/* error if already exists */

/* defined by POSIX 1003.1; BSD default, but required to be distinct */
#define	O_NOCTTY	0x8000		/* don't assign controlling terminal */

/*
 * The O_* flags used to have only F* names, which were used in the kernel
 * and by fcntl.  We retain the F* names for the kernel f_flags field
 * and for backward compatibility for fcntl.
 */
#ifndef _POSIX_SOURCE
#define	FAPPEND		O_APPEND	/* kernel/compat */
#define	FASYNC		O_ASYNC		/* kernel/compat */
#define	FFSYNC		O_SYNC		/* kernel */
#ifndef _XOPEN_SOURCE
#define	O_FSYNC		O_SYNC		/* compat */
#endif
#define	FNONBLOCK	O_NONBLOCK	/* kernel */
#define	FNDELAY		O_NONBLOCK	/* compat */
#define	O_NDELAY	O_NONBLOCK	/* compat */
#endif

/*
 * Constants used for fcntl(2)
 */

/* command values */
#define	F_DUPFD		0		/* duplicate file descriptor */
#define	F_GETFD		1		/* get file descriptor flags */
#define	F_SETFD		2		/* set file descriptor flags */
#define	F_GETFL		3		/* get file status flags */
#define	F_SETFL		4		/* set file status flags */
#ifndef _POSIX_SOURCE
#define	F_GETOWN	5		/* get SIGIO/SIGURG proc/pgrp */
#define F_SETOWN	6		/* set SIGIO/SIGURG proc/pgrp */
#endif
#define	F_GETLK		7		/* get record locking information */
#define	F_SETLK		8		/* set record locking information */
#define	F_SETLKW	9		/* F_SETLK; wait if blocked */

/* file descriptor flags (F_GETFD, F_SETFD) */
#define	FD_CLOEXEC	1		/* close-on-exec flag */

/* record locking flags (F_GETLK, F_SETLK, F_SETLKW) */
#define	F_RDLCK		1		/* shared or read lock */
#define	F_UNLCK		2		/* unlock */
#define	F_WRLCK		3		/* exclusive or write lock */

#ifndef _POSIX_SOURCE
/* lock operations for flock(2) */
#define	LOCK_SH		0x01		/* shared file lock */
#define	LOCK_EX		0x02		/* exclusive file lock */
#define	LOCK_NB		0x04		/* don't block when locking */
#define	LOCK_UN		0x08		/* unlock file */
#endif

/* fseek parameters */
#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

/* struct stat defintion */
#ifndef	_SYS_STAT_H
struct stat {
	unsigned int    st_dev;		/* inode's device */
	unsigned int    st_ino;		/* inode's number */
	unsigned int    st_mode;	/* inode protection mode */
	unsigned int    t_nlink;	/* number of hard links */
	unsigned int    st_uid;		/* user ID of the file's owner */
	unsigned int    st_gid;		/* group ID of the file's group */
	unsigned int    st_rdev;	/* device type */
	int             st_atime;	/* time of last access */
	long            st_atimensec;	/* nsec of last access */
	int             st_mtime;	/* time of last data modification */
	long            st_mtimensec;	/* nsec of last data modification */
	int             st_ctime;	/* time of last file status change */
	long            st_ctimensec;	/* nsec of last file status change */
	off_t           st_size;	/* file size, in bytes */
	off_t           st_blocks;	/* blocks allocated for file */
	unsigned int    st_blksize;	/* optimal blocksize for I/O */
	unsigned int    st_flags;	/* user defined flags for file */
	unsigned int    st_gen;		/* file generation number */
	off_t           st_qspare[2];
};
#define _STAT_H_
#endif	/* _SYS_STAT_H */

/*
 * Protections are chosen from these bits, or-ed together
 */
#define	PROT_NONE	0x00	/* no permissions */
#define	PROT_READ	0x01	/* pages can be read */
#define	PROT_WRITE	0x02	/* pages can be written */
#define	PROT_EXEC	0x04	/* pages can be executed */

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#define	MAP_SHARED	0x0001	/* share changes */
#define	MAP_PRIVATE	0x0002	/* changes are private */
#define	MAP_COPY	0x0004	/* "copy" region at mmap time */

struct StdFileIndex {
	void            *sdmdev;
	int             sdmfd;
};

extern int   open(const char *pathname, int flags);
extern int   close(int fd);
extern int   fstat(int fd, struct stat *buf);
extern off_t lseek(int fd, off_t offset, int whence);
extern long  read(int fd, void *buf, long count);
extern long  write(int fd, const void *buf, long count);
extern void  *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
extern int   munmap(void *start, size_t length);


#endif /* !_SYS_FCNTL_H_ */


