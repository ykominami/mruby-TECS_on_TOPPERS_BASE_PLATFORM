/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2017 by TOPPERS PROJECT
 *                             Educational Working Group.
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
 *  @(#) $Id: tusbh_rtos.h 698 2018-04-04 21:04:00Z roi $
 */
/*
 *  USB Middleware RTOS依存部定義
 */

#ifndef _TUSB_RTOS_H_
#define _TUSB_RTOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "t_syslog.h"
#include "device.h"

#ifdef CACHE_LINE_SIZE
#define USB_DATA_ALIGN      CACHE_LINE_SIZE
#else
#define USB_DATA_ALIGN      4
#endif

#define TUSB_PROCESS_PRIO       6
#define TUSB_PROCESS_STACK_SIZE 4096
#define TUSB_HANDLE_STACK_SIZE  1024

/*
 *  SYSSVCメモリ管理関数
 */
extern void *malloc_cache(uint32_t len);
extern void free_cache(void *addr);


extern ER tusbSendData(ID qid, uint8_t evt, uint8_t prm1, uint8_t prm2, uint8_t prm3);
extern ER tusbiSendData(ID qid, uint8_t evt, uint8_t prm1, uint8_t prm2, uint8_t prm3);
extern ER tusbRecvData(ID qid, uint8_t *pmes, uint32_t timeout);
extern ER tusbStartTask(ID taskid);
extern int32_t tusbGetTaskID(void);
extern ER tusbTimerControl(uint8_t activate);

#define tusbmalloc          malloc_cache
#define tusbfree            free_cache
extern void tusbmemset(void *d, const char val, int len);
extern void tusbmemcpy(void *d, void *s, int len);

#define tusbCpuLock         loc_cpu
#define tusbCpuUnLock       unl_cpu
#define tusbDelay(t)        dly_tsk((t))
#define tusbSleep(t)        tslp_tsk((t))
#define tusbWakeup(id)      wup_tsk((id))


#endif /* _TUSB_RTOS_H_ */

