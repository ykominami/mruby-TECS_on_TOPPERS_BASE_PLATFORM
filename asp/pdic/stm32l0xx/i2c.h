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
 *  @(#) $Id: i2c.h 698 2017-07-19 21:35:38Z roi $
 */
/*
 * 
 *  STM32L0xx I2Cデバイスドライバの外部宣言
 *
 */

#ifndef _I2C_H_
#define _I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif


/*
 *  I2Cポート定義
 */
#define I2C1_PORTID             1
#define I2C2_PORTID             2
#define I2C3_PORTID             3
#define NUM_I2CPORT             3

/*
 *  I2Cアドレッシングモード
 */
#define I2C_ADDRESSINGMODE_7BIT  I2C_OAR1_OA1EN
#define I2C_ADDRESSINGMODE_10BIT (I2C_OAR1_OA1MODE | I2C_OAR1_OA1EN)

/*
 *  I2Cデュアルアドレッシングモード
 */
#define I2C_DUALADDRESS_DISABLE 0x00000000
#define I2C_DUALADDRESS_ENABLE  I2C_OAR1_OA1MODE

/*
 *  I2Cアドレス２マスク
 */
#define I2C_OA2_NOMASK          I2C_OAR2_OA2NOMASK
#define I2C_OA2_MASK01          I2C_OAR2_OA2MASK01
#define I2C_OA2_MASK02          I2C_OAR2_OA2MASK02
#define I2C_OA2_MASK03          I2C_OAR2_OA2MASK03
#define I2C_OA2_MASK04          I2C_OAR2_OA2MASK04
#define I2C_OA2_MASK05          I2C_OAR2_OA2MASK05
#define I2C_OA2_MASK06          I2C_OAR2_OA2MASK06
#define I2C_OA2_MASK07          I2C_OAR2_OA2MASK07

/*
 *  I2Cジェネラルコールアドレッシングモード
 */
#define I2C_GENERALCALL_DISABLE 0x00000000
#define I2C_GENERALCALL_ENABLE  I2C_CR1_GCEN

/*
 *  I2Cノストラッチモード
 */
#define I2C_NOSTRETCH_DISABLE   0x00000000
#define I2C_NOSTRETCH_ENABLE    I2C_CR1_NOSTRETCH

/*
 *  I2Cメモリアドレスモード
 */
#define I2C_MEMADD_SIZE_NONE    0x00000000
#define I2C_MEMADD_SIZE_8BIT    0x00000001
#define I2C_MEMADD_SIZE_16BIT   0x00000002


/*
 *  SPIハードウェア設定構造体
 */

typedef struct _I2C_PortControlBlock{
	uint32_t              base;
	uint32_t              gioclockbase;
	uint32_t              gioclockbit1;
	uint32_t              gioclockbit2;
	uint32_t              i2cclockbase;
	uint32_t              i2cclockbit;
	uint32_t              i2cresetbase;
	uint32_t              i2cresetbit;

	uint32_t              giobase1;
	uint32_t              giobase2;
	uint8_t               sclpin;
	uint8_t               sclaf;
	uint8_t               sdapin;
	uint8_t               sdaaf;
} I2C_PortControlBlock;

/*
 *  I2Cコンフュギュレーション構造体定義
 */
typedef struct
{
	uint32_t              Timing;			/* I2C タイミング設定 */
	uint32_t              OwnAddress1;		/* I2C 最初のデバイスの自分のアドレス */
	uint32_t              AddressingMode;	/* I2C アドレッシングモード */
	uint32_t              DualAddressMode;	/* I2C アドレスデュアルモード */
	uint32_t              OwnAddress2Masks;	/* I2C アドレス２マスク */
	uint32_t              OwnAddress2;		/* I2C ２つ目のデバイスの自分のアドレス */
	uint32_t              GeneralCallMode;	/* I2C ジェネラル・コール・モード */
	uint32_t              NoStretchMode;	/* I2C ノンストレッジモード */
	int                   semid;			/* I2C 通信用セマフォ値 */
	int                   semlock;			/* I2C ロックセマフォ値 */
}I2C_Init_t;

/*
 *  I2C状態定義
 */
#define I2C_STATUS_RESET    0x00			/* I2C リセット状態 */
#define I2C_STATUS_READY    0x01			/* I2C レディ状態 */
#define I2C_STATUS_BUSY     0x02			/* I2C ビジィ状態 */
#define I2C_STATUS_BUSY_TX  0x52			/* I2C 送信中 */
#define I2C_STATUS_BUSY_RX  0x62			/* I2C 受信中 */
#define I2C_STATUS_LISTEN   0x04			/*!< Address Listen Mode is ongoing            */
#define I2C_STATUS_BUSY_TX_LISTEN 0x54		/*!< Address Listen Mode and Data Transmission
                                                 process is ongoing                         */
#define I2C_STATUS_BUSY_RX_LISTEN 0x64		/*!< Address Listen Mode and Data Reception */
#define I2C_STATUS_TIMEOUT  0x80

/*
 *  I2Cエラー定義
 */
#define I2C_ERROR_NONE      0x00000000		/* No error              */
#define I2C_ERROR_BERR      0x00000001		/* BERR error            */
#define I2C_ERROR_ARLO      0x00000002		/* ARLO error            */
#define I2C_ERROR_AF        0x00000004		/* ACKF error            */
#define I2C_ERROR_OVR       0x00000008		/* OVR error             */
#define I2C_ERROR_DMA       0x00000010		/* DMA transfer error    */
#define I2C_ERROR_TIMEOUT   0x00000020		/* Timeout error         */
#define I2C_ERROR_SIZE      0x00000040		/* Size Management error */

/*
 *  I2Cハンドラ定義
 */
typedef struct __I2C_Handle_t I2C_Handle_t;
struct __I2C_Handle_t {
	uint32_t                   base;
	I2C_Init_t                 Init;		/* I2C初期設定パラメータ */
	uint8_t                    *pBuffPtr;	/* I2C 通信バッファへのポインタ */
	uint32_t                   XferOptions;	/* I2C 通信オプション */
	uint32_t                   XferSize;	/* I2C 通信サイズ */
	volatile uint16_t          XferCount;	/* I2C 通信カウンタ */
	volatile uint16_t          XferCount2;	/* I2C 通信カウンタ(RELOARD) */
	volatile uint32_t          AddrEventCount; /*!< I2C Address Event counter                 */
	void                  (*writecallback)(I2C_Handle_t * hi2c);	/* 送信終了コールバック関数 */
	void                  (*readcallback)(I2C_Handle_t * hi2c);		/* 受信終了コールバック関数 */
	void                  (*listencallback)(I2C_Handle_t * hi2c);	/* LISTEN終了コールバック関数 */
	void                  (*addrcallback)(I2C_Handle_t * hi2c, uint8_t direction, uint16_t addrmatch);
	void                  (*errorcallback)(I2C_Handle_t * hi2c);	/* エラーコールバック関数 */
	ID                         i2cid;		/* I2C ID値 */
	volatile uint32_t          status;		/* I2C 実行状態 */
	volatile uint32_t          ErrorCode;	/* I2C Error code */
};

extern I2C_Handle_t *i2c_init(ID port, I2C_Init_t *ii2c);
extern ER i2c_deinit (I2C_Handle_t *hi2c);
extern ER i2c_slavewrite(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size);
extern ER i2c_slaveread(I2C_Handle_t *hi2c, uint8_t *pData, uint16_t Size);
extern ER i2c_memwrite(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
extern ER i2c_memread(I2C_Handle_t *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
extern ER i2c_ready(I2C_Handle_t *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
extern void i2c_ev_handler(I2C_Handle_t *hi2c);
extern void i2c_er_handler(I2C_Handle_t *hi2c);
extern void i2c_isr(intptr_t exinf);


#ifdef __cplusplus
}
#endif

#endif	/* _I2C_H_ */

