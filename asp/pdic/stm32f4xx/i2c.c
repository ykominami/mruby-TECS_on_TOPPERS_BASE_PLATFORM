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
 *  @(#) $Id: i2c.c 698 2016-02-10 12:05:16Z roi $
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
#ifdef TOPPERS_STM32F401_NUCLEO
#define GPIO_AFN_I2C2          ((uint8_t)0x09)  /* I2C2 Alternate Function mapping */
#define GPIO_AFN_I2C3          ((uint8_t)0x09)  /* I2C3 Alternate Function mapping */
#define GPIO_I2C3_SDA          TADR_GPIOB_BASE
#define I2C3_SDA               4
#else
#define GPIO_AFN_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping */
#define GPIO_AFN_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping */
#define GPIO_I2C3_SDA          TADR_GPIOC_BASE
#define I2C3_SDA               9
#endif
#ifdef TOPPERS_STM32F4_DISCOVERY
#define I2C2_SDA               11
#else
#define I2C2_SDA               3
#endif

#define I2C_TIMEOUT_FLAG            35			/* 35 ms */
#define I2C_TIMEOUT_BUSY_FLAG       10000		/* 10 s  */

#define I2C_FREQRANGE(_PCLK_)       ((_PCLK_)/1000000)

#define I2C_RISE_TIME(_FREQ_, _SPEED_)            (((_SPEED_) <= 100000) ? ((_FREQ_) + 1) : ((((_FREQ_) * 300) / 1000) + 1))
#define I2C_SPEED_STANDARD(_PCLK_, _SPEED_)       (((((_PCLK_)/((_SPEED_) << 1)) & I2C_CCR_CCR) < 4)? 4:((_PCLK_) / ((_SPEED_) << 1)))
#define I2C_SPEED_FAST(_PCLK_, _SPEED_, _DCYCLE_) (((_DCYCLE_) == I2C_DUTYCYCLE_2)? ((_PCLK_) / ((_SPEED_) * 3)) : (((_PCLK_) / ((_SPEED_) * 25)) | I2C_DUTYCYCLE_16_9))
#define I2C_SPEED(_PCLK_, _SPEED_, _DCYCLE_)      (((_SPEED_) <= 100000)? (I2C_SPEED_STANDARD((_PCLK_), (_SPEED_))) : \
                                                                  ((I2C_SPEED_FAST((_PCLK_), (_SPEED_), (_DCYCLE_)) & I2C_CCR_CCR) == 0)? 1 : \
                                                                  ((I2C_SPEED_FAST((_PCLK_), (_SPEED_), (_DCYCLE_))) | I2C_CCR_FS))

#define I2C_7BIT_ADD_WRITE(_ADDR_)  ((uint8_t)((_ADDR_) & (~I2C_OAR1_ADD0)))
#define I2C_7BIT_ADD_READ(_ADDR_)   ((uint8_t)((_ADDR_) | I2C_OAR1_ADD0))

#define I2C_10BIT_ADDRESS(_ADDR_)   ((uint8_t)(((_ADDR_) & 0x00FF)))
#define I2C_10BIT_HEADWRITE(_ADDR_) ((uint8_t)(((((_ADDR_) & 0x0300) >> 7) | 0xF0)))
#define I2C_10BIT_HEADREAD(_ADDR_)  ((uint8_t)(((((_ADDR_) & 0x0300) >> 7) | 0xF1)))

#define I2C_MEM_ADD_MSB(_ADDR_)     ((uint8_t)(((_ADDR_) & 0xFF00) >> 8))
#define I2C_MEM_ADD_LSB(_ADDR_)     ((uint8_t)((_ADDR_) & 0x00FF))


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
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIOBEN, RCC_AHB1ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C2EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C2RST,
	TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	10, GPIO_AF4_I2C2, I2C2_SDA, GPIO_AFN_I2C2 },

  {	TADR_I2C3_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR),  RCC_AHB1ENR_GPIOAEN, RCC_AHB1ENR_GPIOBEN,
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR),  RCC_APB1ENR_I2C3EN,
	(TADR_RCC_BASE+TOFF_RCC_APB1RSTR), RCC_APB1RSTR_I2C3RST,
	TADR_GPIOA_BASE, GPIO_I2C3_SDA,
	8, GPIO_AF4_I2C3, I2C3_SDA, GPIO_AFN_I2C3 }
};


/*
 * I2C ハンドラ
 */
static I2C_Handle_t I2cHandle[NUM_I2CPORT];

/*
 *  SR1レジスタセット待ち
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  parameter2 Flag:    待ちフラグ
 *  parameter3 Timeout: 待ち時間(ms)
 *  return              正常終了時、E_OK
 */
static ER
i2c_sr1flagsetwait(I2C_Handle_t *hi2c, uint32_t Flag, uint32_t Timeout)
{
	uint32_t tick = 0;
	uint32_t ctime = Timeout * 1000;

	while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & Flag) == 0){
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

    while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2)) & I2C_SR2_BUSY) != 0){
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
 * アドレス系フラグ設定待ち
 *  parameter1 hi2c:    I2Cハンドラへのポインタ
 *  parameter2 Flag:    待ちフラグ
 *  parameter3 Timeout: 待ち時間(ms)
 *  return              正常終了時、E_OK
 */
static ER
i2c_addrflagwait(I2C_Handle_t *hi2c, uint32_t Flag, uint32_t Timeout)
{
	uint32_t tick = 0;
	uint32_t ctime = Timeout * 1000;

	while((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & Flag) == 0){
		/*
		 *  アクノレッジフォルト判定
		 */
		if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & I2C_SR1_AF) != 0){
			sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_AF);
			hi2c->ErrorCode = I2C_ERROR_AF;
			hi2c->status= I2C_STATUS_READY;
			return E_SYS;
		}
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
 *  アドレスフラグクリア
 *  parameter1 hi2c:   I2Cハンドラへのポインタ
 */
static void
i2c_clear_addr(I2C_Handle_t *hi2c)
{
	volatile uint32_t tmp;

	tmp = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));
	tmp = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2));
	(void)(tmp);
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
	return E_OK;
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

	/*
	 *  アクノレッジ設定許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);

	/*
	 *  スタート設定と設定待ち
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_START);
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_SB, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  デバイスアドレス設定と設定待ち
	 */
	if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT){
		if(MemAddSize == 0)	/* メモリアドレスを設定しない場合、受信設定 */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_7BIT_ADD_READ(DevAddress));
		else
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_7BIT_ADD_WRITE(DevAddress));
	}
	else{	/* メモリアドレス設定なし */
		/*
		 *  10ビットヘッダ設定と設定待ち
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_10BIT_HEADWRITE(DevAddress));
		if((ercd = i2c_addrflagwait(hi2c, I2C_SR1_ADD10, Timeout)) != E_OK){
			return ercd;
		}

		/*
		 *  10ビットアドレス設定と設定待ち
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_10BIT_ADDRESS(DevAddress));
		if((ercd = i2c_addrflagwait(hi2c, I2C_SR1_ADDR, Timeout)) != E_OK){
			return ercd;
		}

		/*
		 *  アドレスフラグクリア
		 */
		i2c_clear_addr(hi2c);

 		/*
		 *  リスタート設定と設定待ち
		 */
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_START);
		if(i2c_sr1flagsetwait(hi2c, I2C_SR1_SB, Timeout) != E_OK){
			return E_TMOUT;
	    }

	   	/*
		 *  スレーブアドレスを受信モードで設定
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_10BIT_HEADREAD(DevAddress));
	}

	/*
	 *  アドレスフラグ設定待ち
	 */
	if((ercd = i2c_addrflagwait(hi2c, I2C_SR1_ADDR, Timeout)) != E_OK){
		return ercd;
	}

	/*
	 *  メモリアドレス設定なしなら正常終了
	 */
	if(MemAddSize == 0)
		return E_OK;

	/*
	 *  アドレスフラグクリア
	 */
	i2c_clear_addr(hi2c);

	/*
	 *  TXEフラグ設定まで待ち
	 */
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_TXE, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  メモリアドレス設定と設定待ち
	 */
	if(MemAddSize == I2C_MEMADD_SIZE_8BIT){
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_LSB(MemAddress));
	}
	else{	/* 16ビットケース */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_MSB(MemAddress));
		if(i2c_sr1flagsetwait(hi2c, I2C_SR1_TXE, Timeout) != E_OK){
			return E_TMOUT;
		}
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_LSB(MemAddress));
	}

	/*
	 *  TXEフラグ設定まで待ち
	 */
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_TXE, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  リスタート設定と設定待ち
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_START);
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_SB, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  デバイスアドレスを受信モードで設定(7ビットのみ)
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_7BIT_ADD_READ(DevAddress));

	/*
	 *  アドレスフラグ設定待ち
	 */
	return i2c_addrflagwait(hi2c, I2C_SR1_ADDR, Timeout);
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

	/*
	 *  スタート設定と設定待ち
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_START);
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_SB, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  デバイスアドレス設定と設定待ち
	 */
	if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT){
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_7BIT_ADD_WRITE(DevAddress));
	}
	else{
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_10BIT_HEADWRITE(DevAddress));
		if((ercd = i2c_addrflagwait(hi2c, I2C_SR1_ADD10, Timeout)) != E_OK){
			return ercd;
		}

		/*
		 *  10ビットアドレス設定と設定待ち
		 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_10BIT_ADDRESS(DevAddress));
	}

	/*
	 *  アドレスフラグ設定待ち
	 */
	if((ercd = i2c_addrflagwait(hi2c, I2C_SR1_ADDR, Timeout)) != E_OK){
		return ercd;
	}

	/*
	 *  アドレスフラグクリア
	 */
	i2c_clear_addr(hi2c);

	/*
	 *  メモリアドレス設定なしなら正常終了
	 */
	if(MemAddSize == 0)
		return E_OK;

	/*
	 *  TXEフラグ設定まで待ち
	 */
	if(i2c_sr1flagsetwait(hi2c, I2C_SR1_TXE, Timeout) != E_OK){
		return E_TMOUT;
	}

	/*
	 *  メモリアドレス設定と設定待ち
	 */
	if(MemAddSize == I2C_MEMADD_SIZE_8BIT){
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_LSB(MemAddress));
	}
	else{	/* 16ビットケース */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_MSB(MemAddress));
		if(i2c_sr1flagsetwait(hi2c, I2C_SR1_TXE, Timeout) != E_OK){
			return E_TMOUT;
		}
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_MEM_ADD_LSB(MemAddress));
	}
	return E_OK;
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
	uint32_t freqrange = 0;
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
	 * 基本周波数(PCLK1)よりレンジを取得
	 */
	freqrange = I2C_FREQRANGE(SysFrePCLK1);

	/*
	 *  I2C周波数レンジ設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), freqrange);

	/*
	 *  I2Cライズ時間設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_TRISE), I2C_RISE_TIME(freqrange, hi2c->Init.ClockSpeed));

	/*
	 *  I2Cクロックスピード設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CCR), I2C_SPEED(SysFrePCLK1, hi2c->Init.ClockSpeed, hi2c->Init.DutyCycle));

	/*
	 *  I2C CR1設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode));

	/*
	 *  I2Cデバイスアドレス１設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR1), (hi2c->Init.AddressingMode | hi2c->Init.OwnAddress1));

	/*
	 *  I2Cデバイスアドレス２設定
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_OAR2), (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2));

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

	if(hi2c->status != I2C_STATUS_READY || i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

	/*
	 *  Acknowledge/PEC Position禁止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);

    hi2c->status = I2C_STATUS_BUSY_RX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;

    /*
	 *  アドレス・アクノレッジ許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
	ercd = i2c_transwait(hi2c, 500);

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

	if(hi2c->status != I2C_STATUS_READY || i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return E_OBJ;
	}

	/*
	 *  Acknowledge/PEC Position禁止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);

    hi2c->status = I2C_STATUS_BUSY_TX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;

    /*
	 *  アドレス・アクノレッジ許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
	ercd = i2c_transwait(hi2c, 500);

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

	/*
	 *  Acknowledge/PEC Position禁止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);

    hi2c->status = I2C_STATUS_BUSY_RX;
    hi2c->ErrorCode = I2C_ERROR_NONE;

    hi2c->pBuffPtr = pData;
    hi2c->XferSize = Size;
    hi2c->XferCount = Size;

	/* スレーブアドレスと読み出しアドレスの設定 */
	if((ercd = i2c_masterreadaddress(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG)) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
        return ercd;
	}

	if(hi2c->XferCount == 1){
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);
		i2c_clear_addr(hi2c);
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);
	}
	else if(hi2c->XferCount == 2){
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);
		i2c_clear_addr(hi2c);
	}
	else{
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);
		i2c_clear_addr(hi2c);
	}

    /*
	 *  読み出し割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
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

	/*
	 *  Acknowledge/PEC Position禁止
	 */
	sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);

	hi2c->status = I2C_STATUS_BUSY_TX;
	hi2c->ErrorCode = I2C_ERROR_NONE;

	hi2c->pBuffPtr = pData;
	hi2c->XferSize = Size;
	hi2c->XferCount = Size;

	/* スレーブアドレスと書込みアドレスの設定 */
	if((ercd = i2c_masterwriteaddress(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG)) != E_OK){
		if(hi2c->Init.semlock != 0)
			sig_sem(hi2c->Init.semlock);
		return ercd;
	}

    /*
	 *  書き込み割込み許可
	 */
	sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
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
	uint32_t tmp1 = 0, tmp2 = 0;
	uint32_t tick = 0;

	if(hi2c->status == I2C_STATUS_READY){
		if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2)) & I2C_SR2_BUSY) != 0)
			return E_OBJ;

		/*
		 *  アクセスロック
		 */
		if(hi2c->Init.semlock != 0)
			wai_sem(hi2c->Init.semlock);

		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);
		hi2c->status    = I2C_STATUS_BUSY;
		hi2c->ErrorCode = I2C_ERROR_NONE;

		do{
			/* START生成 */
			sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_START);

			/* START BIT待ち */
			if(i2c_sr1flagsetwait(hi2c, I2C_SR1_SB, Timeout) != E_OK){
				/*
				 *  ロック解除
				 */
				if(hi2c->Init.semlock != 0)
					sig_sem(hi2c->Init.semlock);
				return E_TMOUT;
			}

			/* スレーブアドレス送信 */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), I2C_7BIT_ADD_WRITE(DevAddress));

			/* アドレス送信、アクノレッジ待ち */
			tmp1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & (I2C_SR1_ADDR | I2C_SR1_AF);
			tmp2 = hi2c->status;
			while(tmp1 == 0 && tmp2 != I2C_STATUS_TIMEOUT){
				if(tick > Timeout){
					hi2c->status = I2C_STATUS_TIMEOUT;
				}
				tmp1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & (I2C_SR1_ADDR | I2C_SR1_AF);
				tmp2 = hi2c->status;
				dly_tsk(1);
				tick++;
			}
			hi2c->status = I2C_STATUS_READY;

			/* アドレス送信フラグチェック */
			if((sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1)) & I2C_SR1_ADDR) != 0){
				/* ストップ送信 */
				sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);

				/* アドレス送信フラグクリア */
				tmp1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));
				tmp1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2));

				/* レディ待ち */
				if(i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}

				hi2c->status = I2C_STATUS_READY;
				/*
				 *  ロック解除
				 */
				if(hi2c->Init.semlock != 0)
					sig_sem(hi2c->Init.semlock);
				return E_OK;
			}
			else{	/* アクノレッジ状態 */
				/* ストップ送信 */
				sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);

				/* アドレス送信フラグリセット */
				sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), I2C_SR1_ADDR);

				/* レディ待ち */
				if(i2c_busreadywait(hi2c, I2C_TIMEOUT_BUSY_FLAG) != E_OK){
					/*
					 *  ロック解除
					 */
					if(hi2c->Init.semlock != 0)
						sig_sem(hi2c->Init.semlock);
					return E_TMOUT;
				}
			}
		}while(I2C_Trials++ < Trials);

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
 *  I2C１バイト送信
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 */
static void i2c_transmit_byte(I2C_Handle_t *hi2c)
{
	if(hi2c->XferCount != 0){
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), (uint32_t)(*hi2c->pBuffPtr++));
		hi2c->XferCount--;
	}
}

/*
 *  I2C１バイト受信
 *  parameter1 hi2c:       I2Cハンドラへのポインタ
 */
static void i2c_receive_byte(I2C_Handle_t *hi2c)
{
	if(hi2c->XferCount != 0){
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;
	}
}

/*
 *  I2Cマスター送信割込み(BTF設定時)
 */
static void
i2c_mastertrans_BTF(I2C_Handle_t *hi2c)
{
	if(hi2c->XferCount != 0){	/* データ送信 */
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), (uint32_t)(*hi2c->pBuffPtr++));
		hi2c->XferCount--;
	}
	else{	/* 送信終了 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);

		hi2c->status = I2C_STATUS_READY;
		if(hi2c->writecallback != NULL)
			hi2c->writecallback(hi2c);
	}
}


/*
 *  I2Cマスター送信割込み(TXE設定時)
 */
static void
i2C_MasterTransmit_TXE(I2C_Handle_t *hi2c)
{
	/*
	 *  １バイト送信
	 */
	sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR), (uint32_t)(*hi2c->pBuffPtr++));
	hi2c->XferCount--;

	if(hi2c->XferCount == 0){	/* 停止 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_ITBUFEN);
	}
}

/*
 *  I2Cマスター受信割込み(RXNE設定時)
 */
static void
i2c_masterreceiv_RXNE(I2C_Handle_t *hi2c)
{
	uint32_t tmp = 0;

	tmp = hi2c->XferCount;
	if(tmp > 3){	/* 残り4バイト以上 */
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;
	}
	else if(tmp == 2 || tmp == 3){	/* 残り2または3バイト */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), I2C_CR2_ITBUFEN);
	}
	else{	/* 受信終了 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));

		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;

		hi2c->status = I2C_STATUS_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
}

/*
 *  I2Cマスター受信割込み(BTF設定時)
 */
static void
i2c_masterreceiv_BTF(I2C_Handle_t *hi2c)
{
	if(hi2c->XferCount == 3){		/* 残りカウント3バイト */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;
	}
	else if(hi2c->XferCount == 2){	/* 残りカウント2バイト */
		sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_STOP);
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;

		/*
		 *  割込み停止
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN));

		hi2c->status = I2C_STATUS_READY;
		if(hi2c->readcallback != NULL)
			hi2c->readcallback(hi2c);
	}
	else{
		(*hi2c->pBuffPtr++) = (uint8_t)sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_DR));
		hi2c->XferCount--;
	}
}

/*
 *  I2C EV割込みハンドラ
 */
void
i2c_ev_handler(I2C_Handle_t *hi2c)
{
	uint32_t sr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));
	uint32_t sr2 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2));
	uint32_t cr2 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2));
	volatile uint32_t tmp;

	/*
	 *  マスターモード
	 */
	if((sr2 & I2C_SR2_MSL) != 0){
		if((sr2 & I2C_SR2_TRA) != 0){	/* 送信モード */
			if(((sr1 & I2C_SR1_TXE) != 0) && ((cr2 & I2C_CR2_ITBUFEN) != 0) && ((sr1 & I2C_SR1_BTF) == 0)){
				i2C_MasterTransmit_TXE(hi2c);
			}
			else if(((sr1 & I2C_SR1_BTF) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){
				i2c_mastertrans_BTF(hi2c);
			}
		}
		else{							/* 受信モード */
			if(((sr1 & I2C_SR1_RXNE) != 0) && ((cr2 & I2C_CR2_ITBUFEN) != 0) && ((sr1 & I2C_SR1_BTF) == 0)){
				i2c_masterreceiv_RXNE(hi2c);
			}
 			else if(((sr1 & I2C_SR1_BTF) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){
				i2c_masterreceiv_BTF(hi2c);
			}
		}
		if(hi2c->status == I2C_STATUS_READY && hi2c->Init.semid != 0)
			isig_sem(hi2c->Init.semid);
	}
	/*
	 *  スレーブモード
	 */
	else{
		if(((sr1 & I2C_SR1_ADDR) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){	/* ADDRフラグ */
			i2c_clear_addr(hi2c);
		}
		else if(((sr1 & I2C_SR1_STOPF) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){	/* STOPフラグ */
			sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
			/*
			 * STOPフラグクリア
			 */
			tmp = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));
			sil_orw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_PE);
			(void)(tmp);

		    /*
			 *  アドレス・アクノレッジ禁止
			 */
			sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);

			hi2c->status = I2C_STATUS_READY;
			if(hi2c->readcallback != NULL)
				hi2c->readcallback(hi2c);
		}
		else if((sr2 & I2C_SR2_TRA) != 0){	/* スレーブ送信 */
			if(((sr1 & I2C_SR1_TXE) != 0) && ((cr2 & I2C_CR2_ITBUFEN) != 0) && ((sr1 & I2C_SR1_BTF) == 0)){
				i2c_transmit_byte(hi2c);
			}
			else if(((sr1 & I2C_SR1_BTF) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){
				i2c_transmit_byte(hi2c);
			}
		}
		else{								/* スレーブ受信 */
			if(((sr1 & I2C_SR1_RXNE) != 0) && ((cr2 & I2C_CR2_ITBUFEN) != 0) && ((sr1 & I2C_SR1_BTF) == 0)){
				i2c_receive_byte(hi2c);
			}
			else if(((sr1 & I2C_SR1_BTF) != 0) && ((cr2 & I2C_CR2_ITEVTEN) != 0)){
				i2c_receive_byte(hi2c);
			}
		}
	}
}

/*
 *  I2C ER割込みハンドラ
 */
void
i2c_er_handler(I2C_Handle_t *hi2c)
{
	uint32_t sr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));
	uint32_t cr2 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2));

	/*
	 *  I2C バスエラー
	 */
	if(((sr1 & I2C_SR1_BERR) != 0) && ((cr2 & I2C_CR2_ITERREN) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_BERR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_BERR);
	}
	sr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));

	/*
	 *  I2C アービテーション・ロス・エラー
	 */
	if(((sr1 & I2C_SR1_ARLO) != 0) && ((cr2 & I2C_CR2_ITERREN) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_ARLO;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_ARLO);
	}
	sr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));

	/*
	 *  I2C アクノレッジ・フェール・エラー
	 */
	if(((sr1 & I2C_SR1_AF) != 0) && ((cr2 & I2C_CR2_ITERREN) != 0)){
		uint32_t sr2 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR2));
		if(((sr2 & I2C_SR2_MSL) == 0) && (hi2c->XferCount == 0) && (hi2c->status == I2C_STATUS_BUSY_TX)){
			/* Disable EVT, BUF and ERR interrupt */
			sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR2), (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN));
			/* Clear AF flag */
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_AF);
			/* Disable Acknowledge */
			sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_ACK);
			hi2c->status = I2C_STATUS_READY;
			if(hi2c->writecallback != NULL)
				hi2c->writecallback(hi2c);
		}
		else{
			hi2c->ErrorCode |= I2C_ERROR_AF;
			sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_AF);
		}
	}
	sr1 = sil_rew_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1));

	/*
	 *  I2C オーバーラン/アンダーラン・エラー
	 */
	if(((sr1 & I2C_SR1_OVR) != 0/*tmp1 == SET*/) && ((cr2 & I2C_CR2_ITERREN) != 0)){
		hi2c->ErrorCode |= I2C_ERROR_OVR;
		sil_wrw_mem((uint32_t *)(hi2c->base+TOFF_I2C_SR1), ~I2C_SR1_OVR);
	}

	if(hi2c->ErrorCode != I2C_ERROR_NONE){
		hi2c->status = I2C_STATUS_READY;

		/*
		 *  Acknowledge/PEC Position禁止
		 */
		sil_andw_mem((uint32_t *)(hi2c->base+TOFF_I2C_CR1), I2C_CR1_POS);

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

