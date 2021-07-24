/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2008 by Ryosuke Takeuchi
 *                     GJ Business Division RICOH COMPANY,LTD. JAPAN
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
 *  @(#) $Id: armv4.c,v 1.1 2009/08/09 09:44:29 roi Exp $
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

extern void _kernel_break_wait();

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
	uint32_t spsr;
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t ip;
	uint32_t lr;
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
 * ARM4のメモリマッピング
 */

static T_MEMDEF const memdefine[] = {
#if defined(__AT91SAM7S128__)
	{0x00000000, 0x0001FFFF,           MEMORY_AREA, MREAD_ONLY },
	{RAM_START,  RAM_START+RAM_SIZE-1, MEMORY_AREA, MREAD_WRITE},
	{0xFFFF0000, 0xFFFFFFFF,           PORT_AREA,   MREAD_WRITE}
#elif defined(MINDSTORMSNXT)
	{0x00000000, RAM_SIZE-1,           MEMORY_AREA, MREAD_ONLY },
	{0x001C0000, 0x001FFFFF,           MEMORY_AREA, MREAD_ONLY },
	{RAM_START,  RAM_START+RAM_SIZE-1, MEMORY_AREA, MREAD_WRITE},
	{0xFFFF0000, 0xFFFFFFFF,           PORT_AREA,   MREAD_WRITE}
#elif defined(__LPC2388__)
	{0x00000000, 0x0007FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x3FFFC000, 0x3FFFFFFF,           PORT_AREA,   MREAD_WRITE},
	{RAM_START,  RAM_START+RAM_SIZE-1, MEMORY_AREA, MREAD_WRITE},
	{0x80000000, 0x8000FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0x81000000, 0x8100FFFF,           MEMORY_AREA, MREAD_ONLY },
	{0xE0000000, 0xE008FFFF,           PORT_AREA,   MREAD_WRITE},
	{0xE01FC000, 0xE01FFFFF,           PORT_AREA,   MREAD_WRITE},
	{0xFFEF0000, 0xFFFFFFFF,           PORT_AREA,   MREAD_WRITE}
#elif (__TARGET_ARCH_ARM == 5)
	{0x01BC0000, 0x01BC0FFF,           MEMORY_AREA, MREAD_WRITE},
	{0x01BC1000, 0x01BC17FF,           PORT_AREA,   MREAD_WRITE},
	{0x01C00000, 0x01C07FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01C08000, 0x01C087FF,           PORT_AREA,   MREAD_WRITE},
	{0x01C10000, 0x01C11FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01C14000, 0x01C14FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01C20000, 0x01C23FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01C40000, 0x01C42FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01D00000, 0x01D02FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01D0C000, 0x01D0DFFF,           PORT_AREA,   MREAD_WRITE},
	{0x01D10000, 0x01D11FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E00000, 0x01E0FFFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E10000, 0x01E10FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E13000, 0x01E1BFFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E20000, 0x01E28FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E2C000, 0x01E2CFFF,           PORT_AREA,   MREAD_WRITE},
	{0x01E30000, 0x01E383FF,           PORT_AREA,   MREAD_WRITE},
	{0x01F00000, 0x01F03FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01F06000, 0x01F08FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01F0C000, 0x01F0EFFF,           PORT_AREA,   MREAD_WRITE},
	{0x01F10000, 0x01F11FFF,           PORT_AREA,   MREAD_WRITE},
	{0x01C40000, 0x01C42FFF,           PORT_AREA,   MREAD_WRITE},
	{0xC0000000, 0xC3FFFFFF,           MEMORY_AREA, MREAD_WRITE},
	{0xFFFD0000, 0xFFFDFFFF,           MEMORY_AREA, MREAD_ONLY },
	{0xFFFEE000, 0xFFFEFFFF,           PORT_AREA,   MREAD_WRITE},
	{0xFFFF0000, 0xFFFF1FFF,           MEMORY_AREA, MREAD_WRITE}
#else
#error "No board type in ARMV4 groups."
#endif
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
		if(rtst.tskpc == (FP)_kernel_break_wait){
			sreg = *((T_REG *)rtst.tsksp);
			printf(" PC =%08lx SP =%08x IR =%08x", (long)sreg.pc, (UW)((long)rtst.tsksp+sizeof(T_REG)), sreg.lr);
			printf(" IP =%08x SR=%08x\n", sreg.ip, sreg.spsr);
			printf(" R0 =%08x R1 =%08x R2 =%08x", sreg.r0, sreg.r1, sreg.r2);
			printf(" R3 =%08x R4 =%08x R5 =%08x\n", sreg.r3, sreg.r4, sreg.r5);
			printf(" R6 =%08x R7 =%08x R8 =%08x", sreg.r6, sreg.r7, sreg.r8);
			printf(" R9 =%08x R10=%08x R11=%08x\n", sreg.r9, sreg.r10, sreg.r11);
			printf("     %08lx    %08x\n", (long)sreg.pc, *((UW*)((sreg.pc+3) & ~3)));
			return;
		}
		else if(rtst.tskstat == TTS_DMT){
			printf("  wait in activate_r() !!\n");
			return;
		}
	}
	printf("  wait in dispatch() !!\n");
}

