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
 *  @(#) $Id: spi.c 698 2016-04-15 07:49:53Z roi $
 */
/*
 * 
 *  SPIドライバ関数群
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"
#include "spi.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

#ifndef SPI_GPIO_PP
#define SPI_GPIO_PP     GPIO_PULLUP
#endif
#ifndef SPI_GPIO_SPEED
#define SPI_GPIO_SPEED  GPIO_SPEED_HIGH
#endif

/*
 *  SPIOポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_SPI(spiid)        ((uint_t)((spiid) - 1))

#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping        */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping        */

#define SPI_TIMEOUT_VALUE  (10*1000)

static const SPI_PortControlBlock spi_pcb[NUM_SPIPORT] = {
  {	TADR_SPI1_BASE,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOAEN, RCC_AHB1ENR_GPIOAEN,
	(TADR_RCC_BASE+TOFF_RCC_APB2ENR), RCC_APB2ENR_SPI1EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA2EN,
	TADR_GPIOA_BASE, TADR_GPIOA_BASE, TADR_GPIOA_BASE,
	5, 6, 7, GPIO_AF5_SPI1,
	DMA_CHANNEL_3, TADR_DMA2_STM3_BASE,
	DMA_CHANNEL_3, TADR_DMA2_STM2_BASE },

  {	TADR_SPI2_BASE,
#if defined(TOPPERS_STM32F769_DISCOVERY)
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOAEN, RCC_AHB1ENR_GPIOBEN,
#else
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_GPIOBEN, RCC_AHB1ENR_GPIOIEN,
#endif
	(TADR_RCC_BASE+TOFF_RCC_APB1ENR), RCC_APB1ENR_SPI2EN,
	(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_DMA1EN,
#if defined(TOPPERS_STM32F769_DISCOVERY)
	TADR_GPIOA_BASE, TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	12, 14, 15, GPIO_AF5_SPI2,
#elif defined(TOPPERS_STM32F7_DISCOVERY)
	TADR_GPIOI_BASE, TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	1, 14, 15, GPIO_AF5_SPI2,
#else
	TADR_GPIOB_BASE, TADR_GPIOB_BASE, TADR_GPIOB_BASE,
	13, 14, 15, GPIO_AF5_SPI2,
#endif
	DMA_CHANNEL_0, TADR_DMA1_STM4_BASE,
	DMA_CHANNEL_0, TADR_DMA1_STM3_BASE },
};

static SPI_Handle_t SpiHandle[NUM_SPIPORT];
static DMA_Handle_t hdma_tx[NUM_SPIPORT];
static DMA_Handle_t hdma_rx[NUM_SPIPORT];


/*
 *  SPI状態変化待ち関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  Flag: チェックを行うSPIフラグ
 *  parameter3  Status: 待ち状態
 *  parameter4  Timeout: タイムアウト時間(マイクロ秒)
 *  return ERコード
 */
static ER
spi_waitflag(SPI_Handle_t *hspi, uint32_t Flag, bool_t Status, uint32_t Timeout)
{
  uint32_t tick = 0;

	while(((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & Flag) == Flag) == Status){
		if(Timeout == 0 || tick > Timeout){
			/*
			 *  送受信とエラーフラグをリセット
			 */
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE));

			/*
			 *  SPIを停止
			 */
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

			/*
			 *  CRC演算を無効に
			 */
			if(hspi->Init.CRC == SPI_CRC_ENABLE){
				sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
				sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
			}

			hspi->status= SPI_STATUS_READY;
			return E_TMOUT;
		}
		tick++;
		sil_dly_nse(1000);
	}
	return E_OK;
}

/*
 *  SPI DMA送信コールバック関数
 */
static void
spi_dmatransmit_func(DMA_Handle_t *hdma)
{
	SPI_Handle_t* hspi = ( SPI_Handle_t* )hdma->localdata;
	volatile uint32_t tmp;

	/*
	 *  TXEフラグセット待ち
	 */
	if(spi_waitflag(hspi, SPI_SR_TXE, false, SPI_TIMEOUT_VALUE) != E_OK){
		hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
	}

	/*
	 *  TX DMA停止
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_TXDMAEN);

	/*
	 *  SPI BUSYリセット待ち
	 */
	if(spi_waitflag(hspi, SPI_SR_BSY, true, SPI_TIMEOUT_VALUE) != E_OK){
		hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
	}

	hspi->TxXferCount = 0;
	hspi->status = SPI_STATUS_READY;

	/*
	 *  2ライン転送でオーバーランが発生しているならリセット
	 */
	if(hspi->Init.Direction == SPI_DIRECTION_2LINES){
		tmp = sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_DR));
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_OVR);
		((void)(tmp));
	}
	if(hspi->Init.semid != 0)
		isig_sem(hspi->Init.semid);
}

/*
 *  SPI DMA受信コールバック関数
 */
static void
spi_dmareceive_func(DMA_Handle_t *hdma)
{
	SPI_Handle_t *hspi = (SPI_Handle_t *)hdma->localdata;
	volatile uint32_t tmp;

	if((hspi->Init.Mode == SPI_MODE_MASTER) &&
		(hspi->Init.Direction == SPI_DIRECTION_1LINE || hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)){
		/*
		 *  SPIを停止
		 */
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);
	}

	/*
	 *  RX DMA停止
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (SPI_CR2_RXDMAEN));

	hspi->RxXferCount = 0;

	/*
	 *  CRC演算
	 */
	if(hspi->Init.CRC == SPI_CRC_ENABLE){
		/*
		 *  受信終了待ち
		 */
		if(spi_waitflag(hspi, SPI_SR_RXNE, false, SPI_TIMEOUT_VALUE) != E_OK){
			hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
		}

		/*
		 *  CRC読み捨て
		 */
		tmp = sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_DR));
		((void)(tmp));

		/*
		 *  CRCエラーチェック
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_CRCERR) != 0){
			hspi->ErrorCode |= SPI_ERROR_CRC;
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_CRCERR);
		}
	}
	hspi->status = SPI_STATUS_READY;

	if(hspi->Init.semid != 0)
		isig_sem(hspi->Init.semid);
}

/*
 *  SPI DMA送受信コールバック関数
 */
static void
spi_dmatransrecv_func(DMA_Handle_t *hdma)
{
	SPI_Handle_t *hspi = (SPI_Handle_t *)hdma->localdata;
	volatile uint32_t tmp;

	/*
	 *  CRC演算
	 */
	if(hspi->Init.CRC == SPI_CRC_ENABLE){
		/*
		 *  受信終了待ち
		 */
		if(spi_waitflag(hspi, SPI_SR_RXNE, false, SPI_TIMEOUT_VALUE) != E_OK){
			hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
		}
		/*
		 *  CRC読み捨て
		 */
 		tmp = sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_DR));
		((void)(tmp));

		/*
		 *  CRCエラーチェック
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_CRCERR) != 0){
			hspi->ErrorCode |= SPI_ERROR_CRC;
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_CRCERR);
		}
	}

	/*
	 *  TXEフラグセット待ち
	 */
	if(spi_waitflag(hspi, SPI_SR_TXE, false, SPI_TIMEOUT_VALUE) != E_OK){
		hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
	}
	/*
	 *  TX DMA停止
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (SPI_CR2_TXDMAEN));

	/*
	 *  SPI BUSY終了待ち
	 */
	if(spi_waitflag(hspi, SPI_SR_BSY, true, SPI_TIMEOUT_VALUE) != E_OK){
		hspi->ErrorCode |= SPI_ERROR_TIMEOUT;
	}

	/*
	 *  RX DMA停止
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (SPI_CR2_RXDMAEN));

	hspi->TxXferCount = 0;
	hspi->RxXferCount = 0;
	hspi->status = SPI_STATUS_READY;

	if(hspi->Init.semid != 0)
		isig_sem(hspi->Init.semid);
}

/*
 *  SPI内部転送終了待ち
 */
ER
spi_inwait(SPI_Handle_t *hspi, uint32_t timeout)
{
	ER ercd = E_OK;
	int tick = timeout;

	while((hspi->status & SPI_STATUS_BUSY) != 0 && tick > 0){
		if(hspi->Init.semid != 0)
    		twai_sem(hspi->Init.semid, 1);
		else
			dly_tsk(1);
		tick--;
	}
	dma_end(hspi->hdmarx);
	dma_end(hspi->hdmatx);
	if(hspi->ErrorCode != 0)
		ercd = E_OBJ;
	else if(tick == 0)
		ercd = E_TMOUT;
	hspi->TxXferCount = 0;
	hspi->RxXferCount = 0;
	return ercd;
}

/*
 *  SPI DMAエラーコールバック関数
 */
static
void spi_dmaerror_func(DMA_Handle_t *hdma)
{
	SPI_Handle_t *hspi = (SPI_Handle_t *)hdma->localdata;

	hspi->TxXferCount = 0;
	hspi->RxXferCount = 0;
	hspi->status= SPI_STATUS_READY;
	hspi->ErrorCode |= SPI_ERROR_DMA;
	syslog_2(LOG_ERROR, "SPI DMA Error handle[%08x] ErrorCode[%08x] !", hdma, hdma->ErrorCode);
	sil_dly_nse(1000*500);	/* エラー時ハングアップ回避の0.5msec待ち */
}

/*
 *  SPI初期設定
 *  parameter1  port: SPIポート番号
 *  parameter2  spii: SPI初期設定構造体へのポインタ
 *  return SPIハンドラへのポインタ、NULLでエラー
 */
SPI_Handle_t *
spi_init(ID port, SPI_Init_t *spii)
{
	GPIO_Init_t GPIO_Init_Data;
	SPI_Handle_t *hspi;
	const SPI_PortControlBlock *spcb;
	DMA_Handle_t *hdma;
	uint32_t no;
	uint32_t frxth;
	volatile uint32_t tmpreg;

	if(port < SPI1_PORTID || port > NUM_SPIPORT)
		return NULL;
	if(spii == NULL)
		return NULL;

	no = INDEX_SPI(port);
	hspi = &SpiHandle[no];
	if(hspi->status != SPI_STATUS_RESET)
		return NULL;
	spcb = &spi_pcb[no];
	memcpy(&hspi->Init, spii, sizeof(SPI_Init_t));
	hspi->base = spcb->base;

	/*
	 *  SPI クロック設定
	 */
	sil_orw_mem((uint32_t *)spcb->gioclockbase, spcb->gioclockbit);
	tmpreg = sil_rew_mem((uint32_t *)spcb->gioclockbase);
	sil_orw_mem((uint32_t *)spcb->gioclockbase, spcb->gioclockbit2);
	tmpreg = sil_rew_mem((uint32_t *)spcb->gioclockbase);
	sil_orw_mem((uint32_t *)spcb->spiclockbase, spcb->spiclockbit);
	tmpreg = sil_rew_mem((uint32_t *)spcb->spiclockbase);
	sil_orw_mem((uint32_t *)spcb->dmaclockbase, spcb->dmaclockbit);
	tmpreg = sil_rew_mem((uint32_t *)spcb->dmaclockbase);
    ((void)(tmpreg));
  
	/*
	 *  SPI GPIOピン設定
	 */
	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = SPI_GPIO_PP;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
	GPIO_Init_Data.speed     = SPI_GPIO_SPEED;
	GPIO_Init_Data.alternate = spcb->apvalue;
	gpio_setup(spcb->giobase1, &GPIO_Init_Data, spcb->sckpin);
	gpio_setup(spcb->giobase2, &GPIO_Init_Data, spcb->misopin);
	gpio_setup(spcb->giobase3, &GPIO_Init_Data, spcb->mosipin);

	/*
	 *  SPI用受信DMP設定
	 */
	hdma = &hdma_tx[no];
	hdma->base                     = spcb->dmatxbase;
	hdma->Init.Channel             = spcb->dmatxchannel;
	hdma->Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma->Init.MemInc              = DMA_MINC_ENABLE;
	hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma->Init.Mode                = DMA_NORMAL;
	hdma->Init.Priority            = DMA_PRIORITY_LOW;
	hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma->Init.MemBurst            = DMA_MBURST_INC4;
	hdma->Init.PeriphBurst         = DMA_PBURST_INC4;

	dma_init(hdma);
	hspi->hdmatx = hdma;
	hdma->localdata                = hspi;

	/*
	 *  SPI用受信DMP設定
	 */
	hdma = &hdma_rx[no];
	hdma->base                     = spcb->dmarxbase;
	hdma->Init.Channel             = spcb->dmarxchannel;
	hdma->Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma->Init.MemInc              = DMA_MINC_ENABLE;
	hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma->Init.Mode                = DMA_NORMAL;
	hdma->Init.Priority            = DMA_PRIORITY_HIGH;
	hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma->Init.MemBurst            = DMA_MBURST_INC4;
	hdma->Init.PeriphBurst         = DMA_PBURST_INC4; 

	dma_init(hdma);
	hspi->hdmarx = hdma;
	hdma->localdata                = hspi;

	/*
	 *  SPIディゼーブル
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	/*
	 *  FIFOスレッシュホールド設定
	 */
	if(hspi->Init.DataSize > SPI_DATASIZE_8BIT)
		frxth = 0x00000000;
	else
		frxth = SPI_CR2_FRXTH;

	/*
	 *  CRC長の再設定
	 */
	if(hspi->Init.DataSize != SPI_DATASIZE_16BIT && hspi->Init.DataSize != SPI_DATASIZE_8BIT)
		hspi->Init.CRC = SPI_CRC_DISABLE;
	if(hspi->Init.CRCLength == SPI_CRC_LENGTH_DATASIZE){
		if(hspi->Init.DataSize > SPI_DATASIZE_8BIT)
			hspi->Init.CRCLength = SPI_CRC_LENGTH_16BIT;
		else
			hspi->Init.CRCLength = SPI_CRC_LENGTH_8BIT;
	}

	/*
	 *  SPI CR1/CR2レジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), (hspi->Init.Mode | hspi->Init.Direction |
							hspi->Init.CLKPolarity | hspi->Init.CLKPhase | (hspi->Init.NSS & SPI_CR1_SSM) |
							hspi->Init.Prescaler | hspi->Init.SignBit | hspi->Init.CRC));

	/*
	 *  SPI NSS設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (((hspi->Init.NSS >> 16) & SPI_CR2_SSOE) | hspi->Init.TIMode |
							hspi->Init.DataSize | frxth));

	/*
	 *  SPI CRC多項式設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CRCPR), hspi->Init.CRCPolynomial);

	/*
	 *  SPI I2CSモードをリセット
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_I2SCFGR), SPI_I2SCFGR_I2SMOD);

	hspi->ErrorCode = SPI_ERROR_NONE;
	hspi->status = SPI_STATUS_READY;
	return hspi;
}

/*
 *  SPI終了設定
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  return ERコード
 */
ER
spi_deinit(SPI_Handle_t *hspi)
{
	if(hspi == NULL)
		return E_PAR;

	dma_deinit(hspi->hdmatx);
	dma_deinit(hspi->hdmarx);

	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	hspi->ErrorCode = SPI_ERROR_NONE;
	hspi->status = SPI_STATUS_RESET;
	return E_OK;
}

/*
 *  SPIモジュールのリセット
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  return ERコード
 */
ER
spi_reset(SPI_Handle_t *hspi)
{
	uint32_t frxth;

	if(hspi == NULL)
		return E_PAR;
	/*
	 *  SPIディゼーブル
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	/*
	 *  FIFOスレッシュホールド設定
	 */
	if(hspi->Init.DataSize > SPI_DATASIZE_8BIT)
		frxth = 0x00000000;
	else
		frxth = SPI_CR2_FRXTH;

	/*
	 *  CRC長の再設定
	 */
	if(hspi->Init.DataSize != SPI_DATASIZE_16BIT && hspi->Init.DataSize != SPI_DATASIZE_8BIT)
		hspi->Init.CRC = SPI_CRC_DISABLE;
	if(hspi->Init.CRCLength == SPI_CRC_LENGTH_DATASIZE){
		if(hspi->Init.DataSize > SPI_DATASIZE_8BIT)
			hspi->Init.CRCLength = SPI_CRC_LENGTH_16BIT;
		else
			hspi->Init.CRCLength = SPI_CRC_LENGTH_8BIT;
	}

	/*
	 *  SPI CR1/CR2レジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), (hspi->Init.Mode | hspi->Init.Direction |
							hspi->Init.CLKPolarity | hspi->Init.CLKPhase | (hspi->Init.NSS & SPI_CR1_SSM) |
							hspi->Init.Prescaler | hspi->Init.SignBit | hspi->Init.CRC));

	/*
	 *  SPI NSS設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (((hspi->Init.NSS >> 16) & SPI_CR2_SSOE) | hspi->Init.TIMode |
							hspi->Init.DataSize | frxth));

	/*
	 *  SPI CRC多項式設定
	 */
	sil_wrw_mem((uint32_t *)(hspi->base+TOFF_SPI_CRCPR), hspi->Init.CRCPolynomial);

	/*
	 *  SPI I2CSモードをリセット
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_I2SCFGR), SPI_I2SCFGR_I2SMOD);

	hspi->ErrorCode = SPI_ERROR_NONE;
	hspi->status = SPI_STATUS_READY;
	return E_OK;
}

/*
 *  SPI送信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  pdata: 送信バッファへのポインタ(4バイトアライン)
 *  parameter3  length: 送信サイズ(CRCでは+1)
 *  return ERコード
 */
ER
spi_transmit(SPI_Handle_t *hspi, uint8_t *pdata, uint16_t length)
{
	ER ercd = E_OK;

	if(hspi == NULL || pdata == NULL || length == 0)
		return E_PAR;
	if((((uint32_t)pdata) & 3) != 0)
		return E_PAR;

	if(hspi->Init.semlock != 0)
		wai_sem(hspi->Init.semlock);
	if(hspi->status != SPI_STATUS_READY){
		if(hspi->Init.semlock != 0)
			sig_sem(hspi->Init.semlock);
		return E_OBJ;
	}

	/* Configure communication */
	hspi->xmode       = SPI_XMODE_TX;
	hspi->status      = SPI_STATUS_BUSY;
	hspi->ErrorCode   = SPI_ERROR_NONE;

	hspi->pTxBuffPtr  = pdata;
	hspi->TxXferSize  = length;
	hspi->TxXferCount = length;
	hspi->RxXferSize   = 0;
	hspi->RxXferCount  = 0;

    /*
	 *  1ライン通信設定
	 */
	if(hspi->Init.Direction == SPI_DIRECTION_1LINE){
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_BIDIOE);
	}

	/*
	 *  CRCモードなら設定を有効に
	 */
	if(hspi->Init.CRC == SPI_CRC_ENABLE){
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
	}

	/*
	 *  送信DMAコントローラ設定
	 */
	hspi->hdmatx->xferhalfcallback = NULL;
	hspi->hdmatx->xfercallback = spi_dmatransmit_func;
	hspi->hdmatx->errorcallback = spi_dmaerror_func;

	/*
	 *  LAST DMA設定
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
	if(hspi->Init.DataSize <= SPI_DATASIZE_8BIT && hspi->hdmatx->Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD){
	    /* Check the even/odd of the data size + crc if enabled */
		if((hspi->TxXferCount & 0x1) == 0){
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
			hspi->TxXferCount = (hspi->TxXferCount >> 1);
		}
		else{
			sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
			hspi->TxXferCount = (hspi->TxXferCount >> 1) + 1;
		}
	}

	/*
	 *  送信DMAコントローラ設定
	 */
	dma_start(hspi->hdmatx, (uint32_t)hspi->pTxBuffPtr, (uint32_t)(hspi->base+TOFF_SPI_DR), hspi->TxXferCount);

	/*  SPI有効化 */
	if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1)) & SPI_CR1_SPE) == 0)
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	/*
	 *  SPI送信DMA設定
	 */
	sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_TXDMAEN);

#if SPI_WAIT_TIME != 0
	ercd = spi_inwait(hspi, SPI_WAIT_TIME * length);

	if(hspi->Init.semlock != 0)
		sig_sem(hspi->Init.semlock);
#endif
	return ercd;
}

/*
 *  SPI受信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  pdata: 受信バッファへのポインタ(4バイトアライン)
 *  parameter3  length: 送信サイズ(CRCでは+1)
 *  return ERコード
 */
ER
spi_receive(SPI_Handle_t *hspi, uint8_t *pdata, uint16_t length)
{
	ER ercd = E_OK;

	if(hspi == NULL || pdata == NULL || length == 0)
		return E_PAR;
	if((((uint32_t)pdata) & 3) != 0)
		return E_PAR;

	if(hspi->Init.semlock != 0)
		wai_sem(hspi->Init.semlock);
	if(hspi->status != SPI_STATUS_READY){
		if(hspi->Init.semlock != 0)
			sig_sem(hspi->Init.semlock);
		return E_OBJ;
	}

    /*
	 *  転送情報を設定
	 */
	hspi->xmode       = SPI_XMODE_RX;
    hspi->status      = SPI_STATUS_BUSY;
    hspi->ErrorCode   = SPI_ERROR_NONE;
    hspi->pRxBuffPtr  = pdata;
    hspi->RxXferSize  = length;
    hspi->RxXferCount = length;
    hspi->TxXferSize   = 0;
    hspi->TxXferCount  = 0;

    /*
	 *  1ライン通信設定
	 */
    if(hspi->Init.Direction == SPI_DIRECTION_1LINE){
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_BIDIOE);
    }
    else if(hspi->Init.Direction == SPI_DIRECTION_2LINES){
	    hspi->status = SPI_STATUS_READY;
		if(hspi->Init.semlock != 0)
			sig_sem(hspi->Init.semlock);
		if(hspi->Init.Mode == SPI_MODE_MASTER){
			/*
			 *  送受信を行う
			 */
			memset(pdata, 0xff, length);
			return spi_transrecv(hspi, pdata, pdata, length);
		}
		else
			return E_PAR;
    }

	/*
	 *  CRCモードなら設定を有効に
	 */
    if(hspi->Init.CRC == SPI_CRC_ENABLE){
      sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
      sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
    }

	/*
	 *  スレッシュホールド設定
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMARX);
	if( hspi->Init.DataSize > SPI_DATASIZE_8BIT){
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_FRXTH);
	}
	else{
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_FRXTH);
	}

    /*
	 *  DMA受信コールバック関数設定
	 */
	if((hspi->Init.Mode == SPI_MODE_MASTER)
		 && (hspi->Init.Direction == SPI_DIRECTION_1LINE || hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)){
	    hspi->hdmarx->xfercallback = spi_dmareceive_func;
	}
	else{
		hspi->hdmarx->xfercallback = spi_dmatransrecv_func;
	}
    hspi->hdmarx->xferhalfcallback = NULL;
    hspi->hdmarx->errorcallback = spi_dmaerror_func;

	/*
	 *  受信DMAコントローラ設定
	 */
    dma_start(hspi->hdmarx, (uint32_t)(hspi->base+TOFF_SPI_DR), (uint32_t)hspi->pRxBuffPtr, hspi->RxXferCount);

	/*  SPI有効化 */
	if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1)) & SPI_CR1_SPE) == 0)
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	/*
	 *  SPI受信DMA設定
	 */
	sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), (SPI_CR2_RXDMAEN));

#if SPI_WAIT_TIME != 0
	ercd = spi_inwait(hspi, SPI_WAIT_TIME * length);

	if(hspi->Init.semlock != 0)
		sig_sem(hspi->Init.semlock);
#endif
	return ercd;
}

/*
 *  SPI送受信実行関数
 *  parameter1  hspi: SPIハンドラへのポインタ
 *  parameter2  ptxdata: 送信バッファへのポインタ(4バイトアライン)
 *  parameter3  prxdata: 受信バッファへのポインタ(4バイトアライン)
 *  parameter4  length: 送信サイズ(CRCでは+1)
 *  return ERコード
 */
ER
spi_transrecv(SPI_Handle_t *hspi, uint8_t *ptxdata, uint8_t *prxdata, uint16_t length)
{
	ER ercd = E_OK;

	if(hspi == NULL || prxdata == NULL || length == 0)
		return E_PAR;
	if((((uint32_t)ptxdata) & 3) != 0 || (((uint32_t)prxdata) & 3) != 0)
		return E_PAR;
	if(hspi->Init.Direction != SPI_DIRECTION_2LINES && hspi->Init.Mode != SPI_MODE_MASTER)
		return E_PAR;

	if(hspi->Init.semlock != 0)
		wai_sem(hspi->Init.semlock);
	if(hspi->status != SPI_STATUS_READY){
		if(hspi->Init.semlock != 0)
			sig_sem(hspi->Init.semlock);
		return E_OBJ;
	}

	hspi->xmode = SPI_XMODE_TXRX;
	hspi->status = SPI_STATUS_BUSY;

	/*
	 *  転送情報を設定
	 */
	hspi->ErrorCode   = SPI_ERROR_NONE;
	hspi->pTxBuffPtr  = (uint8_t*)ptxdata;
	hspi->TxXferSize  = length;
	hspi->TxXferCount = length;
	hspi->pRxBuffPtr  = (uint8_t*)prxdata;
	hspi->RxXferSize  = length;
	hspi->RxXferCount = length;

	/*
	 *  CRCモードなら設定を有効に
	 */
	if(hspi->Init.CRC == SPI_CRC_ENABLE){
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_CRCEN);
	}

    /*
	 *  LAST DMAとスレッシュホールド設定
	 */
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
	sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMARX);
	if(hspi->Init.DataSize > SPI_DATASIZE_8BIT){
		sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_FRXTH);
	}
	else{
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_FRXTH);
		if(hspi->hdmatx->Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD){
	        if((hspi->TxXferSize & 0x1) == 0x0 ){
				sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
				hspi->TxXferCount = hspi->TxXferCount >> 1;
			}
			else{
				sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMATX);
				hspi->TxXferCount = (hspi->TxXferCount >> 1) + 1;
			}
		}
		if(hspi->hdmarx->Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD){
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_FRXTH);
			if((hspi->RxXferCount & 0x1) == 0){
				sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMARX);
				hspi->RxXferCount = hspi->RxXferCount >> 1;
			}
			else{
				sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_LDMARX);
				hspi->RxXferCount = (hspi->RxXferCount >> 1) + 1;
			}
		}
	}

	/*
	 *  DMA受信コールバック関数設定
	 */
	if((hspi->Init.Mode == SPI_MODE_MASTER)
		 && (hspi->Init.Direction == SPI_DIRECTION_1LINE || hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)){
		hspi->hdmarx->xfercallback = spi_dmareceive_func;
	}
	else{
		hspi->hdmarx->xfercallback = spi_dmatransrecv_func;
	}
	hspi->hdmarx->xferhalfcallback = NULL;
	hspi->hdmarx->errorcallback = spi_dmaerror_func;

	/*
	 *  受信DMAコントローラ設定
	 */
	dma_start(hspi->hdmarx, (uint32_t)(hspi->base+TOFF_SPI_DR), (uint32_t)hspi->pRxBuffPtr, hspi->RxXferCount);

	/*
	 *  SPI受信DMA設定
	 */
	sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_RXDMAEN);

	/*
	 *  DMA送信コールバックの設定
	 */
	hspi->hdmatx->xfercallback = NULL;
	hspi->hdmatx->errorcallback = spi_dmaerror_func;

	/*
	 *  送信DMAコントローラ設定
	 */
	dma_start(hspi->hdmatx, (uint32_t)hspi->pTxBuffPtr, (uint32_t)(hspi->base+TOFF_SPI_DR), hspi->TxXferCount);

	/*
	 *  SPIエラー割込み設定
	 */
	sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_ERRIE);

	/*  SPI有効化 */
	if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1)) & SPI_CR1_SPE) == 0)
		sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);

	/*
	 *  SPI送信DMA設定
	 */
	sil_orw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2), SPI_CR2_TXDMAEN);

#if SPI_WAIT_TIME != 0
	ercd = spi_inwait(hspi, SPI_WAIT_TIME * length);

	if(hspi->Init.semlock != 0)
		sig_sem(hspi->Init.semlock);
#endif
	return ercd;
}

/*
 *  SPI転送終了待ち
 */
ER
spi_wait(SPI_Handle_t *hspi, uint32_t timeout)
{
	ER ercd = E_OK;

#if SPI_WAIT_TIME == 0
	if(hspi == NULL)
		return E_PAR;
	ercd = spi_inwait(hspi, timeout);
	if(hspi->Init.semlock != 0)
		sig_sem(hspi->Init.semlock);
#endif
	return ercd;
}

/*
 *  SPI割込みサービスルーチン
 */
void
spi_handler(SPI_Handle_t *hspi)
{
	volatile uint32_t tmp3 = 0;

	/*
	 *  割込みエラー判定
	 */
	if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_CR2)) & SPI_CR2_ERRIE) != 0){
		/*
		 *  SPI CRCエラー判定
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_CRCERR) != 0){
			hspi->ErrorCode |= SPI_ERROR_CRC;
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_CRCERR);
		}
		/*
		 *  SPI モードフォルト判定
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_MODF) != 0){
			hspi->ErrorCode |= SPI_ERROR_MODF;
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_MODF);
            sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);
		}
		/*
		 *  SPI オーバーラン判定
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_OVR) != 0){
			if(hspi->xmode != SPI_XMODE_TX){
				hspi->ErrorCode |= SPI_ERROR_OVR;
				tmp3 = sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_DR));
				((void)(tmp3));
				sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_OVR);
			}
		}
		/*
		 *  SPI フレームエラー判定
		 */
		if((sil_rew_mem((uint32_t *)(hspi->base+TOFF_SPI_SR)) & SPI_SR_FRE) != 0){
			hspi->ErrorCode |= SPI_ERROR_FRE;
			sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_SR), SPI_SR_FRE);
		}
		/*
		 *  エラー発生時はレディに戻す
		 */
		if(hspi->ErrorCode != SPI_ERROR_NONE){
            sil_andw_mem((uint32_t *)(hspi->base+TOFF_SPI_CR1), SPI_CR1_SPE);
			hspi->status = SPI_STATUS_READY;
			syslog_2(LOG_ERROR, "spi hanndler error[%08x] ErrorCode[%08x] !", hspi, hspi->ErrorCode);
		}
	}
}


/*
 *  SPI割込みサービスルーチン
 */
void spi_isr(intptr_t exinf)
{
  spi_handler(&SpiHandle[INDEX_SPI((uint32_t)exinf)]);
}

