/*
 *  TOPPERS/ASP Educative Program
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Just Standard Profile Kernel
 * 
 *  Copyright (C) 2003-2008 by Ryosuke Takeuchi
 *               Platform Development Center RICOH COMPANY,LTD. JAPAN
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
 *  @(#) $Id: h8300h.c,v 1.1 2008/07/06 17:04:29 roi Exp $
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
#ifdef SUPPORT_NETDEV
#include "netdevice.h"
#endif
#include "monitor.h"

extern void _kernel_break_wait();
#ifdef SUPPORT_NETDEV
static UB save_ier;
extern unsigned long vector[];
#endif

/*
 * レジスタの構造体
 */
typedef struct t_reg{
	uint32_t er6;
	uint32_t er5;
	uint32_t er4;
	uint32_t er3;
	uint32_t er2;
	uint32_t er1;
	uint32_t er0;
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
 * AKIH8のメモリマッピング
 */

static T_MEMDEF const memdefine[] = {
	0x00000000, 0x000000FF, MEMORY_AREA, MREAD_ONLY,
#if defined(__H8_3048F__)
	0x00000100,	0x0001FFFF, MEMORY_AREA, MREAD_ONLY,
	0x000FEF10, 0x000FFF0F, MEMORY_AREA, MREAD_WRITE,
	0x00FFFF1C, 0x00FFFFFF, PORT_AREA,   MREAD_WRITE
#elif defined(__H8_3069F__)
	0x00000100,	0x0007FFFF, MEMORY_AREA, MREAD_ONLY,
	0x00400000, 0x005FFFFF, MEMORY_AREA, MREAD_WRITE,
	0x00FEE000, 0x00FEEFFF, PORT_AREA,   MREAD_WRITE,
	0x00FFBF20, 0x00FFFF1F, MEMORY_AREA, MREAD_WRITE,
	0x00FFFF20, 0x00FFFFFF, PORT_AREA,   MREAD_WRITE
#else
#error "No board type in H8300H groups."
#endif
};

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
#ifdef SUPPORT_NETDEV
            if(!NetDeviceRead((unsigned long)address, len, (void *)p))
#endif
			*((UH *)p) = sil_reh_mem((VP)address);
		}
		else if(type == 4){
			len = 4;
#ifdef SUPPORT_NETDEV
            if(!NetDeviceRead((unsigned long)address, len, (void *)p))
#endif
			*((UW *)p) = sil_rew_mem((VP)address);
		}
		else{
#ifdef SUPPORT_NETDEV
			len = 1;
			if(NetDeviceActive() && address == H8IER)
				*((UB *)p) = save_ier;
			else if(address == H8IPRA || address == H8IPRB)
				*((UB *)p) = sil_reb_mem((VP)address);
            else if(!NetDeviceRead((unsigned long)address, len, (void *)p))
				*((UB *)p) = sil_reb_mem((VP)address);
#else
			len = 1;
			*((UB *)p) = sil_reb_mem((VP)address);
#endif
		}
		break;
	case MEMORY_AREA:
		if(type == 2){
			len = 2;
			*((UH *)p) = *((UH *)address);
		}
		else if(type == 4){
			len = 4;
			*((UW *)p) = *((UW *)address);
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
#ifdef SUPPORT_NETDEV
            if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
#endif
			sil_wrh_mem((VP)address, *((UH *)p));
		}
		else if(type == 4){
			len = 4;
#ifdef SUPPORT_NETDEV
            if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
#endif
			sil_wrw_mem((VP)address, *((UW *)p));
		}
		else{
#ifdef SUPPORT_NETDEV
			len = 1;
			if(NetDeviceActive() && address == H8IER){
				save_ier = *((UB *)p);
				if(save_ier & (1<<H8IER_IRQ1E_BIT))
					NetDeviceSetInt(IRQ_EXT1, (void (*)(VP))vector[IRQ_EXT1]);
				else
					NetDeviceSetInt(IRQ_EXT1, 0);
				if(save_ier & (1<<H8IER_IRQ2E_BIT))
					NetDeviceSetInt(IRQ_EXT2, (void (*)(VP))vector[IRQ_EXT2]);
				else
					NetDeviceSetInt(IRQ_EXT2, 0);
			}
			else if(address == H8IPRA || address == H8IPRB)
				sil_wrb_mem((VP)address, *((UB *)p));
            else if(!NetDeviceWrite((unsigned long)address, len, (void *)p))
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
			*((UH *)address) = *((UH *)p);
		}
		else if(type == 4){
			len = 4;
			*((UW *)address) = *((UW *)p);
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
 *  エクセプションの引数よりpcを取り出す関数
 */
ulong_t
get_exception_pc(void * p_excinf)
{
	ulong_t pc;

	pc = *(ulong_t*)((char*)p_excinf+16) & 0xffffff;
	return pc;
}

/*
 *  レジスタ内容の表示
 */
void
display_registers(ID tskid)
{
	ER       ercd;
	T_RTST   rtst;
	T_REG    reg;
	uint32_t pc;
	uint8_t  ccr;

	ercd = ref_tst(tskid, &rtst);
	printf("tskid=%d ercd=%d rtst.tskpc=0x%x rtst.tsksp=0x%x\n", tskid, ercd, (int)rtst.tskpc, (int)rtst.tsksp);

	if(ercd == E_OK){
		if(rtst.tskpc == (FP)_kernel_break_wait){
			reg = *((T_REG *)rtst.tsksp);
			pc  = reg.pc & 0xffffff;
			ccr = reg.pc >> 24;
			printf("  PC=%08lx SP=%08lx", (unsigned long)pc, (unsigned long)rtst.tsksp+sizeof(T_REG));
			printf(" I=%1x UI=%1x H=%1x", (ccr>>7) & 1, (ccr>>6) & 1, (ccr>>5) & 1);
			printf(" U=%1x N=%1x Z=%1x", (ccr>>4) & 1, (ccr>>3) & 1, (ccr>>2) & 1);
			printf(" V=%1x C=%1x    R6=%08x\n", (ccr>>1) & 1, ccr & 1, reg.er6);
			printf("  R0=%08x R1=%08x R2=%08x", reg.er0, reg.er1, reg.er2);
			printf(" R3=%08x R4=%08x R5=%08x\n", reg.er3, reg.er4, reg.er5);
			printf("     %08lx    %02x %02x\n", (unsigned long)pc, *((UB*)pc), *((UB*)(pc+1)));
			return;
		}
		else if(rtst.tskstat == TTS_DMT){
			printf("  wait in activate_r() !!\n");
			return;
		}
	}
	printf("  wait in dispatch() !!\n");
}

/******************************************************************************
 * エラー通知用関数
 ******************************************************************************/
/*
 * イレギュラーエクセプション
 */
void
irregular_ext_handler(void * p_excinf)
{
	uint32_t pc = get_exception_pc(p_excinf);

	if(p_runtsk){
		if(_kernel_intnest > 1){
			syslog_2(LOG_EMERG, "Irregular Exception occured in not task Context pc=0x%x p_excinf=0x%x !", pc, p_excinf);
			target_exit();
		}
		else{
			syslog_3(LOG_ERROR, "Irregular Exception occured in tskid=%d pc=0x%x p_excinf=0x%x !", TSKID(p_runtsk), pc, p_excinf);
			isus_tsk(TSKID(p_runtsk));
		}
	}
	else{
		syslog_2(LOG_EMERG, "Irregular Exception occured in Idle pc=0x%x p_excinf=0x%x !", pc, p_excinf);
		target_exit();
	}
}

