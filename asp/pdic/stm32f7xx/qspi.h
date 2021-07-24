/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  @(#) $Id: qspi.h 698 2017-08-16 12:24:03Z roi $
 */
/*
 * 
 *  QSPIデバイスドライバの外部宣言
 *
 */

#ifndef _QSPI_H_
#define _QSPI_H_

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

/*
 *  QSPIポート定義
 */
#define QSPI1_PORTID            1
#define NUM_QSPIPORT            1

/*
 *  フラッシュ・コマンド
 */
#define QSPI_STIG_OPCODE_WR_STAT_REG          (0x01)
#define QSPI_STIG_OPCODE_PP                   (0x02)
#define QSPI_STIG_OPCODE_READ                 (0x03)
#define QSPI_STIG_OPCODE_WRDIS                (0x04)
#define QSPI_STIG_OPCODE_RD_STAT_REG          (0x05)
#define QSPI_STIG_OPCODE_WREN                 (0x06)
#define QSPI_STIG_OPCODE_FASTREAD             (0x0B)
#define QSPI_DEVSZ_BYTESPERSUBSECTOR_RESET    (0x10)
#define QSPI_STIG_OPCODE_4BYTE_WRITE          (0x12)
#define QSPI_STIG_OPCODE_4BYTE_READ           (0x13)
#define QSPI_STIG_OPCODE_READ_CFG_REG         (0x15)
#define QSPI_STIG_OPCODE_SUBSEC_ERASE         (0x20)
#define QSPI_STIG_OPCODE_4BYTE_SEC_ERASE      (0x21)
#define QSPI_STIG_OPCODE_QUAD_PP              (0x32)
#define QSPI_STIG_OPCODE_4QUAD_WRITE          (0x34)
#define QSPI_STIG_OPCODE_ENTER_QUAD_CMD       (0x35)
#define QSPI_STIG_OPCODE_FASTREAD_DUAL_OUTPUT (0x3B)
#define QSPI_STIG_OPCODE_4BYTE_BLOCK_ERASE    (0x5C)
#define QSPI_STIG_OPCODE_CHIP_ERASE           (0x60)
#define QSPI_STIG_OPCODE_FASTREAD_QUAD_OUTPUT (0x6B)
#define QSPI_STIG_OPCODE_WRITE_VOL_CFG_REG_CMD (0x81)
#define QSPI_STIG_OPCODE_READ_VOL_CFG_REG_CMD (0x85)
#define QSPI_STIG_OPCODE_RDID                 (0x9F)
#define QSPI_STIG_OPCODE_DUAL_PP              (0xA2)
#define QSPI_STIG_OPCODE_ENTER_4BYTE_MODE     (0xB7)
#define QSPI_STIG_OPCODE_FASTREAD_DUAL_IO     (0xBB)
#define QSPI_STIG_OPCODE_DIE_ERASE            (0xC4)
#define QSPI_STIG_OPCODE_WR_EXT_REG           (0xC5)
#define QSPI_STIG_OPCODE_BULK_ERASE           (0xC7)
#define QSPI_STIG_OPCODE_RD_EXT_REG           (0xC8)
#define QSPI_STIG_OPCODE_SEC_ERASE            (0xD8)
#define QSPI_STIG_OPCODE_4BYTE_Q_SEC_ERASE    (0xDC)
#define QSPI_STIG_OPCODE_EXIT_4BYTE_MODE      (0xE9)
#define QSPI_STIG_OPCODE_FASTREAD_QUAD_IO     (0xEB)
#define QSPI_STIG_OPCODE_4QUAD_READ           (0xEC)

/*
 *  フラッシュコマンド Micron依存
 */
#define QSPI_STIG_OPCODE_N25Q512A_CLRFLGSR    (0x50)
#define QSPI_STIG_OPCODE_N25Q512A_DISCVR_PARAM  (0x5A)
#define QSPI_STIG_OPCODE_N25Q128              (0x61)
#define QSPI_STIG_REGISTER_N25Q128_VECR       (0x65)
#define QSPI_STIG_OPCODE_N25Q512A_RESET_EN    (0x66)
#define QSPI_STIG_OPCODE_N25Q512A_RDFLGSR     (0x70)
#define QSPI_STIG_OPCODE_N25Q512A_RESET_MEM   (0x99)

/*
 *  マニュファクチャID
 */
#define QSPI_STIG_RDID_JEDECID_MICRON         (0x20)
#define QSPI_STIG_RDID_JEDECID_SPANSION       (0x01)
#define QSPI_STIG_RDID_JEDECID_MACRONIX       (0xC2)


#define QSPI_MAX_ERASE_CMDS  	2


/*
 *  QSPI Sample Shifting
 */
#define QSPI_SAMPLE_SHIFTING_NONE       0x00000000				/* No clock cycle shift to sample data */
#define QSPI_SAMPLE_SHIFTING_HALFCYCLE  QUADSPI_CR_SSHIFT		/* 1/2 clock cycle shift to sample data */
/**
  * @}
  */ 

/*
 *  QSPI ChipSelect High Time
 */
#define QSPI_CS_HIGH_TIME_1_CYCLE       0x00000000				/* nCS stay high for at least 1 clock cycle between commands */
#define QSPI_CS_HIGH_TIME_2_CYCLE       QUADSPI_DCR_CSHT_0		/* nCS stay high for at least 2 clock cycles between commands */
#define QSPI_CS_HIGH_TIME_3_CYCLE       QUADSPI_DCR_CSHT_1		/* nCS stay high for at least 3 clock cycles between commands */
#define QSPI_CS_HIGH_TIME_4_CYCLE       (QUADSPI_DCR_CSHT_0 | QUADSPI_DCR_CSHT_1)	/* nCS stay high for at least 4 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_5_CYCLE       QUADSPI_DCR_CSHT_2		/* nCS stay high for at least 5 clock cycles between commands */
#define QSPI_CS_HIGH_TIME_6_CYCLE       (QUADSPI_DCR_CSHT_2 | QUADSPI_DCR_CSHT_0)	/* nCS stay high for at least 6 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_7_CYCLE       (QUADSPI_DCR_CSHT_2 | QUADSPI_DCR_CSHT_1)	/* nCS stay high for at least 7 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_8_CYCLE       QUADSPI_DCR_CSHT		/* nCS stay high for at least 8 clock cycles between commands */

/*
 *  QSPI Clock Mode
 */
#define QSPI_CLOCK_MODE_0               0x00000000				/* Clk stays low while nCS is released */
#define QSPI_CLOCK_MODE_3               QUADSPI_DCR_CKMODE		/* Clk goes high while nCS is released */

/*
 *  QSPI Flash Select
 */
#define QSPI_FLASH_ID_1                 0x00000000				/* FLASH 1 selected */
#define QSPI_FLASH_ID_2                 QUADSPI_CR_FSEL			/* FLASH 2 selected */

/*
 *  QSPI Dual Flash Mode
 */
#define QSPI_DUALFLASH_DISABLE          0x00000000				/* Dual-flash mode disabled */
#define QSPI_DUALFLASH_ENABLE           QUADSPI_CR_DFM			/* Dual-flash mode enabled */

/*
 *  QSPI DDR Mode
 */
#define QSPI_DDR_MODE_DISABLE           0x00000000				/* Double data rate mode disabled */
#define QSPI_DDR_MODE_ENABLE            QUADSPI_CCR_DDRM		/* Double data rate mode enabled */

/*
 *  QSPI DDR Data Output Delay
 */
#define QSPI_DDR_HHC_ANALOG_DELAY       0x00000000				/* Delay the data output using analog delay in DDR mode */
#ifdef QUADSPI_CCR_DHHC
#define QSPI_DDR_HHC_HALF_CLK_DELAY     QUADSPI_CCR_DHHC		/* Delay the data output by 1/2 clock cycle in DDR mode */
#endif

/*
 *   QSPI Send Instruction Mode
 */
#define QSPI_SIOO_INST_EVERY_CMD        0x00000000				/* Send instruction on every transaction */
#define QSPI_SIOO_INST_ONLY_FIRST_CMD   QUADSPI_CCR_SIOO		/* Send instruction only for the first command */



/*
 *  QSPI Address Size
 */
#define QSPI_ADDRESS_8_BITS             0x00000000				/* 8-bit address*/
#define QSPI_ADDRESS_16_BITS            QUADSPI_CCR_ADSIZE_0	/* 16-bit address*/
#define QSPI_ADDRESS_24_BITS            QUADSPI_CCR_ADSIZE_1	/* 24-bit address*/
#define QSPI_ADDRESS_32_BITS            QUADSPI_CCR_ADSIZE		/* 32-bit address*/

/*
 *  QSPI Alternate Bytes Size
 */
#define QSPI_ALTERNATE_BYTES_8_BITS     0x00000000				/* 8-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_16_BITS    QUADSPI_CCR_ABSIZE_0	/* 16-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_24_BITS    QUADSPI_CCR_ABSIZE_1	/* 24-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_32_BITS    QUADSPI_CCR_ABSIZE		/* 32-bit alternate bytes*/

/*
 *  QSPI Instruction Mode
 */
#define QSPI_INSTRUCTION_NONE           0x00000000				/* No instruction*/
#define QSPI_INSTRUCTION_1_LINE         QUADSPI_CCR_IMODE_0		/* Instruction on a single line*/
#define QSPI_INSTRUCTION_2_LINES        QUADSPI_CCR_IMODE_1		/* Instruction on two lines*/
#define QSPI_INSTRUCTION_4_LINES        QUADSPI_CCR_IMODE		/* Instruction on four lines*/

/*
 *  QSPI Address Mode
 */
#define QSPI_ADDRESS_NONE               0x00000000				/* No address*/
#define QSPI_ADDRESS_1_LINE             QUADSPI_CCR_ADMODE_0	/* Address on a single line*/
#define QSPI_ADDRESS_2_LINES            QUADSPI_CCR_ADMODE_1	/* Address on two lines*/
#define QSPI_ADDRESS_4_LINES            QUADSPI_CCR_ADMODE		/* Address on four lines*/

/*
 *  QSPI Alternate Bytes Mode
 */
#define QSPI_ALTERNATE_BYTES_NONE       0x00000000				/* No alternate bytes*/
#define QSPI_ALTERNATE_BYTES_1_LINE     QUADSPI_CCR_ABMODE_0	/* Alternate bytes on a single line*/
#define QSPI_ALTERNATE_BYTES_2_LINES    QUADSPI_CCR_ABMODE_1	/* Alternate bytes on two lines*/
#define QSPI_ALTERNATE_BYTES_4_LINES    QUADSPI_CCR_ABMODE		/* Alternate bytes on four lines*/

/*
 *  QSPI Data Mode
 */
#define QSPI_DATA_NONE                  0x00000000				/* No data */
#define QSPI_DATA_1_LINE                QUADSPI_CCR_DMODE_0		/* Data on a single line */
#define QSPI_DATA_2_LINES               QUADSPI_CCR_DMODE_1		/* Data on two lines */
#define QSPI_DATA_4_LINES               QUADSPI_CCR_DMODE		/* Data on four lines */


/*
 *  QSPI Match Mode
 */
#define QSPI_MATCH_MODE_AND             0x00000000				/* AND match mode between unmasked bits */
#define QSPI_MATCH_MODE_OR              QUADSPI_CR_PMM			/* OR match mode between unmasked bits*/

/*
 *  QSPI Automatic Stop
 */
#define QSPI_AUTOMATIC_STOP_DISABLE     0x00000000				/* AutoPolling stops only with abort or QSPI disabling*/
#define QSPI_AUTOMATIC_STOP_ENABLE      QUADSPI_CR_APMS			/* AutoPolling stops as soon as there is a match*/


/*
 *  QSPI ステータス
 */
#define QSPI_STATUS_RESET   0x00000000	/* Peripheral not initialized                            */
#define QSPI_STATUS_READY   0x00000001	/* Peripheral initialized and ready for use              */
#define QSPI_STATUS_BUSY    0x00000002	/* Peripheral in indirect mode and busy                  */ 
#define QSPI_STATUS_BUSY_TX 0x00000012	/* Peripheral in indirect mode with transmission ongoing */ 
#define QSPI_STATUS_BUSY_RX 0x00000022	/* Peripheral in indirect mode with reception ongoing    */
#define QSPI_STATUS_BUSY_AP 0x00000042	/* Peripheral in auto polling mode ongoing               */
#define QSPI_STATUS_BUSY_MM 0x00000082	/* Peripheral in memory mapped mode ongoing              */
#define QSPI_STATUS_ABORT   0x00010000	/* Peripheral with abort request ongoing                 */
#define QSPI_STATUS_ERROR   0x00020000	/* Peripheral in error                                   */

/*
 *  QSPIエラー
 */
#define QSPI_ERROR_NONE     0x00000000		/* No error */
#define QSPI_ERROR_TRANERR  0x00000002		/* transfer error */
#define QSPI_ERROR_TIMEOUT  0x00001000		/* Timeout */



/*
 *  QSPIハードウェア設定構造体
 */
typedef struct _QSPI_PortControlBlock{
	uint32_t              base;
	uint32_t              gioclockbase;

	uint32_t              giocsclockbit;
	uint32_t              giocsbase;
	uint32_t              giocspin;
	uint32_t              giocsaf;
	uint32_t              gioclkclockbit;
	uint32_t              gioclkbase;
	uint32_t              gioclkpin;
	uint32_t              gioclkaf;
	uint32_t              giod0clockbit;
	uint32_t              giod0base;
	uint32_t              giod0pin;
	uint32_t              giod0af;
	uint32_t              giod1clockbit;
	uint32_t              giod1base;
	uint32_t              giod1pin;
	uint32_t              giod1af;
	uint32_t              giod2clockbit;
	uint32_t              giod2base;
	uint32_t              giod2pin;
	uint32_t              giod2af;
	uint32_t              giod3clockbit;
	uint32_t              giod3base;
	uint32_t              giod3pin;
	uint32_t              giod3af;
} QSPI_PortControlBlock;


typedef ER (*QSPI_INIT_FUNC_T)(void *h);
typedef ER (*QSPI_EWRI_FUNC_T)(void *h, uint32_t timeout);
typedef ER (*QSPI_WAIT_FUNC_T)(void *h, uint32_t timeout);

/*
 *  QSPIデバイス設定構造体
 */
typedef struct QSPI_Init
{
	uint16_t                manuf_id;
    uint16_t                type_capacity;
	uint32_t                clockprescaler;		/* prescaler factor for generating clock based on the AHB clock. */
	uint32_t                fifothreshold;		/* threshold number of bytes in the FIFO (used only in indirect mode) */
	uint32_t                sampleshift;		/* Sample Shift. The data is sampled 1/2 clock cycle delay later to take in account external signal delays */
	uint32_t                chipselecthightime;	/* Chip Select High Time. */
	uint32_t                clockmode;			/* the Clock Mode. It indicates the level that clock takes between commands. */
	uint32_t                flashid;			/* the FlashID */
	uint32_t                dualflash;			/* the Dual Flash Mode State */
	uint32_t                ddrmode;
	uint32_t                ddrholdhalfcycle;
	uint32_t                sioomode;
	uint32_t                addr_size;
	uint32_t                inst_type;
	uint32_t                inst_data_xfer_type;
	uint32_t                read_op_code;
	uint32_t                read_addr_xfer_type;
	uint32_t                read_data_xfer_type;
	uint32_t                read_dummy_cycles;
	uint32_t                write_op_code;
	uint32_t                write_addr_xfer_type;
	uint32_t                write_data_xfer_type;
	uint32_t                write_dummy_cycles;
    uint8_t                 erase_count;
    uint32_t                erase_sizes[QSPI_MAX_ERASE_CMDS];
    uint8_t                 erase_cmds[QSPI_MAX_ERASE_CMDS];
	uint8_t                 erase_sector_idx;
	uint32_t                device_size;
	QSPI_INIT_FUNC_T        init_func;
	QSPI_EWRI_FUNC_T        enable_write_func;
	QSPI_WAIT_FUNC_T        wait_func;
	const char              *friendly_name;
} QSPI_Init_t;

/*
 *  QSPIコマンド定義型
 */
typedef struct
{
	uint32_t                InstructionMode;	/* the Instruction Mode */
	uint32_t                Instruction;		/* the Instruction to be sent */
	uint32_t                AddressMode;		/* the Address Mode */
	uint32_t                AddressSize;		/* the Address Size */
	uint32_t                Address;			/* the Address to be sent (Size from 1 to 4 bytes according AddressSize) */
	uint32_t                AlternateByteMode;	/* the Alternate Bytes Mode */
	uint32_t                AlternateBytesSize;	/* the Alternate Bytes Size */
	uint32_t                AlternateBytes;		/* the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize) */
	uint32_t                DummyCycles;		/* the Number of Dummy Cycles. */
	uint32_t                DataMode;			/* the Data Mode (used for dummy cycles and data phases) */
	uint32_t                NbData;				/* the number of data to transfer. (This is the number of bytes) */
} QSPI_Command_t;

/*
 *  QSPIオートポーリング型
 */
typedef struct
{
	uint32_t                Match;				/* the value to be compared with the masked status register to get a match. */
	uint32_t                Mask;				/* the mask to be applied to the status bytes received. */
	uint32_t                Interval;			/* the number of clock cycles between two read during automatic polling phases. */
	uint32_t                StatusBytesSize;	/* the size of the status bytes received. */
	uint32_t                MatchMode;			/* the method used for determining a match. */
	uint32_t                AutomaticStop;		/* if automatic polling is stopped after a match. */
} QSPI_AutoPolling_t;

/*
 *  QSPIデバイスハンドラ定義
 */
typedef struct _QSPI_Handle_t QSPI_Handle_t;
struct _QSPI_Handle_t
{
	uint32_t              base;			/* QSPI レジスタベースアドレス */
	QSPI_Init_t           Init;			/* QSPI FLASHデータ */
	uint32_t              *pBuffPtr;	/* QSPI 通信バッファへのポインタ */
	uint32_t              XferSize;		/* QSPI 通信サイズ */
	volatile uint32_t     XferCount;	/* QSPI 通信カウンタ */
	void               (*timeoutcallback)(QSPI_Handle_t * hqspi);	/* タイムアウトコールバック関数 */
	void               (*errorcallback)(QSPI_Handle_t * hqspi);		/* エラーコールバック関数 */
	volatile uint32_t     istatus;		/* QSPI 割込みステータス */
	uint32_t              timeout;		/* QSPI タイムアウト値(ms) */
	int                   semid;		/* QSPI 通信用セマフォ値 */
	uint32_t              status;		/* QSPI ステータス */
	volatile uint32_t     errorcode;	/* QSPI エラーコード */
};

extern QSPI_Handle_t *qspi_init(ID portid, QSPI_Init_t *pini);
extern ER qspi_deinit(QSPI_Handle_t *hqspi);
extern ER qspi_read(QSPI_Handle_t *hqspi, void * dest, uint32_t src, uint32_t size);
extern ER qspi_write(QSPI_Handle_t *hqspi, uint32_t dest, const void * src, uint32_t size);
extern ER qspi_erase(QSPI_Handle_t *hqspi, uint32_t address, uint32_t size); 
extern ER qspi_erase_sector(QSPI_Handle_t *hqspi, uint32_t address);
extern ER qspi_direct_disable(QSPI_Handle_t *hqspi);
extern ER qspi_direct_enable(QSPI_Handle_t *hqspi);
extern void qspi_handler(QSPI_Handle_t *hqspi);
extern void qspi_isr(intptr_t exinf);

/*
 *  CHIP消去
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  return             正常終了時、E_OK
 */
#define qspi_erase_chip(h) qspi_erase(h, 0, (h)->Init.device_size)


/*
 *  セクターサイズを取り出す
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  return             セクターバイトサイズ
 */
#define get_smallest_sector_size(h) (h)->Init.erase_sizes[(h)->Init.erase_sector_idx]


/*
 *  デバイスサイズを取り出す
 *  parameter1 hqspi   QSPIハンドラへのポインタ
 *  return             デバイスバイトサイズ
 */
#define qspi_get_device_size(h)  (h)->Init.device_size;

extern ER qspi_command(QSPI_Handle_t *hqspi, QSPI_Command_t *cmd, uint32_t Timeout);
extern ER qspi_autopolling(QSPI_Handle_t *hqspi, QSPI_Command_t *cmd, QSPI_AutoPolling_t *cfg, uint32_t Timeout);
extern ER qspi_transmit(QSPI_Handle_t *hqspi, uint32_t *pData, uint32_t Timeout);
extern ER qspi_receive(QSPI_Handle_t *hqspi, uint32_t *pData, uint32_t Timeout);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif	/* _QSPI_H_ */

