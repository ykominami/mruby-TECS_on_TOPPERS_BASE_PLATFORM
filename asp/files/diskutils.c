/*
 *  TOPPERS/JSP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Just Standard Profile Kernel
 * 
 *  Copyright (C) 2010-2015 by GJ Designing Center RICOH COMPANY,LTD. JAPAN
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
 *  @(#) $Id: diskutils.c,v 2.0 2015/05/01 07:32:56 roi Exp $
 */

#include <stdio.h>
#include <string.h>
#include "storagedevice.h"
#include "monitor.h"

#define NUM_VOLUME_CMD      5

static int_t volume_format(int argc, char **argv);
static int_t volume_dir(int argc, char **argv);
static int_t volume_mkdir(int argc, char **argv);
static int_t volume_rmdir(int argc, char **argv);
static int_t volume_delete(int argc, char **argv);

/*
 *  VOLUMEコマンドテーブル
 */
static const COMMAND_INFO volume_command_info[] = {
	{"FORMAT",		volume_format },	/* ボリュームフォーマット */
	{"DIR",			volume_dir    },	/* ディレクトリ表示 */
	{"MKDIR",		volume_mkdir  },	/* ディレクトリ作成 */
	{"RMDIR",		volume_rmdir  },	/* ディレクトリ削除 */
	{"ERASE",		volume_delete }		/* ファイル削除 */
};

static const char volume_name[] = "VOLUME";
static const char volume_help[] =
"  Volume  FORMAT   [drive]\n"
"          DIR      [path] \n"
"          MKDIR    [path] \n"
"          RMDIR    [path] \n"
"          ERASE    [path] \n";

static COMMAND_LINK volume_command_link = {
	NULL,
	NUM_VOLUME_CMD,
	volume_name,
	NULL,
	volume_help,
	&volume_command_info[0]
};

static struct dirent2 finfo;

/*
 *  RTCコマンド設定関数
 */
void volume_info_init(intptr_t exinf)
{
	setup_command(&volume_command_link);
}

/*
 *  ストレージボリュームフォーマット
 */
static int_t
volume_format(int argc, char **argv)
{
	printf("not support !\n");
	return 0;
}

/*
 *  ストレージボリュームディレクトリ
 */
static int_t
volume_dir(int argc, char **argv)
{
	StorageDevice_t *psdev;
	struct statfs2  status;
	char   *path, *spath, name[16];
	int    devno, no, count;
	void   *dir;

	name[0] = 0;
	if(argc > 1)
		spath = path = argv[1];
	else
		spath = path = &name[1];
	devno = SDMGetDeviceNo((const char **)&path);
	psdev = SDMGetStorageDevice(devno);
	if(psdev == 0){
		printf("\nno device !\n");
		return -1;
	}
	if(psdev->pdevff == 0 || psdev->pdevff->_sdevff_opendir == 0){
		printf("\nno function !\n");
		return -1;
	}
	if((psdev->_sdev_attribute & SDEV_EMPLOY) == 0){
		printf("\nno media !\n");
		return -1;
	}
	count = 0;
	dir = psdev->pdevff->_sdevff_opendir(spath);
	if(dir != NULL){
		if(*spath == 0){
			name[0] = devno + '0';
			name[1] = ':';
			name[2] = '/';
			name[3] = 0;
			spath = name;
		}
		printf("\nDirectory [%s]\n", spath);
		while(psdev->pdevff->_sdevff_readdir(dir, &finfo) != 0 && finfo.d_name[0] != 0){
			count++;
			printf("%3d %s ", count, finfo.d_name);
			no = strlen(finfo.d_name);
			while(no < 24){
				putchar(' ');
				no++;
			}
			printf("%04d/%02d/%02d ", (finfo.d_date>>9)+1980, (finfo.d_date>>5) & 15, finfo.d_date & 31);
			printf("%02d:%02d:%02d ", finfo.d_time>>11, (finfo.d_time>>5) & 63, (finfo.d_time & 31)*2);
			if(finfo.d_type & AM_HID)
				name[0] = '*';
			else
				name[0] = ' ';
			if(finfo.d_type & AM_SYS)
				name[1] = 'S';
			else
				name[1] = ' ';
			name[2] = 'R';
			if(finfo.d_type & AM_RDO)
				name[3] = ' ';
			else
				name[3] = 'W';
			name[4] = 0;
			if(finfo.d_type & AM_DIR)
				printf("<DIR>  \n");
			else if(finfo.d_type & AM_VOL)
				printf("<VOL>  \n");
			else
				printf("[%s]  %8d\n", name, (int)finfo.d_fsize);
		}
		printf("     %d file(s)\n", count);
		psdev->pdevff->_sdevff_closedir(dir);
		if(psdev->pdevff->_sdevff_statfs != 0
			 && psdev->pdevff->_sdevff_statfs(spath, &status) == 0){
			printf("     %d free blocks %d bytes in a block\n", (int)status.f_bfree, (int)status.f_bsize);
		}
	}
	else{
		printf("Disk Error !\n");
	}
	return 0;
}

/*
 *  ストレージボリュームMKDIR
 */
static int_t
volume_mkdir(int argc, char **argv)
{
	StorageDevice_t *psdev;
	char   *path, *spath;
	int    devno;

	if(argc > 1){
		spath = path = argv[1];
		devno = SDMGetDeviceNo((const char **)&path);
		psdev = SDMGetStorageDevice(devno);
		if(psdev == 0){
			printf("\nno device !\n");
			return -1;
		}
		if(psdev->pdevff == 0 || psdev->pdevff->_sdevff_mkdir == 0){
			printf("\nno function !\n");
			return -1;
		}
		printf(" mkdir[%s]\n", path);
		if(psdev->pdevff->_sdevff_mkdir(spath) >= 0)
			printf("   Ok\n");
		else
			printf("   Error\n");
	}
	return 0;
}

/*
 *  ストレージボリュームRMDIR
 */
static int_t
volume_rmdir(int argc, char **argv)
{
	StorageDevice_t *psdev;
	char   *path, *spath;
	int    devno;

	if(argc > 1){
		spath = path = argv[1];
		devno = SDMGetDeviceNo((const char **)&path);
		psdev = SDMGetStorageDevice(devno);
		if(psdev == 0){
			printf("\nno device !\n");
			return -1;
		}
		if(psdev->pdevff == 0 || psdev->pdevff->_sdevff_rmdir == 0){
			printf("\nno function !\n");
			return -1;
		}
		printf(" rmdir[%s]\n", path);
		if(psdev->pdevff->_sdevff_rmdir(spath) >= 0)
			printf("   Ok\n");
		else
			printf("   Error\n");
	}
	return 0;
}

/*
 *  ストレージボリュームファイル削除
 */
static int_t
volume_delete(int argc, char **argv)
{
	StorageDevice_t *psdev;
	char   *path, *spath;
	int    devno;

	if(argc > 1){
		spath = path = argv[1];
		devno = SDMGetDeviceNo((const char **)&path);
		psdev = SDMGetStorageDevice(devno);
		if(psdev == 0){
			printf("\nno device !\n");
			return -1;
		}
		if(psdev->pdevff == 0 || psdev->pdevff->_sdevff_unlink == 0){
			printf("\nno function !\n");
			return -1;
		}
		printf(" unlink[%s]\n", path);
		if(psdev->pdevff->_sdevff_unlink(spath) >= 0)
			printf("   Ok\n");
		else
			printf("   Error\n");
	}
	return 0;
}

