/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: i2c.c 698 2016-02-01 15:15:16Z roi $
 */
/*
 * 
 *  I2Cドライバ関数群
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <sil.h>
#include <target_syssvc.h>
#include "kernel_cfg.h"
#include "device.h"
#include "i2c.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  I2CポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_I2C(i2cid)        ((uint_t)((i2cid) - 1))

#define GPIO_AF4_I2C1           0x04	/* I2C1 Alternate Function mapping */
#define GPIO_AF4_I2C2           0x04	/* I2C2 Alternate Function mapping */
#define GPIO_AF4_I2C3           0x04	/* I2C3 Alternate Function mapping */
#define GPIO_AF4_I2C4           0x04	/* I2C4 Alternate Function mapping */
#define GPIO_AF11_I2C4          0x0B	/* I2C4 Alternate Function mapping */

/*
 *  I2Cポート設定テーブル
 */
static const I2C_PortControlBlock i2c_pcb[NUM_I2CPORT] = {
  {	TADR_I2C1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIOBEN, RCC_AHB1ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C1EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C1RST,
	TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	8, GPIO_AF4_I2C1, 9, GPIO_AF4_I2C1 },

  {	TADR_I2C2_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIOHEN, RCC_AHB1ENR_GPIOHEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C2EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C2RST,
	TADR_GPIOH_BASE, TADR_GPIOH_BASE,
	1, GPIO_AF4_I2C2, 0, GPIO_AF4_I2C2 },

  {	TADR_I2C3_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIOHEN, RCC_AHB1ENR_GPIOHEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C3EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C3RST,
	TADR_GPIOH_BASE, TADR_GPIOH_BASE,
	7, GPIO_AF4_I2C3, 8, GPIO_AF4_I2C3 },

#if defined(TOPPERS_STM32F769_DISCOVERY)
  {	TADR_I2C4_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIODEN, RCC_AHB1ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C4EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C4RST,
	TADR_GPIOD_BASE, TADR_GPIOB_BASE,
	12, GPIO_AF4_I2C4, 7, GPIO_AF11_I2C4 }
#else
  {	TADR_I2C4_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIODEN, RCC_AHB1ENR_GPIODEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C4EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C4RST,
	TADR_GPIOD_BASE, TADR_GPIOB_BASE,
	12, GPIO_AF4_I2C4, 13, GPIO_AF4_I2C4 }
#endif
};

static I2C_Handle_t I2cHandle[NUM_I2CPORT];

/*
 *  I2C スタートストップモード定義
 */
#define I2C_NO_STARTSTOP    0x00000000

/*
 *  I2C リロードエンドモード定義
 */
#define I2C_SOFTEND_MODE    0x00000000

#define TIMING_CLEAR_MASK   0xF0FFFFFF		/* I2C TIMING clear register Mask */

#define I2C_CR1_TRANS1      (I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_TXIE | I2C_CR1_ADDRIE)
#define I2C_CR1_TRANS2      (I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_TXIE)

#define I2C_CR1_RECV1       (I2C_CR1_TCIE| I2C_CR1_STOPIE| I2C_CR1_NACKIE | I2C_CR1_RXIE | I2C_CR1_ADDRIE)
#define I2C_CR1_RECV2       (I2C_CR1_TCIE| I2C_CR1_STOPIE| I2C_CR1_NACKIE | I2C_CR1_RXIE)
#define I2C_CR1_ERROR       (I2C_CR1_ERRIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE)
#define I2C_CR2_RESET       (I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN)

/*
 *  I2Cデバイスの初期化
 *  parameter1 hi2c  I2Cハンドラへのポインタ
 *  return           正常終了時、E_OK
 */
I2C_Handle_t *
i2c_init(ID port, I2C_Init_t *pini)
{
	const I2C_PortControlBlock *ipcb;
	GPIO_Init_t  gpio_init;
	I2C_Handle_t *hi2c;
	volatile uint32_t tmp;
	int no;

	if(port < I2C1_PORTID || port > NUM_I2CPORT)
		return NULL;
	if(pini == NULL)
		return NULL;

	no = INDEX_I2C(port);
	ipcb = &i2c_pcb[no];

	hi2c = &I2cHandle[no];
	if(hi2c->State != I2C_STATE_RESET)
		return NULL;
	memcpy(&hi2c->Init, pini, sizeof(I2C_Init_t));
	hi2c->base  = ipcb->base;
	hi2c->State = I2C_STATE_BUSY;
	hi2c->i2cid = port;

	/*
	 *  I2C-GPIOクロック設定
	 */
	sil_orw_mem((uint32_t *)ipcb->gioclockbase, ipcb->gioclockbit1);
	tmp = sil_rew_mem((uint32_t *)ipcb->gioclockbase);
	sil_orw_mem((uint32_t *)ipcb->gioclockbase, ipcb->gioclockbit2);
	tmp = sil_rew_mem((uint32_t *)ipcb->gioclockbase);

	/*
	 *  SCLピン設定
	 */
	gpio_init.mode  = GPIO_MODE_AF;
	gpio_init.pull  = GPIO_NOPULL;
	gpio_init.otype = GPIO_OTYPE_OD;
	gpio_init.speed = GPIO_SPEED_FAST;
	gpio_init.alternate = ipcb->sclaf;
	gpio_setup(ipcb->giobase1, &gpio_init, ipcb->sclpin);

	/*
	 *  SDAピン設定
	 */
	gpio_init.alternate = ipcb->sdaaf;
	gpio_setup(ipcb->giobase2, &gpio_init, ipcb->sdapin);

	sil_orw_mem((uint32_t *)ipcb->i2cclockbase, ipcb->i2cclockbit);
	tmp = sil_rew_mem((uint32_t *)ipcb->i2cclockbase);
	(void)(tmp);
	sil_orw_mem((uint32_t *)ipcb->i2cresetbase, ipcb->i2cresetbit);
	sil_andw_mem((uint32_t *)ipcb->i2cresetbase, ipcb->i2cresetbit);

	/* I2C Disable */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	/*
	 *  I2C TIMINGR設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TIMINGR), (hi2c->Init.Timing & TIMING_CLEAR_MASK));

	/*
	 *  I2Cアドレス1設定
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), I2C_OAR1_OA1EN);
	if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), (I2C_OAR1_OA1EN | hi2c->Init.OwnAddress1));
	else /* I2C_ADDRESSINGMODE_10BIT */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE | hi2c->Init.OwnAddress1));

	/*
	 *  I2C CR2レジスタ設定
	 */
	if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ADD10));
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_AUTOEND | I2C_CR2_NACK));

	/*
	 *  I2Cアドレス2設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR2), (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2 | (hi2c->Init.OwnAddress2Masks << 8)));

	/*
	 *  I2C CR1レジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode));
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	hi2c->ErrorCode = I2C_ERROR_NONE;
	hi2c->State = I2C_STATE_READY;
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
	/* ハンドラのチェック */
	if(hi2c == NULL)
		return E_PAR;

	hi2c->State = I2C_STATE_BUSY;

	/*
	 *  I2Cクロック停止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);

	hi2c->ErrorCode = I2C_ERROR_NONE;
	hi2c->State = I2C_STATE_RESET;
	return E_OK;
}

/*
 *  I2C通信のスタート設定
 *  parameter1 hi2c       I2Cハンドラへのポインタ
 *  parameter2 DevAddress スレーブアドレス
 *  parameter3 Size       メモリアドレスのサイズ
 *  parameter4 Mode       設定モード
 *  return     なし
 */
static void
i2c_transfarconfig(I2C_Handle_t *hi2c, uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request)
{
	uint32_t tmpreg = 0;

	/*
	 * I2C CR2レジスタ再設定
	 */
	tmpreg = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2));
	tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
	tmpreg |= (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << 16 ) & I2C_CR2_NBYTES) |
            (uint32_t)Mode | (uint32_t)Request);
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), tmpreg);
}

/*
 *  I2Cフラグを指定して設定待ち関数
 *  parameter1 hi2c  I2Cハンドラへのポインタ
 *  parameter2 Flag  待ちフラグ
 *  parameter3 Timeout タイムアウト時間(ms)
 *  return           正常終了 E_OK
 */
static ER
i2c_waitsetflag(I2C_Handle_t *hi2c, uint32_t Flag, uint32_t Timeout)
{
	uint32_t tick = 0;

	/*
	 *  フラグ設定待ち
	 */
	while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & Flag) != Flag){
		if(tick > Timeout){
			hi2c->State= I2C_STATE_READY;
			return E_TMOUT;
		}
		dly_tsk(1);
		tick++;
	}
	return E_OK;
}

/*
 *  I2C スレーブアドレスとメモリアドレス設定
 *  parameter1 hi2c       I2Cハンドラへのポインタ
 *  parameter2 DevAddress:スレーブアドレス
 *  parameter3 MemAddress:メモリアドレス
 *  parameter4 MemAddSize:メモリアドレスのサイズ
 *  parameter5 Mode      :設定モード
 *  parameter6 Timeout:タイムアウト時間(ms)
 *  return     正常設定 E_OK
 */
static ER
i2c_requestmemaddr(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Mode, uint32_t Timeout)
{
	ER err = E_OK;

	i2c_transfarconfig(hi2c,DevAddress,MemAddSize, Mode, I2C_CR2_START);
	/*
	 * TXISフラグ設定待ち
	 */
	if((err = i2c_waitsetflag(hi2c, I2C_ISR_TXIS, Timeout)) != E_OK)
		return err;

	/*
	 * メモリアドレス設定
	 */
	if(MemAddSize == I2C_MEMADD_SIZE_8BIT){	/* 8bit設定 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}
	else{									/* 16bit設定 */
		/* メモリアドレスMSB設定 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), ((MemAddress>>8) & 0xff));
		if((err = i2c_waitsetflag(hi2c, I2C_ISR_TXIS, Timeout)) != E_OK)
			return err;
		/* メモリアドレスLSB設定 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (MemAddress & 0xff));
	}
	/*
	 *  TC(R)フラグ設定待ち
	 */
	if(Mode == I2C_SOFTEND_MODE)
		err = i2c_waitsetflag(hi2c, I2C_ISR_TC, Timeout);
	else
		err = i2c_waitsetflag(hi2c, I2C_ISR_TCR, Timeout);
	return err;
}

/*
 *  I2C転送待ち
 *  parameter1 hi2c    I2Cハンドラへのポインタ
 *  parameter2 Timeout:タイムアウト時間(ms)
 */
static ER
i2c_transwait(I2C_Handle_t *hi2c, uint32_t Timeout)
{
	uint32_t tick = 0;

	while(hi2c->State != I2C_STATE_READY){
		if(tick >= Timeout)
			return E_TMOUT;
		if(hi2c->Init.semid != 0)
			twai_sem(hi2c->Init.semid, 1);
		else
			dly_tsk(1);
		tick++;
	}
	return E_OK;
}

/*
 *  I2Cスレーブ受信
 *  parameter1 hi2c   I2Cハンドラへのポインタ
 *  parameter2 pData: 読み出しバッファへのポインタ
 *  parameter3 Size:  読み出しサイズ
 *  return            正常終了ならE_OK
 */
ER
i2c_slaveread(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size)
{
	ER err = E_OK;

	if(pData == NULL || Size == 0)
		return E_PAR;

	/*
	 *  読み出しロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->State != I2C_STATE_READY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

    hi2c->State = I2C_STATE_SLAVE_BUSY_RX;
    hi2c->ErrorCode   = I2C_ERROR_NONE;

    /*
	 *  アドレス・アクノレッジ許可
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_ADDRIE | I2C_CR1_RXIE));
	err = i2c_transwait(hi2c, 500);

	/*
	 *  ロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
    return err;
}

/*
 *  I2Cスレーブ送信
 *  parameter1 hi2c   I2Cハンドラへのポインタ
 *  parameter2 pData: 読み出しバッファへのポインタ
 *  parameter3 Size:  読み出しサイズ
 *  return            正常終了ならE_OK
 */
ER
i2c_slavewrite(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size)
{
	ER err = E_OK;

	if(pData == NULL || Size == 0)
		return E_PAR;

	/*
	 *  読み出しロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->State != I2C_STATE_READY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

    hi2c->State = I2C_STATE_SLAVE_BUSY_TX;
    hi2c->ErrorCode   = I2C_ERROR_NONE;

    /*
	 *  アドレス・アクノレッジ許可
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_ADDRIE | I2C_CR1_TXIE));
	err = i2c_transwait(hi2c, 500);

	/*
	 *  ロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
    return err;
}

/*
 *  I2Cマスターデータリード
 *  parameter1 hi2c        I2Cハンドラへのポインタ
 *  parameter2 DevAddress: スレーブアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 pData:      読み出しバッファへのポインタ
 *  parameter6 Size:       読み出しサイズ
 *  parameter7 Timeout:    タイムアウト時間(ms)
 *  return                 正常終了ならE_OK
 */
ER
i2c_memread(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	ER err = E_OK;
	uint32_t Mode = I2C_CR2_RELOAD;

	if((pData == NULL) || (Size == 0))
		return E_PAR;

	/*
	 *  読み出しロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->State != I2C_STATE_READY
		 || (sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BUSY) == I2C_ISR_BUSY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

	hi2c->State = I2C_STATE_MASTER_BUSY_RX;
	hi2c->ErrorCode = I2C_ERROR_NONE;

	hi2c->pBuffPtr = pData;
	hi2c->XferCount = Size;
	if(Size > 255)
		hi2c->XferSize = 255;
	else
		hi2c->XferSize = Size;

	/* スレーブアドレスと読み出しアドレスの設定 */
	if(MemAddSize != 0){
	    if((err = i2c_requestmemaddr(hi2c, DevAddress, MemAddress, MemAddSize, I2C_SOFTEND_MODE, Timeout)) != E_OK){
			if(hi2c->Init.semlock != 0)
				sig_sem(hi2c->Init.semlock);
			return err;
		}
	}

	/*
	 *  アドレス書き込みフェーズ
	 */
    if( (hi2c->XferSize == 255) && (hi2c->XferSize < hi2c->XferCount) )
		Mode = I2C_CR2_RELOAD;
	else
		Mode = I2C_CR2_AUTOEND;
	i2c_transfarconfig(hi2c, DevAddress, hi2c->XferSize, Mode, (I2C_CR2_START | I2C_CR2_RD_WRN));

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_RXIE));
	err = i2c_transwait(hi2c, Timeout);

	/*
	 *  ロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return err;
}

/*
 *  I2Cマスターデータライト
 *  parameter1 hi2c        I2Cハンドラへのポインタ
 *  parameter2 DevAddress: スレーブアドレス
 *  parameter3 MemAddress: メモリアドレス
 *  parameter4 MemAddSize: メモリアドレスサイズ
 *  parameter5 pData:      書込みバッファへのポインタ
 *  parameter6 Size:       書込みサイズ
 *  parameter7 Timeout:    タイムアウト時間(ms)
 */
ER
i2c_memwrite(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	ER err = E_OK;
	uint32_t Mode = I2C_CR2_RELOAD;
	uint32_t Request = I2C_CR2_START /*I2C_GENERATE_START_WRITE*/;

	if((pData == NULL) || (Size == 0))
		return E_PAR;

	/*
	 *  書込みロック
	 */
	if(hi2c->Init.semlock != 0)
		wai_sem(hi2c->Init.semlock);

	if(hi2c->State != I2C_STATE_READY
		  || (sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BUSY) == I2C_ISR_BUSY){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

	hi2c->State = I2C_STATE_MASTER_BUSY_TX;
	hi2c->ErrorCode = I2C_ERROR_NONE;

	hi2c->pBuffPtr = pData;
	hi2c->XferCount = Size;
	if(Size > 255)
		hi2c->XferSize = 255;
	else
		hi2c->XferSize = Size;

	/* スレーブアドレスと書込みアドレスの設定 */
	if(MemAddSize != 0){
		Request = I2C_NO_STARTSTOP;
		if((err = i2c_requestmemaddr(hi2c, DevAddress, MemAddress, MemAddSize, I2C_CR2_RELOAD, Timeout)) != E_OK){
			if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
			return err;
		}
	}

	/*
	 *  アドレス書き込みフェーズ
	 */
    if(hi2c->XferSize == 255 && hi2c->XferSize < hi2c->XferCount)
		Mode = I2C_CR2_RELOAD;
	else
		Mode = I2C_CR2_AUTOEND;
	i2c_transfarconfig(hi2c, DevAddress, hi2c->XferSize, Mode, Request);

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_TXIE));
	err = i2c_transwait(hi2c, Timeout);

	/*
	 *  ロック解除
	 */
	if(hi2c->Init.semlock != 0)
		sig_sem(hi2c->Init.semlock);
	return err;
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
	uint32_t tick = 0;
	uint32_t tmp;

	if(hi2c->State == I2C_STATE_READY){
		if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BUSY) != 0)
			return E_OBJ;

		/*
		 *  アクセスロック
		 */
		if(hi2c->Init.semlock != 0)
			wai_sem(hi2c->Init.semlock);

		hi2c->State =     I2C_STATE_BUSY;
		hi2c->ErrorCode = I2C_ERROR_NONE;

		do{
			/* スタート生成 */
			if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
				tmp = (((DevAddress & I2C_CR2_SADD) | I2C_CR2_START | I2C_CR2_AUTOEND) & ~I2C_CR2_RD_WRN);
			else
				tmp = (((DevAddress & I2C_CR2_SADD) | I2C_CR2_ADD10 | I2C_CR2_START) & ~I2C_CR2_RD_WRN);
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), tmp);

		    /*
			 *  ストップフラグまたはNACKフラグ待ち
			 */
			while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & (I2C_ISR_STOPF | I2C_ISR_NACKF)) == 0){
				if(tick > Timeout){
					/* Device is ready */
					hi2c->State = I2C_STATE_READY;
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}
				dly_tsk(1);
				tick++;
			}

			/* NACKフラグ状態 */
			if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_NACKF) == 0){
				/* ストップフラグリセット待ち */
				if(i2c_waitsetflag(hi2c, I2C_ISR_STOPF, Timeout) != E_OK){
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}

				/* ストップフラグクリア */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);

				hi2c->State = I2C_STATE_READY;
				/*
				 *  ロック解除
				 */
				if(hi2c->Init.semlock != 0)
					sig_sem(hi2c->Init.semlock);
				return E_OK;
			}
			else{	/* ストップフラグ状態 */
				/* ストップフラグリセット待ち */ 
				if(i2c_waitsetflag(hi2c, I2C_ISR_STOPF, Timeout) != E_OK){
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}

				/* NACKフラグクリア */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);

				/* ストップフラグクリア */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
			}

			/* Check if the maximum allowed number of trials has been reached */
			if(I2C_Trials++ == Trials){
				/* ストップ生成 */
				sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_STOP);

				/* ストップフラグリセット待ち */ 
				if(i2c_waitsetflag(hi2c, I2C_ISR_STOPF, Timeout) != E_OK){
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}

				/* ストップフラグクリア */
				sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
			}
		}while(I2C_Trials < Trials);

		hi2c->State = I2C_STATE_READY;
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
 *  I2Cマスター送信割込み
 */
static void
i2c_mastertrans_isr(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));
	uint16_t DevAddress;

	if((isr & I2C_ISR_TXIS) != 0){	/* 送信中 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (*hi2c->pBuffPtr++));
		hi2c->XferSize--;
		hi2c->XferCount--;
	}
	else if((isr & I2C_ISR_TCR) != 0){	/* 送信再設定 */
		if(hi2c->XferSize == 0 && hi2c->XferCount != 0){
			DevAddress = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2)) & I2C_CR2_SADD;
			if(hi2c->XferCount > 255){
				i2c_transfarconfig(hi2c,DevAddress,255,  I2C_CR2_RELOAD, I2C_NO_STARTSTOP);
				hi2c->XferSize = 255;
			}
			else{
				i2c_transfarconfig(hi2c,DevAddress,hi2c->XferCount, I2C_CR2_AUTOEND, I2C_NO_STARTSTOP);
				hi2c->XferSize = hi2c->XferCount;
			}
		}
		else{	/* サイズエラー */
			hi2c->ErrorCode |= I2C_ERROR_SIZE;
			if(hi2c->errorcallback != NULL)
				hi2c->errorcallback(hi2c);
		}
	}
	else if((isr & I2C_ISR_TC) != 0){	/* 送信終了 */
		if(hi2c->XferCount == 0){
			sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_STOP);
		}
		else{	/* サイズエラー */
			hi2c->ErrorCode |= I2C_ERROR_SIZE;
			if(hi2c->errorcallback != NULL)
				hi2c->errorcallback(hi2c);
		}
	}
	else if((isr & I2C_ISR_STOPF) != 0){	/* ストップフラグ */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_TXIE));
		/*
		 *  ストップフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
		/*
		 * CR2をクリア
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_RESET);
		hi2c->State = I2C_STATE_READY;
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
	else if((isr & I2C_ISR_NACKF) != 0){	/* NACKフラグ */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		hi2c->ErrorCode |= I2C_ERROR_AF;
		if(hi2c->errorcallback != NULL)
			hi2c->errorcallback(hi2c);
	}
}

/*
 *  I2Cマスター受信割込み
 */
static void
i2c_masterreceiv_isr(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));
	uint16_t DevAddress;

	if((isr & I2C_ISR_RXNE) != 0){		/* 受信中 */
		(*hi2c->pBuffPtr++) = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));
		hi2c->XferSize--;
		hi2c->XferCount--;
	}
	else if((isr & I2C_ISR_TCR) != 0){	/* 受信継続 */
		if(hi2c->XferSize == 0 && hi2c->XferCount != 0){
			DevAddress = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2)) & I2C_CR2_SADD;
			if(hi2c->XferCount > 255){
				i2c_transfarconfig(hi2c, DevAddress, 255, I2C_CR2_RELOAD, I2C_NO_STARTSTOP);
				hi2c->XferSize = 255;
			}
			else{
				i2c_transfarconfig(hi2c, DevAddress, hi2c->XferCount, I2C_CR2_AUTOEND, I2C_NO_STARTSTOP);
				hi2c->XferSize = hi2c->XferCount;
			}
		}
		else{	/* サイズエラー */
			hi2c->ErrorCode |= I2C_ERROR_SIZE;
			if(hi2c->errorcallback != NULL)
				hi2c->errorcallback(hi2c);
		}
	}
	else if((isr & I2C_ISR_TC) != 0){	/* 受信終了 */
		if(hi2c->XferCount == 0){
			sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_STOP);
		}
		else{
			/* Wrong size Status regarding TCR flag event */
			hi2c->ErrorCode |= I2C_ERROR_SIZE;
			if(hi2c->errorcallback != NULL)
				hi2c->errorcallback(hi2c);
		}
	}
	else if((isr & I2C_ISR_STOPF) != 0){	/* ストップフラグ */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_RXIE));
		/*
		 *  ストップフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
		/*
		 * CR2をクリア
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_RESET);
		hi2c->State = I2C_STATE_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
	else if((isr & I2C_ISR_NACKF) != 0){	/* NACKフラグ */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		hi2c->ErrorCode |= I2C_ERROR_AF;
		if(hi2c->errorcallback != NULL)
			hi2c->errorcallback(hi2c);
	}
}

/*
 *  I2Cスレーブ送信割込み
 */
static void
i2c_slavesend_isr(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	if((isr & I2C_ISR_NACKF) != 0){	/* NACKフラグ */
		if(hi2c->XferCount == 0){	/* NACKをクリア */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		}
		else{		/* データありでアクノレッジなしエラー */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
			hi2c->ErrorCode |= I2C_ERROR_AF;
			if(hi2c->errorcallback != NULL)
				hi2c->errorcallback(hi2c);
		}
	}
	else if((isr & I2C_ISR_ADDR) != 0){
		/* Clear ADDR flag */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ADDRCF);
	}
	else if((isr & I2C_ISR_STOPF) != 0){	/* ストップフラグ */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_ADDRIE | I2C_CR1_RXIE | I2C_CR1_TXIE));
	    /*
		 *  アドレス・アクノレッジ禁止
	 	 */
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);
		/*
		 *  ストップフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
		hi2c->State = I2C_STATE_READY;
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
	else if((isr & I2C_ISR_TXIS) != 0){	/* 送信状態 */
		if(hi2c->XferCount > 0){
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TXDR), (*hi2c->pBuffPtr++));
			hi2c->XferCount--;
		}
	}
}

/*
 *  I2Cスレーブ受信割込み
 */
static void
i2c_slavereceiv_isr(I2C_Handle_t *hi2c)
{
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	if((isr & I2C_ISR_NACKF) != 0){		/* NACKフラグ */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_NACKCF);
		hi2c->ErrorCode |= I2C_ERROR_AF;
		if(hi2c->errorcallback != NULL)
			hi2c->errorcallback(hi2c);
	}
	else if((isr & I2C_ISR_ADDR) != 0){	/* アドレスマッチ */
	    sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ADDRCF);
	}
	else if((isr & I2C_ISR_RXNE) != 0){	/* 受信中 */
		(*hi2c->pBuffPtr++) = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_RXDR));
		hi2c->XferSize--;
		hi2c->XferCount--;
	}
	else if((isr & I2C_ISR_STOPF) != 0){/* ストップフラグ */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (I2C_CR1_ERROR | I2C_CR1_ADDRIE | I2C_CR1_RXIE));
	    /*
		 *  アドレス・アクノレッジ禁止
	 	 */
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_NACK);
		/*
		 *  ストップフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_STOPCF);
		hi2c->State = I2C_STATE_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
}

/*
 *  I2C EV割込みハンドラ
 */
void
i2c_ev_handler(I2C_Handle_t *hi2c)
{
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1));
	uint32_t isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 * I2C スレーブ送信モード
	 */
	if(((isr & (I2C_ISR_TXIS | I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_STOPF | I2C_ISR_NACKF | I2C_ISR_ADDR)) != 0) && ((cr1 & I2C_CR1_TRANS1) == I2C_CR1_TRANS1)){     
		if(hi2c->State == I2C_STATE_SLAVE_BUSY_TX){
			i2c_slavesend_isr(hi2c);
		}
	}
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 *  I2C マスター送信モード
	 */
	if(((isr & (I2C_ISR_TXIS | I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_STOPF | I2C_ISR_NACKF)) != 0) && ((cr1 & I2C_CR1_TRANS2) == I2C_CR1_TRANS2)){
		if(hi2c->State == I2C_STATE_MASTER_BUSY_TX){
			i2c_mastertrans_isr(hi2c);
		}
	}
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 *  I2C スレーブ受信モード
	 */
	if(((isr & (I2C_ISR_RXNE | I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_STOPF | I2C_ISR_NACKF | I2C_ISR_ADDR)) != 0) && ((cr1 & I2C_CR1_RECV1) == I2C_CR1_RECV1)){
		if(hi2c->State == I2C_STATE_SLAVE_BUSY_RX){
			i2c_slavereceiv_isr(hi2c);
		}
	} 
	isr = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR));

	/*
	 *  I2C マスター受信モード
	 */
	if(((isr & (I2C_ISR_RXNE | I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_STOPF | I2C_ISR_NACKF)) != 0) && ((cr1 & I2C_CR1_RECV2) == I2C_CR1_RECV2)){
		if(hi2c->State == I2C_STATE_MASTER_BUSY_RX){
			i2c_masterreceiv_isr(hi2c);
		}
	}

	if(hi2c->State == I2C_STATE_READY && hi2c->Init.semid != 0)
		isig_sem(hi2c->Init.semid);
}

/*
 *  I2C ER割込みハンドラ
 */
void
i2c_er_handler(I2C_Handle_t *hi2c)
{
	uint32_t cr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1));

	/*
	 *  I2C バスエラー
	 */
	if(((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_BERR) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_BERR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_BERRCF);
	}

	/*
	 *  I2C オーバーラン/アンダーラン
	 */
	if(((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_OVR) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_OVR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_OVRCF);
	}

	/*
	 *  I2C アービトレーション・ロス・エラー
	 */
	if(((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_ISR)) & I2C_ISR_ARLO) != 0) && ((cr1 & I2C_CR1_ERRIE) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_ARLO;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_ICR), I2C_ICR_ARLOCF);
	}

	/*
	 *  エラーコールバック関数呼び出し
	 */
	if(hi2c->ErrorCode != I2C_ERROR_NONE){
		hi2c->State = I2C_STATE_READY;
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

