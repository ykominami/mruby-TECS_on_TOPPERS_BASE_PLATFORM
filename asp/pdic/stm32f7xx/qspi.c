/*
 *  TOPPERS/ASP Kernel
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
 *  @(#) $Id: qspi.c 698 2017-08-08 22:04:49Z roi $
 */
/*
 * 
 *  QSPIドライバ関数群
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"
#include "qspi.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  QSPIポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_QSPI(qspiid)      ((uint_t)((qspiid) - 1))

#define GPIO_AF9_QUADSPI        ((uint8_t)0x09)  /* QUADSPI Alternate Function mapping */
#define GPIO_AF10_QUADSPI       ((uint8_t)0x0A)  /* QUADSPI Alternate Function mapping */

#define QPSI_TIMEOUT_DEFAULT_VALUE (5000)	/* 5sec */

/*
 *  QSPI通信レジスタ設定ファンクション定義
 */
#define QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE 0x00000000			/* Indirect write mode*/
#define QSPI_FUNCTIONAL_MODE_INDIRECT_READ  QUADSPI_CCR_FMODE_0	/* Indirect read mode*/
#define QSPI_FUNCTIONAL_MODE_AUTO_POLLING   QUADSPI_CCR_FMODE_1	/* Automatic polling mode*/
#define QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  QUADSPI_CCR_FMODE	/* Memory-mapped mode*/

static const QSPI_PortControlBlock qspi_pcb[NUM_QSPIPORT] = {
#ifdef TOPPERS_STM32F769_DISCOVERY
  {	TADR_QSPI_R_BASE, (TADR_RCC_BASE+TOFF_RCC_AHB1ENR),
	RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, 6,  GPIO_AF10_QUADSPI,
	RCC_AHB1ENR_GPIOBEN, TADR_GPIOB_BASE, 2,  GPIO_AF9_QUADSPI,
	RCC_AHB1ENR_GPIOCEN, TADR_GPIOC_BASE, 9,  GPIO_AF9_QUADSPI,
	RCC_AHB1ENR_GPIOCEN, TADR_GPIOC_BASE, 10, GPIO_AF9_QUADSPI,
	RCC_AHB1ENR_GPIOEEN, TADR_GPIOE_BASE, 2,  GPIO_AF9_QUADSPI,
	RCC_AHB1ENR_GPIODEN, TADR_GPIOD_BASE, 13, GPIO_AF9_QUADSPI
  }
#else
  {	TADR_QSPI_R_BASE, (TADR_RCC_BASE+TOFF_RCC_AHB2ENR),
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 11, GPIO_AF10_QUADSPI,
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 10, GPIO_AF10_QUADSPI,
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 12, GPIO_AF10_QUADSPI,
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 13, GPIO_AF10_QUADSPI,
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 14, GPIO_AF10_QUADSPI,
	RCC_AHB2ENR_GPIOEEN, TADR_GPIOE_BASE, 15, GPIO_AF10_QUADSPI
  }
#endif
};

QSPI_Handle_t QSPIHandle[NUM_QSPIPORT];

/*
 *  QSPIフラグセット待ち関数
 *  parameter1 hi2c    I2Cハンドラへのポインタ
 *  parameter2 flag    フラグ値
 *  parameter3 result  最終フラグ値
 *  parameter4 timeout タイムアウト値(ms)
 *  parameter5 ptimout タイムアウト値設定領域
 *  result     E_OKで正常終了
 */
static ER
qspi_waitflag(QSPI_Handle_t *hqspi, uint32_t flag, uint32_t result, uint32_t timeout, uint32_t *ptimeout)
{
	uint32_t tick = 0;

	while((sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_SR)) & flag) != result){
		if(tick > timeout){
			hqspi->errorcode |= QSPI_ERROR_TIMEOUT;
			return E_TMOUT;
		}
		tick++;
		dly_tsk(1);
	}
	*ptimeout -= tick;
	return E_OK;
}

/*
 *  QSPIイベント待ち
 *  parameter1 hi2c    I2Cハンドラへのポインタ
 *  parameter2 status  変化待ちステータス値
 *  parameter3 timeout タイムアウト値(ms)
 *  result     E_OKで正常終了
 */
static ER
qspi_eventwait(QSPI_Handle_t *hqspi, uint32_t status, uint32_t timeout)
{
	uint32_t tick = 0;

	while(hqspi->status == status){
		if(hqspi->semid != 0)
			twai_sem(hqspi->semid, 1);
		else
			dly_tsk(1);
		tick++;
		if(tick > timeout)
			return E_TMOUT;
	}
	if(hqspi->errorcode != QSPI_ERROR_NONE)
		return E_SYS;
	else
		return E_OK;
}

/*
 *  QSPIデバイスのリセット
 *  parameter1 hqspi QSPIハンドラへのポインタ
 *  return           正常終了時、E_OK
 */
static ER
qspi_reinit(QSPI_Handle_t *hqspi)
{
	QSPI_Init_t *pini = &hqspi->Init;
	ER       ercd = E_OK;
	uint32_t timeout = hqspi->timeout;
	uint32_t i, temp;

	/*
	 *  FIFOスレッシュホールド設定
	 */
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_FTHRES, (pini->fifothreshold - 1) << 8);

	/*
	 *  Ready待ち
	 */
	if((ercd = qspi_waitflag(hqspi, QUADSPI_SR_BUSY, 0, timeout, &timeout)) != E_OK)
		return ercd;

	/*
	 *  QSPI CR設定
	 */
	temp  = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR));
	temp &= ~(QUADSPI_CR_PRESCALER | QUADSPI_CR_SSHIFT);
	temp |= (pini->clockprescaler << 24) | pini->sampleshift;
#ifdef QUADSPI_CR_FSEL
	temp &= ~QUADSPI_CR_FSEL;
	temp |= pini->flashid;
#endif
#ifdef QUADSPI_CR_DFM
	temp &= ~QUADSPI_CR_DFM;
	temp |= pini->dualflash;
#endif
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), temp);

	/*
	 *  QSPI DCR設定
	 */

	for(i = 0 ; i < 32 ; i++){
		if((pini->device_size & (1<<i)) != 0)
			break;
	}
	temp  = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DCR));
	temp &= ~(QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE);
	temp |= (i << 16) | pini->chipselecthightime | pini->clockmode;
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DCR), temp);


    /*
	 *  QSPIイネーブル
	 */
	sil_orw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_EN);

    /*
	 *  QSPIハンドラ設定
	 */
    hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status    = QSPI_STATUS_READY;
	return E_OK;
}


/*
 *  QSPI通信レジスタ設定
 *  parameter1 hqspi QSPIハンドラへのポインタ
 *  parameter2 cmd   コマンドタイプ構造体へのポインタ
 *  parameter3 fmode ファンクションモード
 *  return           正常終了時、E_OK
 */
static void
qspi_config(QSPI_Handle_t *hqspi, QSPI_Command_t *cmd, uint32_t fmode)
{
	QSPI_Init_t *pini = &hqspi->Init;
	uint32_t instruction, alternatebytessize;
	if(cmd->DataMode != QSPI_DATA_NONE && fmode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED){
		/*
		 *  DLRレジスタ設定
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DLR), (cmd->NbData - 1));
	}

	if(cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
		instruction = cmd->Instruction;
	else
		instruction = 0;

	if(cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
		alternatebytessize = cmd->AlternateBytesSize;
	else
		alternatebytessize = 0;

	/*
	 *  ABRレジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_ABR), cmd->AlternateBytes);

	if (cmd->AddressMode != QSPI_ADDRESS_NONE){
		/*
		 *  アドレス設定あり、CCR設定
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CCR), 
								(pini->ddrmode | pini->ddrholdhalfcycle | pini->sioomode | 
								 cmd->DataMode | (cmd->DummyCycles << 18) | 
								 alternatebytessize | cmd->AlternateByteMode | 
								 cmd->AddressSize | cmd->AddressMode | cmd->InstructionMode | 
								 instruction | fmode));
		if (fmode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED){
			/* Configure QSPI: AR register with address value */
			sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_AR), cmd->Address);
		}
	}
	else{
		/*
		 *  アドレス設定なし、CCR設定
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CCR), 
								(pini->ddrmode | pini->ddrholdhalfcycle | pini->sioomode | 
								 cmd->DataMode | (cmd->DummyCycles << 18) | 
								 alternatebytessize | cmd->AlternateByteMode | 
								 cmd->AddressMode | cmd->InstructionMode | 
								 instruction | fmode));
	}
}

/*
 *  QSPIコマンド送信
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 cmd     コマンドタイプ構造体へのポインタ
 *  parameter3 timeout タイムアウト時間(ms)
 *  return           正常終了時、E_OK
 */
ER
qspi_command(QSPI_Handle_t *hqspi, QSPI_Command_t *cmd, uint32_t Timeout)
{
	ER ercd = E_OK;

	if(cmd->InstructionMode != QSPI_INSTRUCTION_NONE){
		if(cmd->Instruction > 0xff)
			return E_PAR;
	}
	if(cmd->AddressMode != QSPI_ADDRESS_NONE){
		if((cmd->AddressSize & ~QUADSPI_CCR_ADSIZE) != 0)
			return E_PAR;
	}
	if(cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE){
		if((cmd->AlternateBytesSize & ~QUADSPI_CCR_ABSIZE) != 0)
			return E_PAR;
	}
	if(cmd->DummyCycles > 31)
		return E_PAR;
	if((cmd->DataMode & ~QUADSPI_CCR_DMODE) != 0)
		return E_PAR;

	if(hqspi->status != QSPI_STATUS_READY)
		return E_OBJ;

	/*
	 *  ステータス、エラー初期化
	 */
	hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status = QSPI_STATUS_BUSY;

	/*
	 *  READY待ち
	 */
	ercd = qspi_waitflag(hqspi, QUADSPI_SR_BUSY, 0, Timeout, &Timeout);
	if(ercd != E_OK)
		return ercd;

	if(cmd->DataMode == QSPI_DATA_NONE){
		/*
		 *  データ転送なしなら、割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTCF));
	}

	/*
	 *  書込みモード通信レジスタ設定
	 */
	qspi_config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

	if(cmd->DataMode == QSPI_DATA_NONE){
        /*
		 *  TCF設定待ち
		 */
		ercd = qspi_waitflag(hqspi, QUADSPI_SR_TCF, QUADSPI_SR_TCF, Timeout, &Timeout);

		if(ercd == E_OK){
			sil_orw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_TEIE | QUADSPI_CR_TCIE));

			/*
			 *  QSPI割込み終了待ち
			 */
			ercd = qspi_eventwait(hqspi, QSPI_STATUS_BUSY, Timeout);
		}
	}
	else	/* データ転送ありなら、正常終了 */
		hqspi->status = QSPI_STATUS_READY;
	return ercd;
}

/*
 *  QSPIオートマチック ポーリングモード設定
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 cmd     コマンドタイプ構造体へのポインタ
 *  parameter3 cfg     オートマチック ポーリングモード設定
 *  parameter4 timeout タイムアウト時間(ms)
 *  return           正常終了時、E_OK
 */
ER
qspi_autopolling(QSPI_Handle_t *hqspi, QSPI_Command_t *cmd, QSPI_AutoPolling_t *cfg, uint32_t Timeout)
{
	ER ercd = E_OK;

	if((cmd->InstructionMode & ~QUADSPI_CCR_IMODE) != 0)
		return E_PAR;
	if(cmd->InstructionMode != QSPI_INSTRUCTION_NONE && cmd->Instruction > 0xFF)
		return E_PAR;
	if((cmd->AddressMode & ~QUADSPI_CCR_ADMODE) != 0)
		return E_PAR;
	if(cmd->AddressMode != QSPI_ADDRESS_NONE && (cmd->AddressSize & ~QUADSPI_CCR_ADSIZE) != 0)
		return E_PAR;
	if((cmd->AlternateByteMode & ~QUADSPI_CCR_ABMODE) != 0)
		return E_PAR;
	if(cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE && (cmd->AlternateBytesSize & ~QUADSPI_CCR_ABSIZE) != 0)
		return E_PAR;
	if(cmd->DummyCycles > 31)
		return E_PAR;
	if((cmd->DataMode & ~QUADSPI_CCR_DMODE) != 0)
		return E_PAR;

	if(cfg->Interval > QUADSPI_PIR_INTERVAL)
		return E_PAR;
	if(cfg->StatusBytesSize < 1 || cfg->StatusBytesSize > 4)
		return E_PAR;
	if((cfg->MatchMode & ~QUADSPI_CR_PMM) != 0)
		return E_PAR;
	if((cfg->AutomaticStop & ~QUADSPI_CR_APMS) != 0)
		return E_PAR;

	if(hqspi->status != QSPI_STATUS_READY)
		return E_OBJ;

	/*
	 *  ステータス、エラー初期化
	 */
	hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status = QSPI_STATUS_BUSY_AP;

	/*
	 *  READY待ち
	 */
	ercd = qspi_waitflag(hqspi, QUADSPI_SR_BUSY, 0, Timeout, &Timeout);
	if(ercd != E_OK)
		return ercd;

	/*
	 *  PSMARレジスタ MATCH値セット
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_PSMAR), cfg->Match);

	/*
	 *  PSMKRレジスタ MASK値セット
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_PSMKR), cfg->Mask);

	/*
	 *  PIRレジスタ インターバル値セット
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_PIR), cfg->Interval);

	/*
	 *  CRレジスタ MATCHモード、オートマチックストップモードセット
	 */
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_PMM | QUADSPI_CR_APMS), (cfg->MatchMode | cfg->AutomaticStop));

	/* Clear interrupt */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), (QUADSPI_FCR_CTEF | QUADSPI_FCR_CSMF));

	/*
	 *  ステータスバイトサイズをセット
	 */
	cmd->NbData = cfg->StatusBytesSize;

	/*
	 *  通信用レジスタを設定
	 */
	qspi_config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_AUTO_POLLING);

	/*
	 *  割込み設定と割込み待ち
	 */
	sil_orw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_TEIE | QUADSPI_CR_SMIE));
	return qspi_eventwait(hqspi, QSPI_STATUS_BUSY_AP, Timeout);
}

/*
 *  QSPIデータ送信
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 pData   送信データ領域へのポインタ
 *  parameter3 timeout タイムアウト時間(ms)
 *  return           正常終了時、E_OK
 */
ER
qspi_transmit(QSPI_Handle_t *hqspi, uint32_t *pData, uint32_t Timeout)
{
	if(pData == NULL)
		return E_PAR;
	if(hqspi->status != QSPI_STATUS_READY)
		return E_OBJ;

	/*
	 *  ステータス、エラー初期化
	 */
	hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status = QSPI_STATUS_BUSY_TX;

	/*
	 *  通信データ設定
	 */
	hqspi->XferCount = 0;
	hqspi->XferSize = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DLR)) + 1;
	hqspi->pBuffPtr = pData;

	/*
	 *  CCRレジスタにインダイレクト書込み設定
	 */
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CCR), QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

	/*
	 *  割込みクリアと割込み設定
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTCF));
	sil_orw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_TEIE | QUADSPI_CR_FTIE | QUADSPI_CR_TCIE));

	return qspi_eventwait(hqspi, QSPI_STATUS_BUSY_TX, Timeout);
}

/*
 *  QSPIデータ受信
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 pData   受信データ領域へのポインタ
 *  parameter3 timeout タイムアウト時間(ms)
 *  return           正常終了時、E_OK
 */
ER
qspi_receive(QSPI_Handle_t *hqspi, uint32_t *pData, uint32_t Timeout)
{
	uint32_t addr_reg = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_AR));

	if(hqspi->status != QSPI_STATUS_READY)
		return E_OBJ;
	if(pData == NULL)
		return E_PAR;

	/*
	 *  ステータス、エラー初期化
	 */
    hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status = QSPI_STATUS_BUSY_RX;

	/*
	 *  通信データ設定
	 */
	hqspi->XferCount = 0;
	hqspi->XferSize = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DLR)) + 1;
	hqspi->pBuffPtr = pData;

	/*
	 *  CCRレジスタにインダイレクト読込み設定
	 */
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CCR), QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

	/*
	 *  ARレジスタを再設定
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_AR), addr_reg);

	/*
	 *  割込みクリアと割込み設定
	 */
	sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTCF));
	sil_orw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_TEIE | QUADSPI_CR_FTIE | QUADSPI_CR_TCIE));

	return qspi_eventwait(hqspi, QSPI_STATUS_BUSY_RX, Timeout);
}


/*
 *  QSPIデバイスの初期化
 *  parameter1 hi2c  I2Cハンドラへのポインタ
 *  return           ハンドラへのポインタ
 */
QSPI_Handle_t *
qspi_init(ID port, QSPI_Init_t *pini)
{
	GPIO_Init_t GPIO_Init_Data;
	QSPI_Init_t *cfg;
	const QSPI_PortControlBlock *qpcb;
	QSPI_Handle_t *hqspi = NULL;
    ER ercd = E_OK;
	uint32_t no, temp;

	if(port < QSPI1_PORTID || port > NUM_QSPIPORT)
		return NULL;
	if(pini == NULL)
		return NULL;
	if((pini->ddrmode & ~QUADSPI_CCR_DDRM) != 0)
		return NULL;
	if((pini->sioomode & ~QUADSPI_CCR_SIOO) != 0)
		return NULL;

	no = INDEX_QSPI(port);
	hqspi = &QSPIHandle[no];
	memset(hqspi, 0, sizeof(QSPI_Handle_t));
	qpcb = &qspi_pcb[no];
	hqspi->base     = qpcb->base;

	/*
	 *  QSPIクロックイネーブル後リセット
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR), RCC_AHB3ENR_QSPIEN);
	temp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR));
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	(void)(temp);

	/*
	 *  GPIOの初期化
	 */
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->giocsclockbit);
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->gioclkclockbit);
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->giod0clockbit);
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->giod1clockbit);
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->giod2clockbit);
	sil_orw_mem((uint32_t *)(qpcb->gioclockbase), qpcb->giod3clockbit);

	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = GPIO_PULLUP;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Data.alternate = qpcb->giocsaf;
	gpio_setup(qpcb->giocsbase, &GPIO_Init_Data, qpcb->giocspin);

	GPIO_Init_Data.pull      = GPIO_NOPULL;
	GPIO_Init_Data.alternate = qpcb->gioclkaf;
	gpio_setup(qpcb->gioclkbase, &GPIO_Init_Data, qpcb->gioclkpin);

	GPIO_Init_Data.alternate = qpcb->giod0af;
	gpio_setup(qpcb->giod0base, &GPIO_Init_Data, qpcb->giod0pin);

	GPIO_Init_Data.alternate = qpcb->giod1af;
	gpio_setup(qpcb->giod1base, &GPIO_Init_Data, qpcb->giod1pin);

	GPIO_Init_Data.alternate = qpcb->giod2af;
	gpio_setup(qpcb->giod2base, &GPIO_Init_Data, qpcb->giod2pin);

	GPIO_Init_Data.alternate = qpcb->giod3af;
	gpio_setup(qpcb->giod3base, &GPIO_Init_Data, qpcb->giod3pin);

	hqspi->timeout = QPSI_TIMEOUT_DEFAULT_VALUE;
	cfg = &hqspi->Init;
	memcpy(cfg, pini, sizeof(QSPI_Init_t));

	if((ercd = qspi_reinit(hqspi)) != E_OK)
		return NULL;

    /*
	 *  初期化終了関数の実行
	 */
    if(pini->init_func != NULL){
		ercd = pini->init_func(hqspi);
		if(ercd != E_OK){
			syslog_0(LOG_ERROR, "qspi_ini: config.init failed.");
			return NULL;
		}
	}
	return hqspi;
}

/*
 *  QSPIデバイスの無効化
 *  parameter1 hqspi QSPIハンドラへのポインタ
 *  return           正常終了時、E_OK
 */
ER
qspi_deinit(QSPI_Handle_t *hqspi)
{
    /*
	 *  QSPI無効化
	 */
	sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_EN);
    hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status    = QSPI_STATUS_RESET;

	/*
	 *  QSPIリセット後クロック停止
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR), RCC_AHB3ENR_QSPIEN);
    return E_OK;
}

/*
 *  インダイレクト・フラッシュ・リード
 *  parameter1 hqspi QSPIハンドラへのポインタ
 *  parameter2 dst   読み込みアドレス
 *  parameter3 src   フラッシュROMスタートアドレス
 *  parameter4 size  読み込みサイズ
 *  return           正常終了時、E_OK
 */
ER
qspi_read(QSPI_Handle_t *hqspi, void * dst, uint32_t src, uint32_t size) 
{
	QSPI_Init_t *cfg = &hqspi->Init;
	QSPI_Command_t  sCommand;
    ER ercd = E_OK;

	if(hqspi == NULL)
		return E_PAR;
	if(src >= cfg->device_size)
		return E_PAR;
	if((src + size - 1) >= cfg->device_size)
		return E_PAR;
	if(size == 0)
		return E_PAR;
	/*
	 *  パラメータアラインチェック
	 */
    if(((uintptr_t)dst & 0x3) || (src & 0x3) || (size & 0x3))
		return E_PAR;

	sCommand.InstructionMode   = hqspi->Init.inst_type;
	sCommand.Instruction       = hqspi->Init.read_op_code;
	sCommand.AddressMode       = hqspi->Init.read_addr_xfer_type;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AddressSize       = hqspi->Init.addr_size;
	sCommand.AlternateBytesSize = 0;
	sCommand.Address           = src;
	sCommand.DataMode          = hqspi->Init.read_data_xfer_type;
	sCommand.NbData            = size;
	sCommand.DummyCycles       = hqspi->Init.read_dummy_cycles;


	if((ercd = qspi_command(hqspi, &sCommand, hqspi->timeout)) != E_OK)
		return ercd;

#ifdef TOPPERS_STM32F769_DISCOVERY
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DCR), QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_1_CYCLE);
#endif

	ercd = qspi_receive(hqspi, dst, hqspi->timeout);
#ifdef TOPPERS_STM32F769_DISCOVERY
	sil_modw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DCR), QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_4_CYCLE);
#endif
	return ercd;
}

/*
 *  インダイレクト・フラッシュ・ライト
 *  parameter1 hqspi QSPIハンドラへのポインタ
 *  parameter2 dst   フラッシュROMスタートアドレス
 *  parameter3 src   書き込みデータアドレス
 *  parameter4 size  書き込みサイズ
 *  return           正常終了時、E_OK
 */
ER
qspi_write(QSPI_Handle_t *hqspi, uint32_t dst, const void * src, uint32_t size) 
{
	QSPI_Init_t *cfg = &hqspi->Init;
	QSPI_Command_t sCommand;
	ER ercd = E_OK;

	if(hqspi == NULL)
		return E_PAR;
	if(dst >= cfg->device_size)
		return E_PAR;
	if((dst + size - 1) >= cfg->device_size)
		return E_PAR;
	if(size == 0)
		return E_PAR;
	/*
	 *  パラメータアラインチェック
	 */
    if(((uintptr_t)src & 0x3) || (dst & 0x3) || (size & 0x3))
		return E_PAR;
	/*
	 *  書込みイネーブル設定
	 */
	if(hqspi->Init.enable_write_func != NULL)
		ercd = hqspi->Init.enable_write_func(hqspi, hqspi->timeout);
	if(ercd != E_OK)
		return ercd;

	/*
	 *  書込みコマンド設定
	 */
	sCommand.InstructionMode   = hqspi->Init.inst_type;
	sCommand.Instruction       = hqspi->Init.write_op_code;
	sCommand.AddressMode       = hqspi->Init.write_addr_xfer_type;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AddressSize       = hqspi->Init.addr_size;
	sCommand.AlternateBytesSize = 0;
	sCommand.Address           = dst;
	sCommand.DataMode          = hqspi->Init.write_data_xfer_type;
	sCommand.NbData            = size;
	sCommand.DummyCycles       = hqspi->Init.write_dummy_cycles;

	if((ercd = qspi_command(hqspi, &sCommand, hqspi->timeout)) != E_OK)
		return ercd;

	if((ercd = qspi_transmit(hqspi, (uint32_t *)src, hqspi->timeout)) != E_OK)
		return ercd;
	/*
	 *  書込み完了待ち
	 */
	if(hqspi->Init.wait_func != NULL)
		ercd = hqspi->Init.wait_func(hqspi, hqspi->timeout);
	return ercd;
}

/*
 *  セクター消去
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 address セクター先頭アドレス
 *  return             正常終了時、E_OK
 */
ER
qspi_erase_sector(QSPI_Handle_t *hqspi, uint32_t address)
{
	QSPI_Command_t sCommand;
	ER ercd = E_OK;

	/*
	 *  書込みイネーブル設定
	 */
	if(hqspi->Init.enable_write_func != NULL)
		ercd = hqspi->Init.enable_write_func(hqspi, hqspi->timeout);
	if(ercd != E_OK)
		return ercd;

	/*
	 *  ERASEコマンド設定
	 */
	sCommand.InstructionMode   = hqspi->Init.inst_type;	/* F */
	sCommand.Instruction       = hqspi->Init.erase_cmds[0];
	sCommand.AddressMode       = hqspi->Init.write_addr_xfer_type;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;	/* F */
	sCommand.AlternateBytesSize = 0;						/* NU */
	sCommand.AddressSize       = hqspi->Init.addr_size;		/* F */
	sCommand.Address           = address;
	sCommand.DataMode          = QSPI_DATA_NONE;
	sCommand.NbData            = 0;
	sCommand.DummyCycles       = 0;

	if((ercd = qspi_command(hqspi, &sCommand, hqspi->timeout)) != E_OK)
		return ercd;

	if(hqspi->Init.wait_func != NULL)
		ercd = hqspi->Init.wait_func(hqspi, hqspi->timeout);
	return ercd;
}

/*
 *  フラッシュメモリの消去
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  parameter2 address 消去領域の先頭アドレス
 *  parameter3 size    消去バイトサイズ
 *  return             正常終了時、E_OK
 */
ER
qspi_erase(QSPI_Handle_t *hqspi, uint32_t address, uint32_t size)
{
	QSPI_Init_t *cfg = &hqspi->Init;
    uint32_t actual_address = address;
    uint32_t actual_size = size;
    uint32_t remainder;
    ER ercd = E_OK;

    /*
	 *  パラメータチェック
	 */
	if(hqspi == NULL)
		return E_PAR;
	if((address + size) > cfg->device_size)
		return E_PAR;
	if((address + size) > cfg->device_size)
		return E_PAR;

    /*
	 *  消去アライン補正
	 */
	remainder = address % cfg->erase_sizes[0];
	if(remainder != 0){
		actual_address -= remainder;
		actual_size += remainder;
	}
	remainder = actual_size % cfg->erase_sizes[0];
	if(remainder != 0){
		remainder = cfg->erase_sizes[0] - remainder;
		actual_size += remainder;
	}

	/*
	 *  補正が必要ならパラメータエラー
	 */
	if((actual_address != address) || (actual_size != size))
		return E_PAR;

	while(size > 0){
		if((ercd = qspi_erase_sector(hqspi, address)) != E_OK)
			break;
		address += cfg->erase_sizes[0];
		size    -= cfg->erase_sizes[0];
	}
	return ercd;
}

/*
 *  ダイレクト・アクセス無効化
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  return             正常終了時、E_OK
 */
ER
qspi_direct_disable(QSPI_Handle_t *hqspi)
{
	/*
	 *  QSPI無効化
	 */
	sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_EN);

	/*
	 *  QSPIリセット後クロック停止
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3RSTR), RCC_AHB3RSTR_QSPIRST);
	return qspi_reinit(hqspi);
}

/*
 *  ダイレクト・アクセス有効化
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  return             正常終了時、E_OK
 */
ER
qspi_direct_enable(QSPI_Handle_t *hqspi)
{
	QSPI_Command_t sCommand;
	uint32_t Timeout = hqspi->timeout;
	ER ercd = E_OK;

	if(hqspi->status != QSPI_STATUS_READY)
		return E_OBJ;

	/*
	 *  読み出しコマンド設定
	 */
	sCommand.InstructionMode   = hqspi->Init.inst_type;
	sCommand.Instruction       = hqspi->Init.read_op_code;
	sCommand.AddressMode       = hqspi->Init.read_addr_xfer_type;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytesSize = 0;
	sCommand.AddressSize       = hqspi->Init.addr_size;
	sCommand.Address           = 0;
	sCommand.DataMode          = hqspi->Init.read_data_xfer_type;
	sCommand.NbData            = 0;
	sCommand.DummyCycles       = hqspi->Init.read_dummy_cycles;

	/*
	 *  READY待ち
	 */
	if((ercd = qspi_waitflag(hqspi, QUADSPI_SR_BUSY, 0, Timeout, &Timeout)) != E_OK)
		return ercd;

	hqspi->errorcode = QSPI_ERROR_NONE;
	hqspi->status = QSPI_STATUS_BUSY_MM;

	/*
	 *  通信レジスタ設定
	 */
	qspi_config(hqspi, &sCommand, QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED);
	return E_OK;
}

/*
 *  QSPI割込みハンドラ
 */
void
qspi_handler(QSPI_Handle_t *hqspi)
{
	uint32_t sr = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_SR));
	uint32_t cr = sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR));

	hqspi->istatus = sr;
	/*
	 *  QSPI FIFOスレッシュ割込み
	 */
	if((sr & QUADSPI_SR_FTF) != 0 && (cr & QUADSPI_CR_FTIE) != 0){
		if(hqspi->status == QSPI_STATUS_BUSY_TX){
			/* Transmission process */
			while((sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_SR)) & QUADSPI_SR_FTF) != 0){
				if(hqspi->XferCount < hqspi->XferSize){
					sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DR), *hqspi->pBuffPtr++);
					hqspi->XferCount += 4;
				}
				else{	/* FIFO割込みマスク */
					sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_FTIE);
					break;
				}
			}
		}
		else if(hqspi->status == QSPI_STATUS_BUSY_RX){
			/* Receiving Process */
			while((sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_SR)) & QUADSPI_SR_FTF) != 0){
				if(hqspi->XferCount < hqspi->XferSize){
					*hqspi->pBuffPtr++ = (uint32_t)sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DR));
					hqspi->XferCount += 4;
				}
				else{	/* FIFO割込みマスク */
					sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), QUADSPI_CR_FTIE);
					break;
				}
			}
		}
	}

	/*
	 *  QSPI転送終了割込み
	 */
	else if((sr & QUADSPI_SR_TCF) != 0 && (cr & QUADSPI_CR_TCIE) != 0){
		/*
		 *  割込みクリア、要因マスク
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), QUADSPI_FCR_CTCF);
		sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_TCIE | QUADSPI_CR_TEIE | QUADSPI_CR_FTIE));

		/* Transfer complete callback */
		if(hqspi->status == QSPI_STATUS_BUSY_TX){
			/* Change state of QSPI */
			hqspi->status = QSPI_STATUS_READY;
			if(hqspi->semid != 0)
				isig_sem(hqspi->semid);
	    }
		else if(hqspi->status == QSPI_STATUS_BUSY_RX){
			while((sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_SR)) & QUADSPI_SR_FLEVEL) != 0){
				if (hqspi->XferCount < hqspi->XferSize){
					*hqspi->pBuffPtr++ = (uint32_t)sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_DR));
					hqspi->XferCount += 4;
				}
				else{
					/* All data have been received for the transfer */
					break;
				}
			}
			/* Change state of QSPI */
			hqspi->status = QSPI_STATUS_READY;
			if(hqspi->semid != 0)
				isig_sem(hqspi->semid);
		}
		else if(hqspi->status == QSPI_STATUS_BUSY){
			/*
			 *  コマンド送信終了
			 */
			hqspi->status = QSPI_STATUS_READY;
			if(hqspi->semid != 0)
				isig_sem(hqspi->semid);
		}
	}

	/*
	 *  QSPI Status Match割込み
	 */
	else if((sr & QUADSPI_SR_SMF) != 0 && (cr & QUADSPI_CR_SMIE) != 0){
		/*
		 *  STATUS MATCH割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), QUADSPI_FCR_CSMF);
		/*
		 *  AUTOMATIC POLL MODE終了
		 */
		if((sil_rew_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR)) & QUADSPI_CR_APMS) != 0){
			/*
			 *  割込みマスクとステータス変更
			 */
			sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_SMIE | QUADSPI_CR_TEIE));
			hqspi->status = QSPI_STATUS_READY;
		}
		if(hqspi->semid != 0)
			isig_sem(hqspi->semid);
	}

	/*
	 *  QSPI Transfer Error割込み
	 */
	else if((sr & QUADSPI_SR_TEF) != 0 && (cr & QUADSPI_CR_TEIE) != 0){
		/* Clear interrupt */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), QUADSPI_FCR_CTEF);

		/* Disable all the QSPI Interrupts */
		sil_andw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_CR), (QUADSPI_CR_SMIE | QUADSPI_CR_TCIE | QUADSPI_CR_TEIE | QUADSPI_CR_FTIE));

		/* Set error code */
		hqspi->errorcode |= QSPI_ERROR_TRANERR;

		/* Change state of QSPI */
		hqspi->status = QSPI_STATUS_READY;

		/*
		 *  エラーコールバック
		 */
		if(hqspi->errorcallback != NULL)
			hqspi->errorcallback(hqspi);
	}

	/*
	 *  QSPI Timeout割込み
	 */
	else if((sr & QUADSPI_SR_TOF) != 0 && (cr & QUADSPI_CR_TOIE) != 0){
		/*
		 *  タイマー割込みクリア
		 */
		sil_wrw_mem((uint32_t *)(hqspi->base+TOFF_QUADSPI_FCR), QUADSPI_FCR_CTOF);

		/*
		 *  タイムアウトコールバック
		 */
		if(hqspi->timeoutcallback != NULL)
			hqspi->timeoutcallback(hqspi);
	}
}


/*
 *  QSPI割込みサービスルーチン
 */
void qspi_isr(intptr_t exinf)
{
	qspi_handler(&QSPIHandle[INDEX_QSPI((uint32_t)exinf)]);
}


