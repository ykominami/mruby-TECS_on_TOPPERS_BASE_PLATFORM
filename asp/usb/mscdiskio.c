/*
 *  TOPPERS/ASP/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: mscdickio.c,v 1.1 2017/05/27 13:11:52 roi Exp $
 */

/*
 * ＭＳＣ専用ストレージ関数をサポートする
 * 擬似的なストレージ関数郡であり、標準ライブラリィと分けての使用が望ましい。
 * 1. msc_init                 この関数郡の初期化関数(bssがゼロ設定の場合この関数は不要)
 * 2. msc_sense                MSCのセンスを行う
 * 3. msc_diskstatus           MSCのステータス取得:なし
 * 4. msc_diskread             MSCの読み込み:なし
 * 5. msc_diskwrite            MSCの書き込み:なし
 * 6. msc_iocil                MSCのIO制御:なし
 *
 */

#include "kernel_impl.h"
#include <stdlib.h>
#include <string.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "fcntl.h"
#include "device.h"
#include "storagedevice.h"
#include "ff.h"
#include "diskio.h"
#include "usb_otg.h"
#include "mscdiskio.h"
#include "tusbh_msc.h"

static int msc_sense(void *psdev, bool_t on);
static int msc_diskstatus(void *psdev);
static int msc_diskread(void *pif, BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount);
static int msc_diskwrite(void *pif, const BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount);
static int msc_diskioctl(void *pif, BYTE Func, void* Buffer);

static const StorageDeviceFunc_t fatfsMscDeviceFunc = {
	msc_sense,
	msc_diskstatus,
	msc_diskstatus,
	(int(*)())msc_diskread,
	(int(*)())msc_diskwrite,
	(int(*)())msc_diskioctl
};

static MSC_Unit_t MSCLUNInfo;
static FATFS ActiveFatFsObj __attribute__ ((aligned (32)));

/*
 *  FatFs用SDカードファイルドライバの初期化
 */
#if MSC_DEVNO == 0
void sd_init(intptr_t exinf)
#else
void msc_init(intptr_t exinf)
#endif
{
	StorageDevice_t *psdev;

	SDMSetupDevice(MSC_DEVNO, &psdev);
	psdev->pdevf            = (StorageDeviceFunc_t *)&fatfsMscDeviceFunc;
	psdev->pdevff           = (StorageDeviceFileFunc_t *)&fatfsSDeviceFileFunc;
	psdev->_sdev_secsize    = 512;
	psdev->_sdev_port       = MSC_PORTID;
#ifdef SDEV_INSWAIT_TIME
	psdev->_sdev_inswait    = SDEV_INSWAIT_TIME;
#else
	psdev->_sdev_inswait    = 0;
#endif
	psdev->_sdev_attribute |= SDEV_INSERTCHK|SDEV_CHKREMOVE;
	psdev->_sdev_local[0]   = &ActiveFatFsObj;
	psdev->_sdev_local[1]   = NULL;
}

/*
 *  MSCセンス関数
 */
static int msc_sense(void *pif, BOOL on)
{
	StorageDevice_t  *psdev = pif;
	TUSBH_Handle_t   *husbh;
	TUSBH_ERCODE     status;
	bool_t   exist;
	char     pdrv;
	int      result = FR_DISK_ERR;

	pdrv  = psdev->_sdev_port - MSC_PORTID;
	husbh = (TUSBH_Handle_t *)psdev->_sdev_local[1];
	if(husbh == NULL)
		exist = 0;
	else
		exist = tusbhMscUnitIsReady(husbh, pdrv);
	if(on && !exist){
		f_mount(psdev->_sdev_devno, 0);
		psdev->_sdev_attribute &= ~SDEV_DEVERROR;
		return TRUE;
	}
	else if(!on && exist){
		psdev->_sdev_instimer += SENSE_TIME;
		status = tusbhMscGetLUNInfo(husbh, pdrv, &MSCLUNInfo);
		if(status != TUSBH_E_OK)
			psdev->_sdev_attribute |= SDEV_DEVERROR;
		else
			psdev->_sdev_maxsec = MSCLUNInfo.num_block;
		if((psdev->_sdev_attribute & SDEV_DEVERROR) == 0)
			result = f_mount(psdev->_sdev_devno, &ActiveFatFsObj);
		if(result != FR_OK)
			psdev->_sdev_attribute |= SDEV_DEVERROR;
		else
			psdev->_sdev_local[0]   = &ActiveFatFsObj;
#if 1
		syslog_3(LOG_NOTICE, "## attr[%04x] max(%d) result(%d) ##", psdev->_sdev_attribute, psdev->_sdev_maxsec, result);
#endif
		return TRUE;
	}
	else
		return FALSE;
}

/*
 *  FatFs用MSCステータス関数
 */
static int msc_diskstatus(void *pif)
{
	TUSBH_Handle_t  *husbh;
	StorageDevice_t *psdev = pif;

	if(psdev == NULL)
		return STA_NODISK;
	husbh = (TUSBH_Handle_t *)psdev->_sdev_local[1];
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || husbh == NULL)
		return STA_NOINIT;
	else
		return 0;
}

/*
 *  FatFs用MSC読み込み関数
 */
static int msc_diskread(void *pif, BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount)
{
 	StorageDevice_t   *psdev = pif;
	USB_OTG_Handle_t  *husb;
	TUSBH_Handle_t    *husbh;
	TUSBH_ERCODE      status;
	char              pdrv;
	int               retry = 0;
	uint8_t           *buff = NULL;
	unsigned int      align = ((unsigned int)Buffer) & 3;
	BYTE              *buffer = (BYTE *)Buffer;

	husbh = (TUSBH_Handle_t *)psdev->_sdev_local[1];
	husb  = (USB_OTG_Handle_t *)husbh->pSysData;
	pdrv  = psdev->_sdev_port - MSC_PORTID;
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || husbh == NULL)
		return RES_ERROR;
	if(align != 0 && husb->Init.dma_enable == 1){
		uint32_t *addr = malloc_cache(psdev->_sdev_secsize*SectorCount);
		if(addr != NULL){
			buff   = buffer;
			buffer = (BYTE *)addr;
		}
	}
	do{
		status = tusbhMscRead(husbh, pdrv, SectorNumber, (uint8_t *)buffer, SectorCount);
		if(status == TUSBH_E_SYS)
			break;
		retry++;
	}while(status != TUSBH_E_OK && retry < RETRY_COUNT);
	if(buff != NULL){
		tusbmemcpy(buff, buffer, psdev->_sdev_secsize*SectorCount);
		free_cache(buffer);
	}
	if(status == TUSBH_E_OK)
		return RES_OK;
	else if(status == TUSBH_E_SYS)
		psdev->_sdev_attribute |= SDEV_DEVERROR;
	return RES_ERROR;
}

/*
 *  FatFs用MSC書き込み関数
 */
static int msc_diskwrite(void *pif, const BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount)
{
 	StorageDevice_t   *psdev = pif;
	USB_OTG_Handle_t  *husb;
	TUSBH_Handle_t    *husbh;
	TUSBH_ERCODE      status;
	char              pdrv;
	int               retry = 0;
	uint8_t           *buff = NULL;
	unsigned int      align = ((unsigned int)Buffer) & 3;
	BYTE              *buffer = (BYTE *)Buffer;

	husbh = (TUSBH_Handle_t *)psdev->_sdev_local[1];
	husb  = (USB_OTG_Handle_t *)husbh->pSysData;
	pdrv  = psdev->_sdev_port - MSC_PORTID;
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || husbh == NULL)
		return RES_ERROR;
	if(align != 0 && husb->Init.dma_enable == 1){
		uint32_t *addr = malloc_cache(psdev->_sdev_secsize*SectorCount);
		if(addr != NULL){
			buff   = (uint8_t *)buffer;
			buffer = (BYTE *)addr;
			tusbmemcpy(buffer, buff, psdev->_sdev_secsize*SectorCount);
		}
	}
	do{
		status = tusbhMscWrite(husbh, pdrv, SectorNumber, (uint8_t *)buffer, SectorCount);
		if(status == TUSBH_E_SYS)
			break;
		retry++;
	}while(status != TUSBH_E_OK && retry < RETRY_COUNT);
	if(buff != NULL)
		free_cache(buffer);
	if(status == TUSBH_E_OK)
		return RES_OK;
	else if(status == TUSBH_E_SYS)
		psdev->_sdev_attribute |= SDEV_DEVERROR;
	return RES_ERROR;
}


/*
 *  FatFs用MSC-DISKIO制御関数
 */
static int msc_diskioctl(void *pif, BYTE Func, void* Buffer)
{
	StorageDevice_t *psdev = (StorageDevice_t *)pif;
	DRESULT         result;

	if(psdev == NULL)
		return RES_ERROR;
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY)
		return RES_ERROR;
	switch(Func){
	case CTRL_SYNC:
		result = RES_OK;			/* no action */
		break;
	case GET_SECTOR_COUNT:
		*((DWORD *)Buffer) = psdev->_sdev_maxsec;
		syslog_2(LOG_NOTICE, "ioctl notuse (%d)(%d) ", (int)Func, psdev->_sdev_maxsec);
		result = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*((DWORD *)Buffer) = 135;	/* ERASE_BLK */
		syslog_1(LOG_NOTICE, "call disk_ioctl(GET_BLOCK_SIZE, %08x)", (int)(*((DWORD *)Buffer)));
		result = RES_OK;
		break;
	default:
		syslog_2(LOG_NOTICE, "call disk_ioctl(%d, %08x)", (int)psdev->_sdev_devno, (int)Buffer);
		slp_tsk();
		result = RES_PARERR;
		break;
	}
	return result;
}

