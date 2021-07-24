/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
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
 *  @(#) $Id: spi.h 698 2016-02-01 11:00:32Z roi $
 */
/*
 * 
 *  STM32F4XX SPIデバイスドライバの外部宣言
 *
 */

#ifndef _SPI_H_
#define _SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  SPIポート定義
 */
#define SPI1_PORTID             1
#define SPI2_PORTID             2
#define NUM_SPIPORT             2

/*
 *  SPI状態定義
 */
#define SPI_STATUS_RESET        0x00	/* SPI未使用状態 */
#define SPI_STATUS_READY        0x01	/* SPIレディ状態 */
#define SPI_STATUS_ERROR        0x02	/* SPIエラー状態 */
#define SPI_STATUS_BUSY         0x04	/* SPI処理中 */

/*
 *  SPIエラー定義
 */ 
#define SPI_ERROR_NONE          0x00000000	/* No error */
#define SPI_ERROR_MODF          0x00000001	/* MODF error */
#define SPI_ERROR_CRC           0x00000002	/* CRC error */
#define SPI_ERROR_OVR           0x00000004	/* OVR error */
#define SPI_ERROR_FRE           0x00000008	/* FRE error */
#define SPI_ERROR_DMA           0x00000010	/* DMA transfer error */
#define SPI_ERROR_TIMEOUT       0x00000020

/*
 *  SPI転送モード
 */
#define SPI_XMODE_TX            0x00000000	/* 送信モード */
#define SPI_XMODE_RX            0x00000001	/* 受信モード */
#define SPI_XMODE_TXRX          0x00000002	/* 送受信モード */

/*
 *  SPIモード定義
 */
#define SPI_MODE_SLAVE          0x00000000
#define SPI_MODE_MASTER         (SPI_CR1_MSTR | SPI_CR1_SSI)


/*
 *  SPIデータサイズ定義
 */
#define SPI_DATASIZE_4BIT       0x00000300
#define SPI_DATASIZE_5BIT       0x00000400
#define SPI_DATASIZE_6BIT       0x00000500
#define SPI_DATASIZE_7BIT       0x00000600
#define SPI_DATASIZE_8BIT       0x00000700
#define SPI_DATASIZE_9BIT       0x00000800
#define SPI_DATASIZE_10BIT      0x00000900
#define SPI_DATASIZE_11BIT      0x00000A00
#define SPI_DATASIZE_12BIT      0x00000B00
#define SPI_DATASIZE_13BIT      0x00000C00
#define SPI_DATASIZE_14BIT      0x00000D00
#define SPI_DATASIZE_15BIT      0x00000E00
#define SPI_DATASIZE_16BIT      0x00000F00

/*
 *  SPI転送クロック極性定義
 */
#define SPI_POLARITY_LOW        0x00000000
#define SPI_POLARITY_HIGH       SPI_CR1_CPOL

/*
 *  SPIクロック位相定義
 */
#define SPI_PHASE_1EDGE         0x00000000
#define SPI_PHASE_2EDGE         SPI_CR1_CPHA

/*
 *  SPI NSS定義
 */
#define SPI_NSS_SOFT            SPI_CR1_SSM
#define SPI_NSS_HARD_INPUT      0x00000000
#define SPI_NSS_HARD_OUTPUT     0x00040000

/*
 *  SPI MSB/LSB定義
 */
#define SPI_DATA_MSB            0x00000000
#define SPI_DATA_LSB            SPI_CR1_LSBFIRST

/*
 *  SPIプリスケーラ設定
 */
#define SPI_BAUDRATEPRESCALER_2   0x00000000
#define SPI_BAUDRATEPRESCALER_4   0x00000008
#define SPI_BAUDRATEPRESCALER_8   0x00000010
#define SPI_BAUDRATEPRESCALER_16  0x00000018
#define SPI_BAUDRATEPRESCALER_32  0x00000020
#define SPI_BAUDRATEPRESCALER_64  0x00000028
#define SPI_BAUDRATEPRESCALER_128 0x00000030
#define SPI_BAUDRATEPRESCALER_256 0x00000038

/*
 *  CRC長定義
 */
#define SPI_CRC_LENGTH_DATASIZE   0x00000000
#define SPI_CRC_LENGTH_8BIT       0x00000001
#define SPI_CRC_LENGTH_16BIT      0x00000002

/*
 *  SPI TIモード設定定義
 */
#define SPI_TIMODE_DISABLE      0x00000000
#define SPI_TIMODE_ENABLE       SPI_CR2_FRF

/*
 *  SPIライン設定
 */
#define SPI_DIRECTION_2LINES             ((uint32_t)0x00000000)
#define SPI_DIRECTION_2LINES_RXONLY      SPI_CR1_RXONLY
#define SPI_DIRECTION_1LINE              SPI_CR1_BIDIMODE

/*
 *  SPI CRC設定定義
 */
#define SPI_CRC_DISABLE         0x00000000
#define SPI_CRC_ENABLE          SPI_CR1_CRCEN

/*
 *  SPIハードウェア設定構造体
 */

typedef struct _SPI_PortControlBlock{
	uint32_t              base;
	uint32_t              gioclockbase;
	uint32_t              gioclockbit;
	uint32_t              gioclockbit2;
	uint32_t              spiclockbase;
	uint32_t              spiclockbit;
	uint32_t              dmaclockbase;
	uint32_t              dmaclockbit;

	uint32_t              giobase1;
	uint32_t              giobase2;
	uint32_t              giobase3;
	uint8_t               sckpin;
	uint8_t               misopin;
	uint8_t               mosipin;
	uint8_t               apvalue;
	uint32_t              dmatxchannel;
	uint32_t              dmatxbase;
	uint32_t              dmarxchannel;
	uint32_t              dmarxbase;
} SPI_PortControlBlock;

/*
 *  SPI 設定初期設定構造体
 */
typedef struct
{
	uint32_t              Mode;				/* SPIマスタースレーブ設定 */
	uint32_t              Direction;		/* SPI転送方向 */
    uint32_t              DataSize;			/* SPI転送データサイズ */
	uint32_t              CLKPolarity;		/* SPI転送クロックの極性 */
	uint32_t              CLKPhase;			/* SPIクロック位相 */
	uint32_t              NSS;				/* SPI NSS */
	uint32_t              Prescaler;		/* SPIクロック分周設定 */
	uint32_t              SignBit;			/* SPI MSB/LSB設定 */
	uint32_t              TIMode;			/* SPI TIモード */
	uint32_t              CRC;				/* SPI CRC演算設定 */
	uint32_t              CRCPolynomial;	/* SPI CRC多項式設定 */
	uint32_t              CRCLength;		/* SPI CRC長 */
	int                   semid;			/* SPI 通信用セマフォ値 */
	int                   semlock;			/* SPI ロックセマフォ値 */
}SPI_Init_t;

/*
 *  SPIハンドラ
 */
typedef struct _SPI_Handle_t
{
	uint32_t              base;				/* SPI registers base address */
	SPI_Init_t            Init;				/* SPI communication parameters */
	uint8_t               *pTxBuffPtr;		/* Pointer to SPI Tx transfer Buffer */
	uint16_t              TxXferSize;		/* SPI Tx transfer size */
	uint16_t              TxXferCount;		/* SPI Tx Transfer Counter */
	uint8_t               *pRxBuffPtr;		/* Pointer to SPI Rx transfer Buffer */
	uint16_t              RxXferSize;		/* SPI Rx transfer size */
	uint16_t              RxXferCount;		/* SPI Rx Transfer Counter */
	DMA_Handle_t          *hdmatx;			/* SPI Tx DMA handle parameters */
	DMA_Handle_t          *hdmarx;			/* SPI Rx DMA handle parameters */
	uint32_t              xmode;
	volatile uint32_t     status;			/* SPI communication state */
	volatile uint32_t     ErrorCode;		/* SPI Error code */
}SPI_Handle_t;



extern SPI_Handle_t *spi_init(ID port, SPI_Init_t *spii);
extern ER spi_deinit(SPI_Handle_t *hspi);
extern ER spi_reset(SPI_Handle_t *hspi);
extern ER spi_transmit(SPI_Handle_t *hspi, uint8_t *pdata, uint16_t length);
extern ER spi_receive(SPI_Handle_t *hspi, uint8_t *pdata, uint16_t length);
extern ER spi_transrecv(SPI_Handle_t *hspi, uint8_t *ptxData, uint8_t *prxData, uint16_t length);
extern ER spi_wait(SPI_Handle_t *hspi, uint32_t timeout);
extern void spi_handler(SPI_Handle_t *hspi);
extern void spi_isr(intptr_t exinf);

#ifdef __cplusplus
}
#endif

#endif	/* _SPI_H_ */

