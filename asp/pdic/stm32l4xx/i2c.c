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
 *  @(#) $Id: i2c.c 698 2017-08-03 16:04:31Z roi $
 */
/*
 *
 *  I2Cドライバ関数群(STM32L4XX)
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"
#include "i2c.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  I2CポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_I2C(i2cid)        ((uint_t)((i2cid) - 1))

/*
 *  I2C用AF設定
 */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping */

#define I2C_TIMEOUT_FLAG            35			/* 35 ms */
#define I2C_TIMEOUT_BUSY_FLAG       10000		/* 10 s  */

#define TIMING_CLEAR_MASK           0xF0FFFFFF	/* I2C TIMING clear register Mask */

/*
 *  最大転送サイズ
 */
#define MAX_NBYTE_SIZE              255

/*
 *  I2C_RELOADモード定義
 */
#define  I2C_RELOAD_MODE            I2C_CR2_RELOAD
#define  I2C_AUTOEND_MODE           I2C_CR2_AUTOEND
#define  I2C_SOFTEND_MODE           0x00000000

/*
 *  I2C_XFEROPTION定義
 */
#define I2C_FIRST_FRAME             I2C_SOFTEND_MODE
#define I2C_FIRST_AND_NEXT_FRAME    (I2C_RELOAD_MODE | I2C_SOFTEND_MODE)
#define I2C_NEXT_FRAME              (I2C_RELOAD_MODE | I2C_SOFTEND_MODE)
#define I2C_FIRST_AND_LAST_FRAME    I2C_AUTOEND_MODE
#define I2C_LAST_FRAME              I2C_AUTOEND_MODE
#define I2C_NO_OPTION_FRAME         0xFFFF0000

/*
 *  I2C_START_STOP_MODE定義
 */
#define  I2C_NO_STARTSTOP           (0x00000000U)
#define  I2C_GENERATE_STOP          I2C_CR2_STOP
#define  I2C_GENERATE_START_READ    (I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_GENERATE_START_WRITE   I2C_CR2_START

#define SlaveAddr_SHIFT             7
#define SlaveAddr_MSK               0x06

#define I2C_INT_TX                  (I2C_CR1_TCIE   | I2C_CR1_TXIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE)
#define I2C_INT_RX                  (I2C_CR1_TCIE   | I2C_CR1_RXIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE)
#define I2C_INT_LISTEN              (I2C_CR1_ADDRIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE)
#define I2C_INT_ALL                 (I2C_INT_TX | I2C_INT_RX | I2C_INT_LISTEN)

/*
 *  I2Cポート設定テーブル
 */
static const I2C_PortControlBlock i2c_pcb[NUM_I2CPORT] = {
  {	TADR_I2C1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),   RCC_AHB2ENR_GPIOBEN, RCC_AHB2ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR1),  RCC_APB1ENR1_I2C1EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR1), RCC_APB1RSTR1_I2C1RST,
	TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	8, GPIO_AF4_I2C1, 9, GPIO_AF4_I2C1 },

  {	TADR_I2C2_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),   RCC_AHB2ENR_GPIOBEN, RCC_AHB2ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR1),  RCC_APB1ENR1_I2C2EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR1), RCC_APB1RSTR1_I2C2RST,
	TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	10, GPIO_AF4_I2C2, 11, GPIO_AF4_I2C2 },

  {	TADR_I2C3_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB2ENR),   RCC_AHB2ENR_GPIOAEN, RCC_AHB2ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR1),  RCC_APB1ENR1_I2C3EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR1), RCC_APB1RSTR1_I2C3RST,
	TADR_GPIOC_BASE, TADR_GPIOC_BASE,
	0, GPIO_AF4_I2C3, 1, GPIO_AF4_I2C3 }
};


/*
 * I2C ハンドラ
 */
static I2C_Handle_t I2cHandle[NUM_I2CPORT];

/*
 *  I2C転送設定
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 DevAddress: デバイスアドレス
 *  parameter3 Size:       転送サイズ
 *  parameter4 Mode:       転送モード
 *  parameter5 Request:    スタート状態リクエスト
 *  return                 なし
 */
static void
i2c_transfconfig(I2C_Handle_t *hi2c, uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request)
{
	uint32_t tmp = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2));

	/*
	 *  設定モードクリア
	 */
	tmp &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);

	/*
	 *  転送設定を行う
	 */
	tmp |= (uint32_t)((DevAddress & I2C_CR2_SADD) | ((Size << 16) & I2C_CR2_NBYTES) | Mode | Request);
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), tmp);
}

/*
 *  I2C送信データレジスタフラッシュ
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  return              なし
 */
static void
i2c_flushtxdr(I2C_Handle_t *hi2c)
{
	/*
	 *  TXISフラグオンなら、TXDRにゼロをセット
	 */
	if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_TXIS) != 0)
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), 0);

	/*
	 *  TXEフラグがオフなら、EMPTYセット
	 */
	if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_TXE) == 0)
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR), I2C_ISR_TXE);
}

/*
 *  ISRレジスタセット待ち
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  parameter2 Flag:    待ちフラグ
 *  parameter3 Timeout: 待ち時間(ms)
 *  parameter4 pTimeout:次回待ち時間設定ポインタ
 *  return              正常終了時、E_OK
 */
static ER
i2c_isrflagsetwait(I2C_Handle_t *hi2c, uint32_t Flag, uint32_t Timeout, uint32_t *pTimeout)
{
	uint32_t tick = 0;
	uint32_t ctime = Timeout * 1000;

	*pTimeout = Timeout;
	while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & Flag) == 0){
		if(ctime == 0 || tick > ctime){
			hi2c->status= I2C_STATUS_READY;
			return E_TMOUT;
		}
        sil_dly_nse(1000);
		tick++;
	}
	*pTimeout -= tick / 1000;
	return E_OK;
}

/*
 *  I2C ACK受信確認
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  parameter2 Timeout: 待ち時間(ms)
 *  parameter3 pTimeout:次回待ち時間設定ポインタ
 *  return              正常終了時、E_OK
 */
static ER
i2c_checkacknowledgefailed(I2C_Handle_t *hi2c, uint32_t Timeout, uint32_t *pTimeout)
{
	uint32_t tick = 0;

	if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_NACKF) != 0){
		/*
		 *  NACK受信ならSTOPフラグ待ち
		 */
		while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_STOPF) == 0){
			if(tick > (Timeout * 1000)){
				hi2c->status= I2C_STATUS_READY;
				*pTimeout -= tick * 1000;
				return E_TMOUT;
			}
			tick++;
			sil_dly_nse(1000);
		}
		*pTimeout -= tick * 1000;

		/*
		 *  NACKフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);

		/*
		 *  STOPフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);

		/*
		 *  送信レジスタクリア
		 */
		i2c_flushtxdr(hi2c);

		/*
		 *  転送設定レジスタクリア
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), ((I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN)));

		hi2c->ErrorCode = I2C_ERROR_AF;
		hi2c->status= I2C_STATUS_READY;
		return E_SYS;
	}
	return E_OK;
}

/*
 *  I2C TXISフラグセット待ち
 *  parameter2 Timeout: 待ち時間(ms)
 *  parameter3 pTimeout:次回待ち時間設定ポインタ
 *  return              正常終了時、E_OK
 */
static ER
i2c_waitsettxisflag(I2C_Handle_t *hi2c, uint32_t Timeout, uint32_t *pTimeout)
{
	ER ercd = E_OK;
	uint32_t tick = 0;

	while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_TXIS) == 0){
	    /*
		 *  NACK受信確認
		 */
	    if((ercd = i2c_checkacknowledgefailed(hi2c, Timeout, pTimeout)) != E_OK)
			return ercd;
		if(tick > (Timeout * 1000)){
			hi2c->ErrorCode |= I2C_ERROR_TIMEOUT;
			hi2c->status = I2C_STATUS_READY;
			return E_TMOUT;
		}
		tick++;
		sil_dly_nse(1000);
	}
	*pTimeout -= tick / 1000;
	return E_OK;
}

/*
 *  BUS READY待ち
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  parameter2 Timeout: 待ち時間(ms)
 *  return              正常終了時、E_OK
 */
static ER
i2c_busreadywait(I2C_Handle_t *hi2c, uint32_t Timeout)
{
	uint32_t tick = 0;
	uint32_t ctime = Timeout * 1000;

    while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BUSY) != 0){
		if(ctime == 0 || tick > ctime){
			hi2c->status= I2C_STATUS_READY;
			return E_TMOUT;
		}
        sil_dly_nse(1000);
		tick++;
	}
	return E_OK;
}

/*
 *  I2C転送待ち
 *  parameter1 hi2c:   I2Cハンドラへのポインタ
 *  parameter2 Timeout:タイムアウト時間(ms)
 */
static ER
i2c_transwait(I2C_Handle_t *hi2c, uint32_t Timeout)
{
	uint32_t tick = 0;

	while(hi2c->status != I2C_STATUS_READY){
		if(tick >= Timeout)
			return E_TMOUT;
		if(hi2c->Init.semid != 0)
			twai_sem(hi2c->Init.semid, 1);
		else
			dly_tsk(1);
		tick++;
	}
	if(hi2c->ErrorCode == I2C_ERROR_NONE)
		return E_OK;
	else
		return E_SYS;
}

/*
 *  マスター受信 デバイスアドレスとメモリアドレス設定
 *  parameter1 hi2c:  I2Cハンドラへのポインタ
 *  parameter2 DevAddress: デバイスアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 Timeout:    タイムアウト時間(ms)
 *  return                 正常終了時、E_OK
 */
static ER
i2c_masterreadaddress(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout)
{
	ER ercd = E_OK;

	if(MemAddSize == 0)
		return E_OK;

	i2c_transfconfig(hi2c, DevAddress, MemAddSize, I2C_SOFTEND_MODE, I2C_GENERATE_START_WRITE);

	/*
	 *  TXISフラグ設定待ち
	 */
	if((ercd = i2c_waitsettxisflag(hi2c, Timeout, &Timeout)) != E_OK){
		if(hi2c->ErrorCode == I2C_ERROR_AF)
			return E_SYS;
		else
			return ercd;
	}

	/*
	 *  メモリアドレス設定
	 */
	if(MemAddSize == I2C_MEMADD_SIZE_8BIT){
		/*
		 *  8ビットメモリアドレス設定
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}
	else{
		/*
		 *  16ビットメモリアドレス設定
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), ((MemAddress >> 8) & 0xff));

		/*
		 *  TXISフラグ設定待ち
		 */
		if((ercd = i2c_waitsettxisflag(hi2c, Timeout, &Timeout)) != E_OK){
			if(hi2c->ErrorCode == I2C_ERROR_AF)
				return E_SYS;
			else
				return ercd;
		}
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}

	/*
	 *  TCフラグ設定待ち
	 */
	return i2c_isrflagsetwait(hi2c, I2C_ISR_TC, Timeout, &Timeout);
}

/*
 *  マスター送信 デバイスアドレスとメモリアドレス設定
 *  parameter1 hi2c:  I2Cハンドラへのポインタ
 *  parameter2 DevAddress: デバイスアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 Timeout:    タイムアウト時間(ms)
 *  return                 正常終了時、E_OK
 */
static ER
i2c_masterwriteaddress(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout)
{
	ER ercd = E_OK;

	if(MemAddSize == 0)
		return E_OK;

	i2c_transfconfig(hi2c,DevAddress,MemAddSize, I2C_RELOAD_MODE, I2C_GENERATE_START_WRITE);

	/*
	 *  TXISフラグ設定待ち
	 */
	if((ercd = i2c_waitsettxisflag(hi2c, Timeout, &Timeout)) != E_OK){
		if(hi2c->ErrorCode == I2C_ERROR_AF)
			return E_SYS;
		else
			return ercd;
	}

	/*
	 *  メモリアドレス設定
	 */
	if(MemAddSize == I2C_MEMADD_SIZE_8BIT){
		/*
		 *  8ビットメモリアドレス設定
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}
	else{
		/*
		 *  16ビットメモリアドレス設定
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), ((MemAddress >> 8) & 0xff));

		/*
		 *  TXISフラグ設定待ち
		 */
		if((ercd = i2c_waitsettxisflag(hi2c, Timeout, &Timeout)) != E_OK){
			if(hi2c->ErrorCode == I2C_ERROR_AF)
				return E_SYS;
			else
				return ercd;
		}

		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}

	/*
	 *  TCRフラグ設定待ち
	 */
	return i2c_isrflagsetwait(hi2c, I2C_ISR_TCR, Timeout, &Timeout);
}

/*
 *  I2Cデバイスの初期化
 *  parameter1 hi2c  I2Cハンドラへのポインタ
 *  return           正常終了時、E_OK
 */
I2C_Handle_t *
i2c_init(ID port, I2C_Init_t *ii2c)
{
	GPIO_Init_t GPIO_Init_Data;
	const I2C_PortControlBlock *ipcb;
	I2C_Handle_t *hi2c;
	uint32_t no;

	if(port < I2C1_PORTID || port > NUM_I2CPORT)
		return NULL;
	if(ii2c == NULL)
		return NULL;

	no = INDEX_I2C(port);
	ipcb = &i2c_pcb[no];
	hi2c = &I2cHandle[no];
	if(hi2c->status != I2C_STATUS_RESET)
		return NULL;
	memcpy(&hi2c->Init, ii2c, sizeof(I2C_Init_t));
	hi2c->base  = ipcb->base;
	hi2c->i2cid = port;

	/*
	 *  SCL/SDAピン/I2Cクロック設定
	 */
	sil_orw_mem((uint32_t *)ipcb->gioclockbase, ipcb->gioclockbit1);
	sil_orw_mem((uint32_t *)ipcb->gioclockbase, ipcb->gioclockbit2);
	sil_orw_mem((uint32_t *)ipcb->i2cclockbase, ipcb->i2cclockbit);

	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = GPIO_NOPULL;
	GPIO_Init_Data.otype     = GPIO_OTYPE_OD;
	GPIO_Init_Data.speed     = GPIO_SPEED_FAST;
	GPIO_Init_Data.alternate = ipcb->sclaf;
	gpio_setup(ipcb->giobase1, &GPIO_Init_Data, ipcb->sclpin);

	GPIO_Init_Data.alternate = ipcb->sdaaf;
	gpio_setup(ipcb->giobase2, &GPIO_Init_Data, ipcb->sdapin);


	hi2c->status = I2C_STATUS_BUSY;

	/*
	 *  I2C停止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	/*
	 *  I2Cタイミングレジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TIMINGR), (hi2c->Init.Timing &  TIMING_CLEAR_MASK));

	/*
	 *  I2Cデバイスアドレス１設定
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), I2C_OAR1_OA1EN);
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), (hi2c->Init.AddressingMode | hi2c->Init.OwnAddress1));

	/*
	 *  I2Cマスターモード設定
	 */
	if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_ADD10);
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_AUTOEND | I2C_CR2_NACK));

	/*
	 *  I2Cデバイスアドレス２設定
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR2), I2C_DUALADDRESS_ENABLE);
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR2), (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2 | hi2c->Init.OwnAddress2Masks));

	/*
	 *  I2C CR1設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode));

	/*
	 *  I2C起動
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	hi2c->ErrorCode = I2C_ERROR_NONE;
	hi2c->status = I2C_STATUS_READY;
	return hi2c;
}

/*
 *  I2Cデバイスの無効化
 *  parameter1 hi2c  I2Cハンドラへのポインタ
 *  return           正常終了時、E_OK
 */
ER
i2c_deinit(I2C_Handle_t *hi2c)
{
	const I2C_PortControlBlock *ipcb;

	if(hi2c == NULL){
		return E_PAR;
	}

	hi2c->status = I2C_STATUS_BUSY;
	ipcb = &i2c_pcb[INDEX_I2C(hi2c->i2cid)];

	/*
	 *  I2C停止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	/*
	 *  I2Cクロックリセット
	 */
	sil_orw_mem((uint32_t *)ipcb->i2cresetbase, ipcb->i2cresetbit);
	sil_andw_mem((uint32_t *)ipcb->i2cresetbase, ipcb->i2cresetbit);

	/*
	 *  I2Cクロック停止
	 */
	sil_andw_mem((uint32_t *)ipcb->i2cclockbase, ipcb->i2cclockbit);

	hi2c->ErrorCode = I2C_ERROR_NONE;
	hi2c->status = I2C_STATUS_RESET;
	return E_OK;
}

/*
 *  I2Cスレーブ受信
 *  parameter1 hi2c:   I2Cハンドラへのポインタ
 *  parameter2 pData:  読み出しバッファへのポインタ
 *  parameter3 Size:   読み出しサイズ
 *  return             正常終了ならE_OK
 */
ER
i2c_slaveread(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size)
{
	ER ercd = E_OK;

	if(pData == NULL || Size == 0)
		return E_PAR;

	/*
	 *  読み出しロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->status != I2C_STATUS_READY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

    /*
	 *  アドレスアクノレッジを有効化
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);

    hi2c->status = I2C_STATUS_BUSY_RX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;
    hi2c->XferCount2 = Size;
	hi2c->XferOptions = I2C_NO_OPTION_FRAME;

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_INT_RX | I2C_INT_LISTEN));
	ercd = i2c_transwait(hi2c, 5000);

	/*
	 *  読み出しロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return ercd;
}

/*
 *  I2Cスレーブ送信
 *  parameter1 hi2c:   I2Cハンドラへのポインタ
 *  parameter2 pData:  読み出しバッファへのポインタ
 *  parameter3 Size:   読み出しサイズ
 *  return             正常終了ならE_OK
 */
ER
i2c_slavewrite(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size)
{
	ER ercd = E_OK;

	if(pData == NULL || Size == 0)
		return E_PAR;

	/*
	 *  書き込みロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->status != I2C_STATUS_READY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

    /*
	 *  アドレスアクノレッジを有効化
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);

    hi2c->status = I2C_STATUS_BUSY_TX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;
    hi2c->XferCount2 = Size;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_INT_TX | I2C_INT_LISTEN));
	ercd = i2c_transwait(hi2c, 5000);

	/*
	 *  書き込みロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return ercd;
}

/*
 *  I2Cデータリード
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 DevAddress: スレーブアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 pData:      読み出しバッファへのポインタ
 *  parameter6 Size:       読み出しサイズ
 *  return                 正常終了ならE_OK
 */
ER
i2c_memread(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
	ER ercd = E_OK;
	uint32_t xfermode;

	if(pData == NULL || Size == 0)
		return E_PAR;
	if(hi2c->Init.AddressingMode != I2C_ADDRESSINGMODE_7BIT && MemAddSize != 0)
		return E_PAR;

	/*
	 *  読み出しロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->status != I2C_STATUS_READY || i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

    hi2c->status = I2C_STATUS_BUSY_RX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;
    if(hi2c->XferCount > MAX_NBYTE_SIZE){
		hi2c->XferCount2 = MAX_NBYTE_SIZE;
		xfermode = I2C_RELOAD_MODE;
	}
	else{
		hi2c->XferCount2 = hi2c->XferCount;
		xfermode = I2C_AUTOEND_MODE;
	}

	/* スレーブアドレスと読み出しアドレスの設定 */
	if((ercd = i2c_masterreadaddress(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG)) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
        return ercd;
	}

    /* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE and generate RESTART */
	i2c_transfconfig(hi2c,DevAddress, hi2c->XferCount2, xfermode, I2C_GENERATE_START_READ);

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_RX);
	ercd = i2c_transwait(hi2c, 100);

	/*
	 *  読み出しロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return ercd;
}

/*
 *  I2Cデータライト
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 DevAddress: スレーブアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 pData:      書込みバッファへのポインタ
 *  parameter6 Size:       書込みサイズ
 *  return                 正常終了ならE_OK
 */
ER
i2c_memwrite(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
	ER ercd = E_OK;
	uint32_t xfermode;

	if(pData == NULL || Size == 0)
		return  E_PAR;
	if(hi2c->Init.AddressingMode != I2C_ADDRESSINGMODE_7BIT && MemAddSize != 0)
		return  E_PAR;

	/*
	 *  書き込みロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->status != I2C_STATUS_READY || i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

	hi2c->status = I2C_STATUS_BUSY_TX;
	hi2c->ErrorCode = I2C_ERROR_NONE;

	hi2c->pBuffPtr = pData;
	hi2c->XferSize = Size;
	hi2c->XferCount = Size;
    if(hi2c->XferCount > MAX_NBYTE_SIZE){
		hi2c->XferCount2 = MAX_NBYTE_SIZE;
		xfermode = I2C_RELOAD_MODE;
	}
	else{
		hi2c->XferCount2 = hi2c->XferCount;
		xfermode = I2C_AUTOEND_MODE;
	}

	/* スレーブアドレスと書込みアドレスの設定 */
	if((ercd = i2c_masterwriteaddress(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG)) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return ercd;
	}

    /* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE and generate RESTART */
	if(MemAddSize == 0)
	    i2c_transfconfig(hi2c, DevAddress, hi2c->XferSize, xfermode, I2C_GENERATE_START_WRITE);
	else
	    i2c_transfconfig(hi2c, DevAddress, hi2c->XferCount2, xfermode, I2C_NO_STARTSTOP);

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_TX);
	ercd = i2c_transwait(hi2c, 500);

	/*
	 *  書き込みロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return ercd;
}

/*
 *  I2Cレディチェック
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 DevAddress: スレーブアドレス
 *  parameter3 Trials:     トライアス値
 *  parameter4 Timeout:    タイムアウト値
 *  return                 レディならE_OK
 */
ER
i2c_ready(I2C_Handle_t *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
	volatile uint32_t I2C_Trials = 0;
	uint32_t tmp1 = 0;

	if(hi2c->status == I2C_STATUS_READY){
		if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BUSY) != 0)
			return E_OBJ;

		/*
		 *  アクセスロック
		 */
		if(hi2c->Init.semlock != 0)
			wai_sem(hi2c->Init.semlock);

		hi2c->status    = I2C_STATUS_BUSY;
		hi2c->ErrorCode = I2C_ERROR_NONE;

		do{
			/* START生成 */
			if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
				tmp1 = ((DevAddress & I2C_CR2_SADD) | I2C_CR2_START | I2C_CR2_AUTOEND) & (~I2C_CR2_RD_WRN);
			else
				tmp1 = ((DevAddress & I2C_CR2_SADD) | I2C_CR2_ADD10 | I2C_CR2_START) & (~I2C_CR2_RD_WRN);
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), tmp1);

			/* START STOP/NACK待ち */
			if(i2c_isrflagsetwait(hi2c, (I2C_ISR_STOPF | I2C_ISR_NACKF), Timeout, &Timeout) != E_OK){
				/*
				 *  ロック解除
				 */
				if(hi2c->Init.semlock != 0)
					sig_sem(hi2c->Init.semlock);
				return E_TMOUT;
			}


			/* NACKフラグチェック */
			if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_NACKF) == 0){
				/*
				 *  STOPフラグが立つまで待つ
				 */
				if(i2c_isrflagsetwait(hi2c, I2C_ISR_STOPF, Timeout, &Timeout) != E_OK)
					return E_TMOUT;

				/*
				 *  STOPフラグクリア
				 */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
				hi2c->status = I2C_STATUS_READY;
				/*
				 *  ロック解除
				 */
				if(hi2c->Init.semlock != 0)
					sig_sem(hi2c->Init.semlock);
				return E_OK;
			}
			else{	/* アクノレッジ非受信状態 */
				/*
				 *  STOPフラグが立つまで待つ
				 */
				if(i2c_isrflagsetwait(hi2c, I2C_ISR_STOPF, Timeout, &Timeout) != E_OK)
					return E_TMOUT;

				/*
				 *  NACKフラグクリア
				 */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);

				/*
				 *  STOPフラグクリア
				 */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
			}
			if(I2C_Trials++ == Trials){
				/*
				 *  停止要求
				 */
				sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_STOP);

				/*
				 *  STOPフラグが立つまで待つ
				 */
				if(i2c_isrflagsetwait(hi2c, I2C_ISR_STOPF, Timeout, &Timeout) != E_OK)
					return E_TMOUT;

				/*
				 *  STOPフラグクリア
				 */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
			}
		}while(I2C_Trials < Trials);

		hi2c->status = I2C_STATUS_READY;
		/*
		 *  ロック解除
		 */
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
	    return E_TMOUT;
	}
	else
		return E_OBJ;
}

/*
 *  I2Cマスターシーケンシャルによる終了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  return                 なし
 */
static void
i2c_intmastersequentialcomplete(I2C_Handle_t *hi2c)
{
	/* No Generate Stop, to permit restart mode */
	/* The stop will be done at the end of transfer, when I2C_AUTOEND_MODE enable */
	if(hi2c->status == I2C_STATUS_BUSY_TX){
		/*
		 *  送信中ならば終了設定
		 */
		hi2c->status = I2C_STATUS_READY;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_TX);
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
	else{
		/*
		 *  受信中ならば終了設定
		 */
		hi2c->status = I2C_STATUS_READY;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_RX);
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
	if(hi2c->Init.semid != 0)
		isig_sem(hi2c->Init.semid);
}

/*
 *  I2CマスターSTOPフラグによる終了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 isr:        ISRレジスタ値
 *  return                 なし
 */
static void
i2c_intmastercomplete(I2C_Handle_t *hi2c, uint32_t isr)
{
	/*
	 *  STOPフラグクリア
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);

	/*
	 *  転送レジスタクリア
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), ((I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN)));

	/*
	 *  オプション設定クリア
	 */
	hi2c->XferOptions   = I2C_NO_OPTION_FRAME;

	/*
	 *  NACK受信ならエラー設定
	 */

	if((isr & I2C_ISR_NACKF) != 0){
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		hi2c->ErrorCode |= I2C_ERROR_AF;
	}

	/*
	 *  送信レジスタフラッシュ
	 */
	i2c_flushtxdr(hi2c);

	/*
	 *  割込みマスク
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_INT_TX | I2C_INT_RX));

	/*
	 *  エラー発生ならばサイズエラーを付加
	 */
	if(hi2c->ErrorCode != I2C_ERROR_NONE){
		hi2c->ErrorCode |= I2C_ERROR_SIZE;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);
	}
	else if(hi2c->status == I2C_STATUS_BUSY_TX){
		/*
		 *  送信中ならば終了設定
		 */
		hi2c->status = I2C_STATUS_READY;
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
		if(hi2c->Init.semid != 0)
			isig_sem(hi2c->Init.semid);
	}
	else if(hi2c->status == I2C_STATUS_BUSY_RX){
		/*
		 *  受信中ならば終了設定
		 */
		hi2c->status = I2C_STATUS_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
		if(hi2c->Init.semid != 0)
			isig_sem(hi2c->Init.semid);
	}
}

/*
 *  I2CスレーブLISTEN終了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 isr:        ISRレジスタ値
 *  return                 なし
 */
static void
i2c_intlistencomplete(I2C_Handle_t *hi2c, uint32_t isr)
{
	/*
	 *  I2C設定をクリア
	 */
	hi2c->XferOptions = I2C_NO_OPTION_FRAME;
	hi2c->status = I2C_STATUS_READY;

	/* Store Last receive data if any */
	if((isr & I2C_ISR_RXNE) != 0){
	    /*
		 *  受信データがあれば、取り込む
		 */
	    (*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));

		/*
		 *  受信が完結していなければ、Non-Acknowledgeエラーとする
		 */

	    if(hi2c->XferCount2 > 0){
			hi2c->XferCount2--;
			hi2c->XferCount--;
			hi2c->ErrorCode |= I2C_ERROR_AF;
		}
	}

	/*
	 *  全割込み禁止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);

	/*
	 *  NACKフラグクリア
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);

	/*
	 *  LISTENコールバック処理
	 */
	if(hi2c->listencallback != NULL)
		hi2c->listencallback(hi2c);
}

/*
 *  I2Cスレーブアドレス完了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 isr:        ISRレジスタ値
 *  return                 なし
 */
static void
i2c_intaddrcomplete(I2C_Handle_t *hi2c, uint32_t isr)
{
	uint8_t  transferdirection = 0;
	uint16_t slaveaddrcode = 0;
	uint16_t ownadd1code = 0;
	uint16_t ownadd2code = 0;

	/*
	 *  LISTEN中ならば、アドレス情報を取り出す
	 */
	if((hi2c->status & I2C_STATUS_LISTEN) == I2C_STATUS_LISTEN){
		transferdirection = (sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_DIR) >> 16;
		slaveaddrcode     = (sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_ADDCODE) >> 16;
		ownadd1code       = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1)) & I2C_OAR1_OA1;
		ownadd2code       = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR2)) & I2C_OAR2_OA2;

		if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT){
			/*
			 *  10ビットアドレスモード
			 */
			if((slaveaddrcode & SlaveAddr_MSK) == ((ownadd1code >> SlaveAddr_SHIFT) & SlaveAddr_MSK)){
				slaveaddrcode = ownadd1code;
				hi2c->AddrEventCount++;
				if(hi2c->AddrEventCount == 2){
					/*
					 *  イベントカウンタークリア
					 */
					hi2c->AddrEventCount = 0;

					/*
					 *  ADDRフラグクリア
					 */
					sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ADDRCF);

				}
				/*
				 *  アドレスコールバック
				 */
				if(hi2c->addrcallback != NULL)
					hi2c->addrcallback(hi2c, transferdirection, slaveaddrcode);
			}
			else{
				slaveaddrcode = ownadd2code;

				/*
				 *  LISTEN割込み禁止
				 */
				sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_LISTEN);

				/*
				 *  アドレスコールバック
				 */
				if(hi2c->addrcallback != NULL)
					hi2c->addrcallback(hi2c, transferdirection, slaveaddrcode);
			}
		}
		else{
			/*
			 *  7ビットアドレスモード、LISTEN割込み禁止
			 */
			sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_LISTEN);

			/*
			 *  アドレスコールバック
			 */
			if(hi2c->addrcallback != NULL)
				hi2c->addrcallback(hi2c, transferdirection, slaveaddrcode);
		}
	}
	else{
		/*
		 *  ADDRフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ADDRCF);
	}
}

/*
 *  I2Cスレーブシーケンシャルによる終了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  return                 なし
  */
static void
i2c_intslavesequencecomplete(I2C_Handle_t *hi2c)
{
	if(hi2c->status == I2C_STATUS_BUSY_TX_LISTEN){
		/*
		 *  送信中なら終了処理
		 */
		hi2c->status = I2C_STATUS_LISTEN;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_TX);
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
	else if(hi2c->status == I2C_STATUS_BUSY_RX_LISTEN){
		/*
		 *  受信中なら終了処理
		 */
	    hi2c->status = I2C_STATUS_LISTEN;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_RX);
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
}

/*
 *  I2CスレーブSTOPフラグによる終了処理
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 *  parameter2 isr:        ISRレジスタ値
 *  return                 なし
 */
static void
i2c_intslavecomplete(I2C_Handle_t *hi2c, uint32_t isr)
{
	/*
	 *  STOPフラグをクリア
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);

	/*
	 *  ADDRフラグをクリア
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ADDRCF);

	/*
	 *  全割込み停止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);

	/*
	 *  アドレスアクノレッジ有効化
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);

	/*
	 *  転送設定レジスタリセット
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));

	/*
	 *  送信レジスタフラッシュ
	 */
	i2c_flushtxdr(hi2c);

	if(hi2c->XferCount != 0){
		/*
		 *  転送終了でなければ、Non-Acknowledgeエラー
		 */
		hi2c->ErrorCode |= I2C_ERROR_AF;
	}

	/*
	 *  受信要求があれば取り込む
	 */
	if((isr & I2C_ISR_RXNE) != 0){
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));
		if((hi2c->XferCount > 0U)){
			hi2c->XferCount2--;
			hi2c->XferCount--;
		}
	}

	if(hi2c->ErrorCode != I2C_ERROR_NONE){
	    /*
		 *  エラーが発生していればサイズエラーを加える
		 */
		hi2c->ErrorCode |= I2C_ERROR_SIZE;
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);

	    if(hi2c->status == I2C_STATUS_LISTEN){
			/*
			 *  LISTEN中ならば、LISTEN終了処理を行う
			 */
			i2c_intlistencomplete(hi2c, isr);
		}
	}
	else if(hi2c->XferOptions != I2C_NO_OPTION_FRAME){
		/*
		 *  オプション設定がある場合、LISTEN終了処理
		 */
	    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
	    hi2c->status = I2C_STATUS_READY;
		if(hi2c->listencallback != NULL)
			hi2c->listencallback(hi2c);
	}
	else if(hi2c->status == I2C_STATUS_BUSY_RX){
		/*
		 *  受信中なら終了処理
		 */
		hi2c->status = I2C_STATUS_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
	else{
		/*
		 *  送信中なら終了処理
		 */
		hi2c->status = I2C_STATUS_READY;
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
}


/*
 *  I2C EV割込みハンドラ
 */
void
i2c_ev_handler(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1));
	uint16_t devaddress = 0;
	volatile uint32_t tmp;

	/*
	 *  マスターモード
	 */
	if(hi2c->Init.OwnAddress1 == 0){
		if((isr & I2C_ISR_NACKF) != 0 && (cr1 & I2C_CR1_NACKIE) != 0){
			/*
			 *  NACKエラーフラグクリア
			 */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		    hi2c->ErrorCode |= I2C_ERROR_AF;

		    /*
			 *  送信レジスタフラッシュ
			 */
		    i2c_flushtxdr(hi2c);
		}
		else if((isr & I2C_ISR_RXNE) != 0 && (cr1 & I2C_CR1_RXIE) != 0){
			/*
			 *  受信データ取り込み
			 */
			*hi2c->pBuffPtr++ = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));
			hi2c->XferCount--;
			hi2c->XferCount2--;
		}
		else if((isr & I2C_ISR_TXIS) != 0 && (cr1 & I2C_CR1_TXIE) != 0){
			/*
			 *  送信データ設定
			 */
			tmp = (uint8_t)(*hi2c->pBuffPtr++);
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), tmp);
			hi2c->XferCount--;
			hi2c->XferCount2--;
		}
		else if((isr & I2C_ISR_TCR) != 0 && (cr1 & I2C_CR1_TCIE) != 0){
			if(hi2c->XferCount2 == 0 && hi2c->XferCount != 0){
				devaddress = (sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2)) & I2C_CR2_SADD);
				if(hi2c->XferCount > MAX_NBYTE_SIZE){
					hi2c->XferCount2 = MAX_NBYTE_SIZE;
					i2c_transfconfig(hi2c, devaddress, hi2c->XferCount2, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
				}
				else{
					hi2c->XferCount2 = hi2c->XferCount;
					if(hi2c->XferOptions != I2C_NO_OPTION_FRAME)
						i2c_transfconfig(hi2c, devaddress, hi2c->XferCount2, hi2c->XferOptions, I2C_NO_STARTSTOP);
					else
						i2c_transfconfig(hi2c, devaddress, hi2c->XferCount2, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
				}
			}
			else{
				/*
				 *  AUTOENDモードなら正常終了
				 */
				if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2)) & I2C_CR2_AUTOEND) != 0)
					i2c_intmastersequentialcomplete(hi2c);
				else{
					/*
					 *  異常終了ケース
					 */
					hi2c->ErrorCode |= I2C_ERROR_SIZE;
					sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);
				}
			}
		}
		else if((isr & I2C_ISR_TC) != 0 && (cr1 & I2C_CR1_TCIE) != 0){
			if(hi2c->XferCount == 0){
				if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2)) & I2C_CR2_AUTOEND) != I2C_CR2_AUTOEND){
					if(hi2c->XferOptions == I2C_NO_OPTION_FRAME){
						/*
						 *  オプションがなければSTOP要求
						 */
						sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_STOP);
					}
					else
						i2c_intmastersequentialcomplete(hi2c);
				}
			}
			else{
				/*
				 *  TCフラグエラー
				 */
				hi2c->ErrorCode |= I2C_ERROR_SIZE;
				sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_INT_ALL);
			}
		}
		if((isr & I2C_ISR_STOPF) != 0 && (cr1 & I2C_CR1_STOPIE) != 0){
			/*
			 *  停止フラグから終了処理
			 */
			i2c_intmastercomplete(hi2c, isr);
		}
	}

	/*
	 *  スレーブモード
	 */
	else{
		if((isr & I2C_ISR_NACKF) != 0 && (cr1 & I2C_CR1_NACKIE) != 0){
			/*
			 *  NACK発生
			 */
			if(hi2c->XferCount == 0){
				/*
				 *  転送終了時
				 */
				if((hi2c->XferOptions == I2C_FIRST_AND_LAST_FRAME || hi2c->XferOptions == I2C_LAST_FRAME) && \
			        (hi2c->status == I2C_STATUS_LISTEN)){
					/* Call I2C Listen complete process */
					i2c_intlistencomplete(hi2c, isr);
				}
				else if(hi2c->XferOptions != I2C_NO_OPTION_FRAME && hi2c->status == I2C_STATUS_BUSY_TX_LISTEN){
					/*
					 *  NACKフラグクリア
					 */
					sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);

					/* Flush TX register */
					i2c_flushtxdr(hi2c);

					/* Last Byte is Transmitted */
					/* Call I2C Slave Sequential complete process */
					i2c_intslavesequencecomplete(hi2c);
				}
				else{
					/*
					 *  NACKフラグクリア
					 */
					sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
				}
			}
			else{
				/*
				 *  転送が終了していない場合、Non-Acknowledgeエラー
				 */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
				hi2c->ErrorCode |= I2C_ERROR_AF;
			}
		}
		else if((isr & I2C_ISR_RXNE) != 0 && (cr1 & I2C_CR1_RXIE) != 0){
			/*
			 *  受信割込み
			 */
			if(hi2c->XferCount > 0){
				(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));
				hi2c->XferCount2--;
				hi2c->XferCount--;
			}
			if(hi2c->XferCount == 0 && hi2c->XferOptions != I2C_NO_OPTION_FRAME){
				/*
				 *  I2Cスレーブシーケンス終了
				 */
				i2c_intslavesequencecomplete(hi2c);
			}
		}
		else if((isr & I2C_ISR_ADDR) != 0 && (cr1 & I2C_CR1_ADDRIE) != 0){
			/*
			 *  ADDRフラグ
			 */
			i2c_intaddrcomplete(hi2c, isr);
		}
		else if((isr & I2C_ISR_TXIS) != 0 && (cr1 & I2C_CR1_TXIE) != 0){
			/*
			 *  送信割込み
			 */
		    if(hi2c->XferCount > 0){
				/* Write data to TXDR */
				tmp = (*hi2c->pBuffPtr++);
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), tmp);
				hi2c->XferCount--;
				hi2c->XferCount2--;
			}
			else{
				if(hi2c->XferOptions == I2C_NEXT_FRAME || hi2c->XferOptions == I2C_FIRST_FRAME){
					/*
					 *  I2Cスレーブ送信シーケンス終了
					 */
 					i2c_intslavesequencecomplete(hi2c);
				}
			}
		}
		if((isr & I2C_ISR_STOPF) != 0 && (cr1 & I2C_CR1_STOPIE) != 0){
			/*
			 *  停止フラグから終了処理
			 */
			i2c_intslavecomplete(hi2c, isr);
		}
	}
}

/*
 *  I2C ER割込みハンドラ
 */
void
i2c_er_handler(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1));

	/*
	 *  I2C バスエラー
	 */
	if(((isr & I2C_ISR_BERR) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_BERR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_BERRCF);
	}
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 *  I2C オーバーラン・エラー
	 */
	if(((isr & I2C_ISR_OVR) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_OVR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_OVRCF);
	}
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 *  I2C アービテーション・ロス・エラー
	 */
	if(((isr & I2C_ISR_ARLO) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_ARLO;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ARLOCF);
	}
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	if(hi2c->ErrorCode != I2C_ERROR_NONE){
		hi2c->status = I2C_STATUS_READY;

		/*
		 *  通信禁止
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_TCIE |
			 I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_ERRIE | I2C_CR1_STOPIE | I2C_CR1_ADDRIE));

		if(hi2c->errorcallback != NULL)
			hi2c->errorcallback(hi2c);
	}
}


/*
 *  I2C EV割込みサービスルーチン
 */
void
i2c_ev_isr(intptr_t exinf)
{
	i2c_ev_handler(&I2cHandle[INDEX_I2C((uint32_t)exinf)]);
}

/*
 *  I2C ER割込みサービスルーチン
 */
void
i2c_er_isr(intptr_t exinf)
{
	i2c_er_handler(&I2cHandle[INDEX_I2C((uint32_t)exinf)]);
}

