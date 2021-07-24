/*
 *  TOPPERS/ASP/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
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
 *  @(#) $Id: malloc.c 698 2017-06-02 08:48:16Z roi $
 */
/*
 * 
 *  メモリアロケーション機能
 *
 */
#include <kernel.h>
#include <t_syslog.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_cfg.h"
#include "tlsf.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE	    32	/* default cacle line size */
#endif

static void *memory_pool;

/*
 *  プール初期化
 */
void heap_init(intptr_t exinf)
{
	uint32_t *param = (uint32_t *)exinf;
	uint32_t *p;
	int i, size;
	memory_pool = (void*)param[0];
	p = (uint32_t *)param[0];
	size = param[1] / sizeof(uint32_t);
	for(i = 0 ; i < size ; i++)
		*p++ = 0;
	init_memory_pool(param[1], (void*)memory_pool);
}

/*
 *  MALLOC
 */
_PTR	_EXFUN_NOTHROW(malloc,(size_t __size))
{
	void *mempool, *addr;
	mempool = (void *)memory_pool;
	if(mempool == NULL)
		return NULL;
	wai_sem(TLSF_SEM);
	addr = (void*)malloc_ex(__size, mempool);
	sig_sem(TLSF_SEM);
	return addr;
}

/*
 *  CALLOC
 */
_PTR	_EXFUN_NOTHROW(calloc,(size_t __nmemb, size_t __size))
{
	void *mempool, *addr;
	mempool = (void *)memory_pool;
	if(mempool == NULL)
		return NULL;
	wai_sem(TLSF_SEM);
	addr = (void*)malloc_ex(__nmemb * __size, mempool);
	sig_sem(TLSF_SEM);
	memset(addr, 0, __nmemb * __size);
	return addr;
}

/*
 *  FREE
 */
_VOID	_EXFUN_NOTHROW(free,(_PTR ptr))
{
	void *mempool = memory_pool;
	if(mempool != NULL){
		wai_sem(TLSF_SEM);
		free_ex(ptr, mempool);
		sig_sem(TLSF_SEM);
	}
}

/*
 *  CACHE ALINE MALLOC
 */
void *malloc_cache(uint32_t len)
{
	void * addr = malloc(len+CACHE_LINE_SIZE);
	uint32_t *p, uaddr;

	p = addr;
	uaddr = (uint32_t)addr;
	if((uaddr & (CACHE_LINE_SIZE-1)) != 0)
		uaddr &= ~(CACHE_LINE_SIZE-1);
	uaddr += CACHE_LINE_SIZE;
	p = (uint32_t *)uaddr;
	*(p-1) = (uint32_t)addr;
	return p;
}

/*
 *  CACHE ALINE FREE
 */
void free_cache(void *addr)
{
	uint32_t *p = addr;

	addr = (void*)(*(p-1));
	free(addr);
}

