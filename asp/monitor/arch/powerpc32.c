/*
 *  TOPPERS/ASP Educative Program
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Just Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2009 by Ryosuke Takeuchi
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
 *  @(#) $Id: powerpc32.c,v 1.2 2009/05/08 16:20:29 roi Exp $
 */

/* 
 *  TOPPERS/JSP用タスクモニタCPU依存プログラム．
 *
 */

#include <itron.h>
#include <sil.h>
#include <stdio.h>
#include "kernel_impl.h"
#include "task.h"
#include "task_expansion.h"
#ifdef SUPPORT_ETHER
#include "netdevice.h"
#endif
#include "monitor.h"

extern void _kernel_break_wait();

#define set_align(a, b)	((UW)a & ~(b-1))

/*
 * レジスタの構造体
 */
typedef struct t_reg{
#ifdef SUPPORT_FLOATING_POINT_REG
	UW  memo_msr;
	UW  memo_off;
	UW  fps[33*2+1];
#endif	/* SUPPORT_FLOATING_POINT_REG */
	UW  r13;
	UW  r14;
	UW  r15;
	UW  r16;
	UW  r17;
	UW  r18;
	UW  r19;
	UW  r20;
	UW  r21;
	UW  r22;
	UW  r23;
	UW  r24;
	UW  r25;
	UW  r26;
	UW  r27;
	UW  r28;
	UW  r29;
	UW  r30;
	UW  r31;
	UW  lr2;

	UW	r0;
	UW	r3;
	UW	r4;
	UW	r5;
	UW	r6;
	UW	r7;
	UW	r8;
	UW	r9;
	UW	r10;
	UW	r11;
	UW	r12;
	UW	srr0;
	UW	srr1;
	UW  lr;
	UW	ctr;
	UW	cr;
	UW	xer;
	UW  dummy;
}T_REG;

/*
 * メモリのマッピング定義構造体
 */

typedef struct t_memdef{
	UW	mstart;
	UW	mend;
	UB	mtype;
	UB	mstate;
}T_MEMDEF;

/*
 * POWERPC32のメモリマッピング
 */

static T_MEMDEF const memdefine[] = {
	{0x00000000, 0x0FFFFFFF, MEMORY_AREA, MREAD_WRITE},
#ifdef SUPPORT_MIRROR_MEMORY
	{0x20000000, 0x2FFFFFFF, MEMORY_AREA, MREAD_WRITE},
#endif
#if defined(SANTAMARIA)
	{0xEC000000, 0xEC7FFFFF, MEMORY_AREA, MREAD_ONLY },
	{0xFF400000, 0xFF4FFFFF, PORT_AREA,   MREAD_WRITE},
	{0xFFF00000, 0xFFFFFFFF, MEMORY_AREA, MREAD_ONLY }
#elif defined(EP440XC)
	{0xE0000000, 0xE00004FF, PORT_AREA,   MREAD_ONLY },
	{0xE0010000, 0xE0013FFF, MEMORY_AREA, MREAD_WRITE},
	{0xEF600000, 0xEF6011FF, PORT_AREA,   MREAD_WRITE},
	{0xFFF00000, 0xFFFFFFFF, MEMORY_AREA, MREAD_ONLY }
#elif defined(EP460XC)
	{0xD8000000, 0xD8FFFFFF, PORT_AREA,   MREAD_WRITE},
	{0xE2000000, 0xE20FFFFF, PORT_AREA,   MREAD_WRITE},
	{0xEF600000, 0xEF6011FF, PORT_AREA,   MREAD_WRITE},
	{0xFFF00000, 0xFFFFFFFF, MEMORY_AREA, MREAD_ONLY }
#else
#error "No board type in POWERPC32 groups."
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

UB
getMemoryType(UW address, INT mode)
{
	INT count = sizeof(memdefine) / sizeof(T_MEMDEF);
	INT i;

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

UW
MonAlignAddress(UW address)
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
INT
MemoryRead(UW address, VP_INT p, INT type)
{
	INT  len;

	switch(getMemoryType(address, 0)){
	case PORT_AREA:
		if(type == 2){
			len = 2;
#ifdef PANEL_SIMULATION
            if(!NetDeviceRead((unsigned long)address, len, (void *)p))
#endif
			asm("eieio");
			*((UH *)p) = sil_reh_mem((VP)address);
		}
		else if(type == 4){
			len = 4;
#ifdef PANEL_SIMULATION
            if(!NetDeviceRead((unsigned long)address, len, (void *)p))
#endif
			*((UW *)p) = sil_rew_mem_ppc((VP)address);
		}
		else{
#ifdef PANEL_SIMULATION
			len = 1;
            if(!NetDeviceRead((unsigned long)address, len, (void *)p))
				*((UB *)p) = sil_reb_mem_ppc((VP)address);
#else
			len = 1;
			*((UB *)p) = sil_reb_mem_ppc((VP)address);
#endif
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
INT
MemoryWrite(UW address, VP_INT p, INT type)
{
	INT  len;

	switch(getMemoryType(address, 0)){
	case PORT_AREA:
		if(type == 2){
			len = 2;
			address = set_align(address, len);
#ifdef PANEL_SIMULATION
            if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
#endif
			sil_wrh_mem((VP)address, *((UH *)p));
		}
		else if(type == 4){
			len = 4;
			address = set_align(address, len);
#ifdef PANEL_SIMULATION
            if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
#endif
			sil_wrw_mem((VP)address, *((UW *)p));
		}
		else{
#ifdef PANEL_SIMULATION
			len = 1;
            if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
				sil_wrb_mem((VP)address, *((UB *)p));
#else
			len = 1;
			sil_wrb_mem((VP)address, *((UB *)p));
#endif
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
			sreg = *((T_REG *)(rtst.tsksp-4));
			printf(" PC =%08lx SP =%08x MSR =%08x", (long)sreg.srr0, (UW)((long)rtst.tsksp+sizeof(T_REG)), sreg.srr1);
			printf(" LR =%08x CTR=%08x CR=%08x\n", sreg.lr, sreg.ctr, sreg.cr);
			printf(" R0 =%08x R3 =%08x R4 =%08x", sreg.r0, sreg.r3, sreg.r4);
			printf(" R5 =%08x R6 =%08x R7 =%08x\n", sreg.r5, sreg.r6, sreg.r7);
			printf(" R8 =%08x R9 =%08x R10=%08x", sreg.r8, sreg.r9, sreg.r10);
			printf(" R11=%08x R12=%08x R13=%08x\n", sreg.r11, sreg.r12, sreg.r13);
			printf(" R14=%08x R15=%08x R16=%08x\n", sreg.r14, sreg.r15, sreg.r16);
			printf(" R17=%08x R18=%08x R19=%08x\n", sreg.r17, sreg.r18, sreg.r19);
			printf(" R20=%08x R21=%08x R22=%08x\n", sreg.r20, sreg.r21, sreg.r22);
			printf(" R23=%08x R24=%08x R25=%08x\n", sreg.r23, sreg.r24, sreg.r25);
			printf(" R26=%08x R27=%08x R28=%08x\n", sreg.r26, sreg.r27, sreg.r28);
			printf(" R29=%08x R30=%08x R31=%08x\n", sreg.r29, sreg.r30, sreg.r31);
			printf(" XER=%08x\n", sreg.xer);
			printf("     %08lx    %08x\n", (long)sreg.srr0, *((UW*)sreg.srr0));
			return;
		}
		else if(rtst.tskstat == TTS_DMT){
			printf("  wait in activate_r() !!\n");
			return;
		}
	}
	printf("  wait in dispatch() !!\n");
}


void __va_arg_type_violation(void)
{}

