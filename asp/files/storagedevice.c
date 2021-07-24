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
 *  @(#) $Id: storagedevice.c,v 1.3 2016/04/24 11:50:55 roi Exp $
 */

#include <stdlib.h>
#include <string.h>
#include "kernel_cfg.h"
#include "storagedevice.h"

#define SENSE_DEVICE    (SDEV_ACTIVE|SDEV_INSERTCHK)

/*
 *  Storage Device Mnanagerの全体定義
 */
StorageDeviceHead_t SDeviceHead;

/*
 *  デバイス単位の構造体領域定義
 */
static StorageDevice_t SDevice[NUM_STORAGEDEVICE];



void sdev_init(intptr_t exinf)
{
	unsigned int no;

	for(no = 0 ; no < NUM_STORAGEDEVICE ; no++){
		memset(&SDevice[no], 0, sizeof(StorageDevice_t));
	}
	memset(&SDeviceHead, 0, sizeof(StorageDeviceHead_t));
	SDeviceHead._psd            = &SDevice[0];
	SDeviceHead._default_device = DEFAULT_DEVNO;
    SDeviceHead._sdev_active    = true;
}

void sdev_terminate(void)
{
	SDeviceHead._sdev_active    = false;
}

ER SDMSetupDevice(int16_t devno, StorageDevice_t **ppsdev)
{
	StorageDevice_t *psdev;
	unsigned int    no;

	if(devno < MINIMUM_DEVNO || devno > MAXIMUM_DEVNO)
		return E_ID;
	if(SDeviceHead._num_activedev >= NUM_STORAGEDEVICE)
		return E_OBJ;
	for(no = 0, psdev = SDeviceHead._psd ; no < SDeviceHead._num_activedev ; no++, psdev++){
		if(devno == psdev->_sdev_devno)
			return E_ID;
	}
	psdev->_sdev_attribute = SDEV_ACTIVE;
	psdev->_sdev_devno     = devno;
	*ppsdev = psdev;
	SDeviceHead._num_activedev++;
	return E_OK;
}

ER_ID SDMGetDeviceNo(const char **ppathname)
{
	ER_ID devno = DEFAULT_DEVNO;
	const char *p = *ppathname;

	if(*p >= '0'&& *p <= '9'){
		devno = *p++ - '0';
		while(*p >= '0' && *p <= '9'){
			devno *= 10;
			devno  = *p++ - '0';
		}
		if(devno < MINIMUM_DEVNO || devno > MAXIMUM_DEVNO)
			devno = E_ID;
	}
	if(*p == ':')
		p++;
	*ppathname = p;
	return devno;
}

StorageDevice_t *SDMGetStorageDevice(int devno)
{
	StorageDevice_t *psdev = SDeviceHead._psd;
	unsigned int no;

    for(no = 0 ; no < SDeviceHead._num_activedev ; no++){
		if((psdev->_sdev_attribute & SDEV_ACTIVE) != 0
			 && psdev->_sdev_devno == devno)
			return psdev;
		psdev++;
	}
	return NULL;
}

ER SDMEmploy(StorageDevice_t *psdev, bool_t sw)
{
	if(psdev == 0)
		return E_PAR;
	if((psdev->_sdev_attribute & SDEV_ACTIVE) == 0)
		return E_PAR;
	if(sw)
		psdev->_sdev_attribute |= SDEV_EMPLOY;
	else
		psdev->_sdev_attribute &= ~SDEV_EMPLOY;
	return E_OK;
}

void SDMSence_task(intptr_t exinf)
{
	StorageDevice_t *psdev;
	unsigned int dno;
	bool_t       employ;

	while(SDeviceHead._sdev_active == true){
		psdev = SDeviceHead._psd;
		for(dno = 0 ; dno < SDeviceHead._num_activedev ; dno++, psdev++){
			if((psdev->_sdev_attribute & SENSE_DEVICE) == SENSE_DEVICE){
				employ = (psdev->_sdev_attribute & SDEV_EMPLOY) != 0;
				if(!employ && psdev->pdevf->_sdevf_sense(psdev, employ) != 0){
					psdev->_sdev_attribute |= SDEV_EMPLOY;
                    if(psdev->_sdev_notice)
						psdev->_sdev_notice(psdev, true);
				}
				else if((psdev->_sdev_attribute & SDEV_CHKREMOVE) != 0 && employ){
					if(psdev->pdevf->_sdevf_sense(psdev, employ) != 0){
						psdev->_sdev_attribute &= ~SDEV_EMPLOY;
						psdev->_sdev_attribute |= SDEV_ONEEXIT;
						psdev->_sdev_instimer   = 0;
	                    if(psdev->_sdev_notice)
							psdev->_sdev_notice(psdev, false);
					}
				}
			}
		}
		dly_tsk(SENSE_TIME);
#ifdef SDEV_SENSE_ONETIME
		break;
#endif
	}
}


