/*
 *  FatFs for TOPPERS
 *      FAT File system/
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 * 
 *  Copyright (C) 2005- by Industrial Technology Institute,
 *                          Miyagi Prefectural Government, JAPAN
 *  Copyright (C) 2007-2010 by 
 *                     GJ Business Division RICOH COMPANY,LTD. JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Group.
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
 *  @(#) $Id: diskio.c,v 1.3 2017/07/06 16:24:36 roi Exp $
 */

/*
 *  FatFsが規定している下位レイヤI/F
 *  　本来は複数ドライブに対応すべきだが、このターゲットボードでは
 *  　ドライブは１つのみサポートしている。
 */

#include "kernel_impl.h"
#include <string.h>	/*  memcpy()  */
#include <t_syslog.h>
#include <t_stdlib.h>
#include "storagedevice.h"
#include "diskio.h"

#define RETRY_COUNT    8

/*
 *  ディスク・ドライブの初期化
 *　　引数
 *　　　 BYTE Drive	物理ドライブ番号
 */
DSTATUS
disk_initialize(BYTE Drive)
{
	StorageDevice_t *psdev = SDMGetStorageDevice(Drive);

	if(psdev == NULL || psdev->pdevf == NULL || psdev->pdevf->_sdevf_diskinit == NULL)
		return STA_NODISK;
	return psdev->pdevf->_sdevf_diskinit(psdev);
}


/*
 *  ディスク・ドライブの状態取得
 *　　引数
 *　　　 BYTE Drive	物理ドライブ番号
 */
DSTATUS
disk_status(BYTE Drive)
{
	StorageDevice_t *psdev = SDMGetStorageDevice(Drive);

	if(psdev == NULL || psdev->pdevf == NULL || psdev->pdevf->_sdevf_diskstatus == NULL)
		return STA_NODISK;
	return psdev->pdevf->_sdevf_diskstatus(psdev);
}


/*
 *  ディスクからの読み込み
 *　　引数
 *　　　 BYTE Drive　　　　　物理ドライブ番号
 *　　　 BYTE* Buffer        読み出しバッファへのポインタ
 *　　　 DWORD SectorNumber  読み出し開始セクタ番号
 *　　　 BYTE SectorCount    読み出しセクタ数
 */
DRESULT
disk_read(BYTE Drive, BYTE* Buffer, DWORD SectorNumber, BYTE SectorCount)
{
	StorageDevice_t *psdev = SDMGetStorageDevice(Drive);

	if(psdev == NULL || psdev->pdevf == NULL || psdev->pdevf->_sdevf_diskread == NULL)
		return RES_ERROR;
	return psdev->pdevf->_sdevf_diskread(psdev, Buffer, SectorNumber, SectorCount);
}

/*
 *  ディスクへの書き込み
 *　　引数
 *　　　 BYTE Drive　　　　　物理ドライブ番号
 *　　　 BYTE* Buffer        書き込むデータへのポインタ
 *　　　 DWORD SectorNumber  書き込み開始セクタ番号
 *　　　 BYTE SectorCount    書き込みセクタ数
 */
DRESULT
disk_write(BYTE Drive, const BYTE* Buffer, DWORD SectorNumber, BYTE SectorCount)
{
	StorageDevice_t *psdev = SDMGetStorageDevice(Drive);

	if(psdev == NULL || psdev->pdevf == NULL || psdev->pdevf->_sdevf_diskwrite == NULL)
		return RES_ERROR;
	return psdev->pdevf->_sdevf_diskwrite(psdev, Buffer, SectorNumber, SectorCount);
}

DRESULT
disk_ioctl(BYTE Drive, BYTE Func, void* Buffer)
{
	StorageDevice_t *psdev = SDMGetStorageDevice(Drive);

	if(psdev == NULL || psdev->pdevf == NULL || psdev->pdevf->_sdevf_diskioctl == NULL)
		return RES_ERROR;
	return psdev->pdevf->_sdevf_diskioctl(psdev, Func, Buffer);
}

/*
 *  日付・時刻の取得
 *  　　ToDo：未実装
 */
DWORD
get_fattime(void)
{
	SYSTIM systim;
	DWORD  fdate = ((2017-1980)<<25)+(1<<21)+(1<<16);
	DWORD  ftime;

	if(SDeviceHead._get_datetime != 0)
		return (DWORD)SDeviceHead._get_datetime();
	else{
		get_tim(&systim);
		systim = (systim+1000L) /2000L;
		ftime  = systim % 30;
		ftime += ((systim/30) % 60)<<5;
		ftime += ((systim/(60*30))<<11);
		return (fdate+ftime);
	}
}

