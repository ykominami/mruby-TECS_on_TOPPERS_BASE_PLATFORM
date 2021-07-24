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
 *  @(#) $Id: sddickio.c,v 1.2 2016/01/19 21:56:56 roi Exp $
 */

/*
 * このプログラムはITRON専用のTYPE3ソフトウェアである。
 * ＳＤカード専用ストレージ関数をサポートする
 * 擬似的なストレージ関数郡であり、標準ライブラリィと分けての使用が望ましい。
 * 1. sd_init                  この関数郡の初期化関数(bssがゼロ設定の場合この関数は不要)
 * 2. sdcard_sense             SD-CARDのセンスを行う
 * 3. sdcard_diskstatus        SD-CARDのステータス取得:なし
 * 4. sdcard_diskread          SD-CARDの読み込み:なし
 * 5. sdcard_diskwrite         SD-CARDの書き込み:なし
 * 6. sdcard_iocil             SD-CARDのIO制御:なし
 *
 */

#include "kernel_impl.h"
#include <stdlib.h>
#include <string.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "fcntl.h"
#include "device.h"
#include "mcicmd.h"
#include "storagedevice.h"
#include "ff.h"
#include "diskio.h"
#include "sddiskio.h"

static int sdcard_sense(void *psdev, BOOL on);
static int sdcard_diskstatus(void *psdev);
static int sdcard_diskread(void *pif, BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount);
static int sdcard_diskwrite(void *pif, const BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount);
static int sdcard_diskioctl(void *pif, BYTE Func, void* Buffer);

static const StorageDeviceFunc_t fatfsSDeviceFunc = {
	sdcard_sense,
	sdcard_diskstatus,
	sdcard_diskstatus,
	sdcard_diskread,
	sdcard_diskwrite,
	sdcard_diskioctl
};

static MCIINFO SDCardInfo;
static FATFS ActiveFatFsObj __attribute__ ((aligned (32)));

#ifdef DMA_ALINE
static unsigned int abuff[512/sizeof(unsigned int)];

static void mem_cpy(unsigned char *d, unsigned char *s, int len)
{
	while(len > 0){
		*d++ = *s++;
		len--;
	}
}
#endif

/*
 *  FatFs用SDカードファイルドライバの初期化
 */
void sd_init(intptr_t exinf)
{
	StorageDevice_t *psdev;

	SDMSetupDevice(SDCARD_DEVNO, &psdev);
	psdev->pdevf            = (StorageDeviceFunc_t *)&fatfsSDeviceFunc;
	psdev->pdevff           = (StorageDeviceFileFunc_t *)&fatfsSDeviceFileFunc;
	psdev->_sdev_secsize    = 512;
	psdev->_sdev_port       = SDCRAD_PORTID;
#ifdef SDEV_INSWAIT_TIME
	psdev->_sdev_inswait    = SDEV_INSWAIT_TIME;
#else
	psdev->_sdev_inswait    = 0;
#endif
	psdev->_sdev_attribute |= SDEV_INSERTCHK|SDEV_CHKREMOVE;
	psdev->_sdev_local[0]   = &ActiveFatFsObj;
}

/*
 *  SDCARDセンス関数
 */
static int sdcard_sense(void *pif, BOOL on)
{
	StorageDevice_t *psdev = pif;
	bool_t   exist = mci_ses_por(((StorageDevice_t *)psdev)->_sdev_port);
	MCIPCB   *pmci;
	int      result = FR_DISK_ERR;

	pmci = psdev->_sdev_local[1];
	if(on && !exist){
		f_mount(psdev->_sdev_devno, 0);
		psdev->_sdev_attribute &= ~SDEV_DEVERROR;
		mci_cls_por(pmci);
		return TRUE;
	}
	else if(!on && exist){
		psdev->_sdev_instimer += SENSE_TIME;
		if((psdev->_sdev_attribute & SDEV_ONEEXIT) != 0 && psdev->_sdev_instimer < psdev->_sdev_inswait)
			return FALSE;
		pmci = mci_opn_por(((StorageDevice_t *)psdev)->_sdev_port);
		if(pmci == NULL)
			return FALSE;
		psdev->_sdev_local[1] = pmci;
		if(MciCheckCID(pmci) != E_OK){
			psdev->_sdev_attribute |= SDEV_DEVERROR;
			return TRUE;
		}
		if(MciSetAddress(pmci) != E_OK){
			psdev->_sdev_attribute |= SDEV_DEVERROR;
			return TRUE;
		}
		if(MciSendCID(pmci) != E_OK){
			psdev->_sdev_attribute |= SDEV_DEVERROR;
			return TRUE;
		}
		if(MciGetCardInfo(pmci, &SDCardInfo) != E_OK)
			psdev->_sdev_attribute |= SDEV_DEVERROR;
		else{
			psdev->_sdev_maxsec = SDCardInfo.maxsector;
			if(MciSelectCard(pmci, (((uint32_t)SDCardInfo.RCA) << 16)) != E_OK)
				psdev->_sdev_attribute |= SDEV_DEVERROR;
		}
		if(MciConfiguration(pmci) != E_OK)
			psdev->_sdev_attribute |= SDEV_DEVERROR;
		if(MciSetWideBus(pmci) != E_OK)
			psdev->_sdev_attribute |= SDEV_DEVERROR;
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
 *  FatFs用SDCARDステータス関数
 */
static int sdcard_diskstatus(void *pif)
{
	StorageDevice_t *psdev = pif;
	MCIPCB          *pmci;

	if(psdev == NULL)
		return STA_NODISK;
	pmci = psdev->_sdev_local[1];
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || pmci == NULL)
		return STA_NOINIT;
	else if((pmci->status & SDMODE_PROTECT) != 0)
		return STA_PROTECT;
	else
		return 0;
}

/*
 *  FatFs用SDCARD読み込み関数
 */
static int sdcard_diskread(void *pif, BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount)
{
 	StorageDevice_t *psdev = pif;
	MCIPCB          *pmci;
	ER              ercd = E_OK;
	int             retry = 0;
#ifdef DMA_ALINE
	unsigned int    align = ((unsigned int)Buffer) & 3;
	int             i;
#endif

	pmci = psdev->_sdev_local[1];
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || pmci == NULL)
		return RES_ERROR;
#ifndef DMA_ALINE
	do{
		ercd = mci_red_blk(pmci, Buffer, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, SectorCount);
		if(ercd == E_OK)
			ercd = mci_wai_trn(pmci, 30*1000);
		retry++;
	}while(ercd != E_OK && retry < RETRY_COUNT);
#else
	if(align == 0){
		do{
			ercd = mci_red_blk(pmci, Buffer, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, SectorCount);
			if(ercd == E_OK)
				ercd = mci_wai_trn(pmci, 30*1000);
			retry++;
		}while(ercd != E_OK && retry < RETRY_COUNT);
	}
	else{
		for(i = 0 ; i < SectorCount ; i++, SectorNumber++, Buffer += psdev->_sdev_secsize){
			do{
				ercd = mci_red_blk(pmci, abuff, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, 1);
				if(ercd == E_OK)
					ercd = mci_wai_trn(pmci, 30*1000);
				retry++;
			}while(ercd != E_OK && retry < RETRY_COUNT);
			mem_cpy(Buffer, abuff, psdev->_sdev_secsize);
		}
	}
#endif
	if(ercd == E_OK)
		return RES_OK;
	else
		return RES_ERROR;
}

/*
 *  FatFs用SDCARD書き込み関数
 */
static int sdcard_diskwrite(void *pif, const BYTE *Buffer, DWORD SectorNumber, BYTE SectorCount)
{
	StorageDevice_t *psdev = pif;
	MCIPCB          *pmci;
	ER              ercd = E_OK;
	int             retry = 0;
#ifdef DMA_ALINE
	unsigned int    align = ((unsigned int)Buffer) & 3;
	int             i;
#endif

	pmci = psdev->_sdev_local[1];
	if((psdev->_sdev_attribute & (SDEV_EMPLOY|SDEV_NOTUSE)) != SDEV_EMPLOY || pmci == NULL)
		return RES_ERROR;
#ifndef DMA_ALINE
	do{
		ercd = mci_wri_blk(pmci, Buffer, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, SectorCount);
		if(ercd == E_OK)
			ercd = mci_wai_trn(pmci, 30*1000);
		retry++;
	}while(ercd != E_OK && retry < RETRY_COUNT);
#else
	if(align == 0){
		do{
			ercd = mci_wri_blk(pmci, Buffer, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, SectorCount);
			if(ercd == E_OK)
				ercd = mci_wai_trn(pmci, 30*1000);
			retry++;
		}while(ercd != E_OK && retry < RETRY_COUNT);
	}
	else{
		for(i = 0 ; i < SectorCount ; i++, SectorNumber++, Buffer += psdev->_sdev_secsize){
			mem_cpy(abuff, Buffer, psdev->_sdev_secsize);
			do{
				ercd = mci_wri_blk(pmci, abuff, SectorNumber * psdev->_sdev_secsize, psdev->_sdev_secsize, 1);
				if(ercd == E_OK)
					ercd = mci_wai_trn(pmci, 30*1000);
				retry++;
			}while(ercd != E_OK && retry < RETRY_COUNT);
		}
	}
#endif
	if(ercd == E_OK)
		return RES_OK;
	else
		return RES_ERROR;
}


/*
 *  FatFs用SDCARDIO制御関数
 */
static int sdcard_diskioctl(void *pif, BYTE Func, void* Buffer)
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

