/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2016 by Ryosuke Takeuchi
 *                     GJ Business Division RICOH COMPANY,LTD. JAPAN
 *  Copyright (C) 2017-2018 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，Free Software Foundation によって公表されている 
 *  GNU General Public License の Version 2 に記述されている条件か，以
 *  下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェア（本ソフトウェ
 *  アを改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを再利用可能なバイナリコード（リロケータブルオブ
 *      ジェクトファイルやライブラリなど）の形で利用する場合には，利用
 *      に伴うドキュメント（利用者マニュアルなど）に，上記の著作権表示，
 *      この利用条件および下記の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを再利用不可能なバイナリコードの形または機器に組
 *      み込んだ形で利用する場合には，次のいずれかの条件を満たすこと．
 *    (a) 利用に伴うドキュメント（利用者マニュアルなど）に，上記の著作
 *        権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 利用の形態を，別に定める方法によって，上記著作権者に報告する
 *        こと．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者を免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者は，
 *  本ソフトウェアに関して，その適用可能性も含めて，いかなる保証も行わ
 *  ない．また，本ソフトウェアの利用により直接的または間接的に生じたい
 *  かなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: armv7.c,v 1.4 2018/05/20 18:50:54 roi Exp $
 */

/* 
 *  TOPPERS/ASP用タスクモニタCPU依存プログラム．
 *
 */

#include <itron.h>
#include <sil.h>
#include <stdio.h>
#include "kernel_impl.h"
#include "task.h"
#include "task_expansion.h"
#include "monitor.h"

extern void dispatch_r(void);

#define set_align(a, b)	((ulong_t)a & ~(b-1))

/*
 * レジスタの構造体
 */
typedef struct t_reg{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t pc;
}T_REG;

/*
 * メモリのマッピング定義構造体
 */

typedef struct t_memdef{
	uint32_t mstart;
	uint32_t mend;
	uint8_t  mtype;
	uint8_t  mstate;
}T_MEMDEF;

/*
 * ARMv7のメモリマッピング
 */

static T_MEMDEF const memdefine[] = {
#if defined(TOPPERS_CQ_FRK_FM3)
	{0x00000000, 0x000FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x1FFF0000, 0x201EFFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x400FFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F091_NUCLEO64)	/* ARMV6m */
	{0x08000000, 0x0803FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x20007FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32L073_NUCLEO64) || defined(TOPPERS_STM32LORA_DISCOVERY)	/* ARMV6m */
	{0x08000000, 0x0802FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x20004FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32L476_NUCLEO64) || defined(TOPPERS_STM32L476_DISCOVERY)
	{0x08000000, 0x080FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x10000000, 0x10007FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x20000000, 0x20017FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0x90000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F4_DISCOVERY) || defined(TOPPERS_STM32_E407)
	{0x08000000, 0x080FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2001FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F401_NUCLEO)
	{0x08000000, 0x0807FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x20017FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F446_NUCLEO64) || defined(TOPPERS_STM32F446_NUCLEO144)
	{0x08000000, 0x0807FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2001FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F429_BOARD)
	{0x08000000, 0x080FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2002FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0xA0000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
#elif defined(TOPPERS_STM32F723_DISCOVERY)
	{0x00000000, 0x0003FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x08000000, 0x0807FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2003FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0x90000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
	{0xC0000000, 0xC07FFFFF,           MEMORY_AREA, MREAD_WRITE},
#elif defined(TOPPERS_STM32F7_DISCOVERY) ||defined(TOPPERS_STM32F746_NUCLEO144)
	{0x00000000, 0x0003FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x08000000, 0x080FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2004FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0x90000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
	{0xC0000000, 0xC07FFFFF,           MEMORY_AREA, MREAD_WRITE},
#elif defined(TOPPERS_STM32F769_DISCOVERY) || defined(TOPPERS_STM32F767_NUCLEO144)
	{0x00000000, 0x0003FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x08000000, 0x081FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x20000000, 0x2007FFFF,           MEMORY_AREA, MREAD_WRITE},
	{0x40000000, 0x50060FFF,           PORT_AREA,   MREAD_WRITE},
	{0x90000000, 0xBFFFFFFF,           PORT_AREA,   MREAD_WRITE},
	{0xC0000000, 0xC07FFFFF,           MEMORY_AREA, MREAD_WRITE},
#else
#error "No support board type in ARMV7-M groups."
#endif
	{0xE0000000, 0xFFFFFFFF,           PORT_AREA,   MREAD_WRITE}
};

static T_REG  sreg;

/******************************************************************************
 * ハードウェアポート属性参照関数
 ******************************************************************************/
/*
 *  アドレスからメモリ領域属性を取り出す
 *  mode=0:領域の型
 *  mode=1:読み取り書き込み属性
 */

char
getMemoryType(ulong_t address, int_t mode)
{
	int_t count = sizeof(memdefine) / sizeof(T_MEMDEF);
	int_t i;

	for(i = 0 ; i < count ; i++){
		if(address >= memdefine[i].mstart && address <= memdefine[i].mend){
			if(mode == 0)
				return memdefine[i].mtype;
			else
				return memdefine[i].mstate;
		}
	}
	return NONE_AREA;
}

/*
 *  アドレスからアライン後のアドレスを取り出す
 */

ulong_t
MonAlignAddress(ulong_t address)
{
	return address;
}

/******************************************************************************
 * メモリアクセス用関数
 ******************************************************************************/
/*
 *  メモリ領域に対する読み出し関数
 *  領域のチェックを行い、エラーならゼロを返す
 */
int_t
MemoryRead(ulong_t address, intptr_t p, int_t type)
{
	int_t len;

	switch(getMemoryType(address, 0)){
	case PORT_AREA:
		if(type == 2){
			len = 2;
			*((UH *)p) = sil_reh_mem((VP)address);
		}
		else if(type == 4){
			len = 4;
			*((UW *)p) = sil_rew_mem((VP)address);
		}
		else{
			len = 1;
			*((UB *)p) = sil_reb_mem((VP)address);
		}
		break;
	case MEMORY_AREA:
		if(type == 2){
			len = 2;
			*((UH *)p) = *((UH *)set_align(address, len));
		}
		else if(type == 4){
			len = 4;
			*((UW *)p) = *((UW *)set_align(address, len));
		}
		else{
			len = 1;
			*((UB *)p) = *((UB *)address);
		}
		break;
	default:
		len = 0;
		break;
	}
	return len;
}

/*
 *  メモリ領域に対する書き込み関数
 *  領域のチェックを行い、エラーならゼロを返す
 */
int_t
MemoryWrite(ulong_t address, intptr_t p, int_t type)
{
	int_t len;

	switch(getMemoryType(address, 0)){
	case PORT_AREA:
		if(type == 2){
			len = 2;
			address = set_align(address, len);
			sil_wrh_mem((VP)address, *((UH *)p));
		}
		else if(type == 4){
			len = 4;
			address = set_align(address, len);
			sil_wrw_mem((VP)address, *((UW *)p));
		}
		else{
			len = 1;
			sil_wrb_mem((VP)address, *((UB *)p));
		}
		break;
	case MEMORY_AREA:
		if(getMemoryType(address, 1) == MREAD_ONLY){
			len = 0;
		}
		else if(type == 2){
			len = 2;
			*((UH *)address) = *((UH *)set_align(p, len));
		}
		else if(type == 4){
			len = 4;
			*((UW *)address) = *((UW *)set_align(p, len));
		}
		else{
			len = 1;
			*((UB *)address) = *((UB *)p);
		}
		break;
	default:
		len = 0;
		break;
	}
	return len;
}

/******************************************************************************
 * モニタ用関数
 ******************************************************************************/
/*
 *  レジスタ内容の表示
 */
void
display_registers(ID tskid)
{
	ER     ercd;
	T_RTST rtst;

	ercd = ref_tst(tskid, &rtst);
	if(ercd == E_OK){
		if(rtst.tskpc == (FP)dispatch_r){
			sreg = *((T_REG *)rtst.tsksp);
			printf(" PC  =%08lx SP =%08x\n", (long)sreg.pc, (UW)((long)rtst.tsksp+sizeof(T_REG)));
			printf(" R4  =%08x R5 =%08x R6  =%08x R7  =%08x\n", sreg.r4, sreg.r5, sreg.r6, sreg.r7);
			printf(" R8  =%08x R9 =%08x R10 =%08x R11 =%08x\n", sreg.r8, sreg.r9, sreg.r10, sreg.r11);
			printf("     %08lx    %04x\n", (long)sreg.pc, *((UH*)(sreg.pc & ~1)));
			return;
		}
		else if(rtst.tskstat == TTS_DMT){
			printf("  wait in activate_r() !!\n");
			return;
		}
	}
	printf("  wait in dispatch() !!\n");
}

