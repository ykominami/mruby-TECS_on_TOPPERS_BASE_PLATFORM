/*
 *
 *  STM32F746-Discovery SDRAM設定
 *
 */
#include "kernel_impl.h"
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <target_syssvc.h>
#include "device.h"

/*
 *  SIL関数のマクロ定義
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

/*
 *  AF 12セクション定義
 */ 
#define GPIO_AF12_FMC           0x0C	/* FMC Alternate Function mapping                      */
#define GPIO_AF12_OTG_HS_FS     0x0C	/* OTG HS configured in FS, Alternate Function mapping */
#define GPIO_AF12_SDMMC1        0x0C	/* SDMMC1 Alternate Function mapping                     */

/*
 *  SDRAM設定タイムアウト定義(μsec)
 */
#define SDRAM_TIMEOUT          (0xFFFF*1000)

/*
 *  FMC SDRAMモードコマンド定義
 */
#define FMC_SDRAM_CMD_NORMAL_MODE       0x00000000
#define FMC_SDRAM_CMD_CLK_ENABLE        0x00000001
#define FMC_SDRAM_CMD_PALL              0x00000002
#define FMC_SDRAM_CMD_AUTOREFRESH_MODE  0x00000003
#define FMC_SDRAM_CMD_LOAD_MODE         0x00000004
#define FMC_SDRAM_CMD_SELFREFRESH_MODE  0x00000005
#define FMC_SDRAM_CMD_POWERDOWN_MODE    0x00000006

/*
 *  FMC SDRAM カラムビット番号
 */
#define FMC_SDRAM_COLUMN_BITS_NUM_8     0x00000000
#define FMC_SDRAM_COLUMN_BITS_NUM_9     0x00000001
#define FMC_SDRAM_COLUMN_BITS_NUM_10    0x00000002
#define FMC_SDRAM_COLUMN_BITS_NUM_11    0x00000003

/*
 *  FMC SDRAM ロービット番号
 */
#define FMC_SDRAM_ROW_BITS_NUM_11       0x00000000
#define FMC_SDRAM_ROW_BITS_NUM_12       0x00000004
#define FMC_SDRAM_ROW_BITS_NUM_13       0x00000008

/*
 *  FMC SDRAM メモリビット幅
 */
#define FMC_SDRAM_MEM_BUS_WIDTH_8       0x00000000
#define FMC_SDRAM_MEM_BUS_WIDTH_16      0x00000010
#define FMC_SDRAM_MEM_BUS_WIDTH_32      0x00000020

/*
 *  FMC SDRAM インターナルバンク番号
 */
#define FMC_SDRAM_INTERN_BANKS_NUM_2    0x00000000
#define FMC_SDRAM_INTERN_BANKS_NUM_4    0x00000040

/*
 *  FMC SDRAM CAS レーテンシィ
 */
#define FMC_SDRAM_CAS_LATENCY_1         0x00000080
#define FMC_SDRAM_CAS_LATENCY_2         0x00000100
#define FMC_SDRAM_CAS_LATENCY_3         0x00000180

/*
 *  FMC SDRAM ライトプロテクション
 */
#define FMC_SDRAM_WRITE_PROTECTION_DISABLE  0x00000000
#define FMC_SDRAM_WRITE_PROTECTION_ENABLE   0x00000200

/*
 *  FMC SDRAM リードブースト
 */
#define FMC_SDRAM_RBURST_DISABLE        0x00000000
#define FMC_SDRAM_RBURST_ENABLE         0x00001000

/*
 *  FMC SDRAM リードパイプデレィ
 */
#define FMC_SDRAM_RPIPE_DELAY_0         0x00000000
#define FMC_SDRAM_RPIPE_DELAY_1         0x00002000
#define FMC_SDRAM_RPIPE_DELAY_2         0x00004000

/*
 *  FMC SDRAM Clock Period
 */
#define FMC_SDRAM_CLOCK_DISABLE         0x00000000
#define FMC_SDRAM_CLOCK_PERIOD_2        0x00000800
#define FMC_SDRAM_CLOCK_PERIOD_3        0x00000C00


#define SDRAM_MODEREG_BURST_LENGTH_1             0x0000
#define SDRAM_MODEREG_BURST_LENGTH_2             0x0001
#define SDRAM_MODEREG_BURST_LENGTH_4             0x0002
#define SDRAM_MODEREG_BURST_LENGTH_8             0x0004
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      0x0000
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     0x0008
#define SDRAM_MODEREG_CAS_LATENCY_2              0x0020
#define SDRAM_MODEREG_CAS_LATENCY_3              0x0030
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    0x0000
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED 0x0000
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     0x0200


/*
 *  FMC SDRAM コンフィギュレーション定義
 */
typedef struct {
	uint32_t        ColumnBitsNumber;		/* Defines the number of bits of column address. FMC_NORSRAM_Bank */
	uint32_t        RowBitsNumber;			/* Defines the number of bits of column address. FMC_SDRAM_Row_Bits_number. */
	uint32_t        MemoryDataWidth;		/* Defines the memory device width. FMC_SDRAM_Memory_Bus_Width. */
	uint32_t        InternalBankNumber;		/* Defines the number of the device's internal banks. FMC_SDRAM_Internal_Banks_Number. */
	uint32_t        CASLatency;				/* Defines the SDRAM CAS latency in number of memory clock cycles. FMC_SDRAM_CAS_Latency. */
	uint32_t        WriteProtection;		/* Enables the SDRAM device to be accessed in write mode. FMC_SDRAM_Write_Protection. */
	uint32_t        SDClockPeriod;			/* Define the SDRAM Clock Period for both SDRAM devices and they allow 
                                             to disable the clock before changing frequency. FMC_SDRAM_Clock_Period. */
	uint32_t        ReadBurst;				/* This bit enable the SDRAM controller to anticipate the next read 
                                             commands during the CAS latency and stores data in the Read FIFO. FMC_SDRAM_Read_Burst. */
	uint32_t        ReadPipeDelay;			/* Define the delay in system clock cycles on read data path. FMC_SDRAM_Read_Pipe_Delay. */
}FMC_SDRAM_Init_t;

/*
 *  FMC SDRAM タイミング パラメータ
 */
typedef struct {
	uint32_t        LoadToActiveDelay;		/* Defines the delay between a Load Mode Register command and 
                                              an active or Refresh command in number of memory clock cycles. */
    uint32_t        ExitSelfRefreshDelay;	/* Defines the delay from releasing the self refresh command to 
                                              issuing the Activate command in number of memory clock cycles. */
    uint32_t        SelfRefreshTime;		/* Defines the minimum Self Refresh period in number of memory clock cycles.*/
    uint32_t        RowCycleDelay;			/* Defines the delay between the Refresh command and the Activate command
                                              and the delay between two consecutive Refresh commands in number of memory clock cycles. */
    uint32_t        WriteRecoveryTime;		/* Defines the Write recovery Time in number of memory clock cycles. */
    uint32_t        RPDelay;				/* Defines the delay between a Precharge Command and an other command 
                                              in number of memory clock cycles. */
    uint32_t        RCDDelay;				/* Defines the delay between the Activate Command and a Read/Write 
                                              command in number of memory clock cycles. */
}FMC_SDRAM_Timing_t;

#if defined(TOPPERS_STM32F769_DISCOVERY)
#define SDRAM_AHB1ENR (RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN | \
					   RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN | \
					   RCC_AHB1ENR_GPIOHEN | RCC_AHB1ENR_GPIOIEN)
#define SDRAM_MODEREG_CAS_LATENCY SDRAM_MODEREG_CAS_LATENCY_3
#define REFLESH_RATE  1539
static const GPIO_Init_Table sdram_gpio_table[] = {
	{TADR_GPIOD_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
					  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOF_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | \
					  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOG_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15) },
	{TADR_GPIOH_BASE, (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
					  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOI_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | \
					  GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10) }
};

static const FMC_SDRAM_Init_t sdinit = {
	FMC_SDRAM_COLUMN_BITS_NUM_8,		/* ColumnBitsNumber */
	FMC_SDRAM_ROW_BITS_NUM_12,			/* RowBitsNumber */
	FMC_SDRAM_MEM_BUS_WIDTH_32,			/* MemoryDataWidth */
	FMC_SDRAM_INTERN_BANKS_NUM_4,		/* InternalBankNumber */
	FMC_SDRAM_CAS_LATENCY_3,			/* CASLatency */
	FMC_SDRAM_WRITE_PROTECTION_DISABLE,	/* WriteProtection */
	FMC_SDRAM_CLOCK_PERIOD_2,			/* SDClockPeriod */
	FMC_SDRAM_RBURST_ENABLE,			/* ReadBurst */
	FMC_SDRAM_RPIPE_DELAY_1				/* ReadPipeDelay */
};
static const FMC_SDRAM_Timing_t sdtiming = {
	2,									/* LoadToActiveDelay */
	7,									/* ExitSelfRefreshDelay */
	4,									/* SelfRefreshTime */
	7,									/* RowCycleDelay */
	2,									/* WriteRecoveryTime */
	2,									/* RPDelay */
	2									/* RCDDelay */
};

#else
#define SDRAM_AHB1ENR (RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | \
					   RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | \
					   RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN)
#define SDRAM_MODEREG_CAS_LATENCY SDRAM_MODEREG_CAS_LATENCY_2
#define REFLESH_RATE  1292
static const GPIO_Init_Table sdram_gpio_table[] = {
	{TADR_GPIOC_BASE, (GPIO_PIN_3) },
	{TADR_GPIOD_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
					  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOF_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | \
					  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) },
	{TADR_GPIOG_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15) },
	{TADR_GPIOH_BASE, (GPIO_PIN_3 | GPIO_PIN_5) }
};

static const FMC_SDRAM_Init_t sdinit = {
	FMC_SDRAM_COLUMN_BITS_NUM_8,		/* ColumnBitsNumber */
	FMC_SDRAM_ROW_BITS_NUM_12,			/* RowBitsNumber */
	FMC_SDRAM_MEM_BUS_WIDTH_16,			/* MemoryDataWidth */
	FMC_SDRAM_INTERN_BANKS_NUM_4,		/* InternalBankNumber */
	FMC_SDRAM_CAS_LATENCY_2,			/* CASLatency */
	FMC_SDRAM_WRITE_PROTECTION_DISABLE,	/* WriteProtection */
	FMC_SDRAM_CLOCK_PERIOD_2,			/* SDClockPeriod */
	FMC_SDRAM_RBURST_ENABLE,			/* ReadBurst */
	FMC_SDRAM_RPIPE_DELAY_1				/* ReadPipeDelay */
};
static const FMC_SDRAM_Timing_t sdtiming = {
	2,									/* LoadToActiveDelay */
	7,									/* ExitSelfRefreshDelay */
	4,									/* SelfRefreshTime */
	7,									/* RowCycleDelay */
	2,									/* WriteRecoveryTime */
	2,									/* RPDelay */
	2									/* RCDDelay */
};

#endif

#define NUM_SDRAM_GPIO_ITEM (sizeof(sdram_gpio_table)/sizeof(GPIO_Init_Table))

/*
 *  SDRAM GPIO初期化
 */
static void
sdram_gpio_init(void)
{
	GPIO_Init_t GPIO_Init_Data;
	int i, pin;
	volatile uint32_t tmp;

	/*
	 *  GPIOクロック設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), SDRAM_AHB1ENR);
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));

	/*
	 *  FMCクロック設定
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR), RCC_AHB3ENR_FSMCEN);
	tmp = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB3ENR));
	(void)(tmp);

	/*
	 *  GPIO設定
	 */
	GPIO_Init_Data.mode      = GPIO_MODE_AF;
	GPIO_Init_Data.pull      = GPIO_PULLUP;
	GPIO_Init_Data.otype     = GPIO_OTYPE_PP;
#if defined(TOPPERS_STM32F769_DISCOVERY)
	GPIO_Init_Data.speed     = GPIO_SPEED_HIGH;
#else
	GPIO_Init_Data.speed     = GPIO_SPEED_FAST;
#endif
	GPIO_Init_Data.alternate = GPIO_AF12_FMC;

	for(i = 0 ; i < NUM_SDRAM_GPIO_ITEM ; i++){
		for(pin = 0 ; pin < 16 ; pin++){
			if((sdram_gpio_table[i].pinmap & 1<<pin) != 0)
				gpio_setup(sdram_gpio_table[i].base, &GPIO_Init_Data, pin);
		}
	}
}

/*
 *  FMC SDRAMバンクへのコマンド送信
 *  patameter1 mode   コマンド値
 *  parameter2 target ターゲット番号
 *  parameter3 num    バンク番号
 *  parameter4 moder  モード
 *  parameter5 timeout タイムアウト値
 *  return     正常終了でtrue
 */
static bool_t
FMC_SDRAM_SendCommand(uint32_t cmd, uint32_t target, uint32_t num, uint32_t moder, uint32_t Timeout)
{
	uint32_t tickstart = 0;

	/*
	 *  コマンドレジスタ設定
	 */
	sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCMR), (cmd | target | ((num-1) << 5) | (moder << 9)));

	/*
	 *  コマンド送信待ち
	 */
	tickstart = 0;
	while((sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDSR)) & FMC_SDSR_BUSY) != 0){
		if(tickstart > Timeout){
			return false;
		}
		sil_dly_nse(1000);
		tickstart++;
	}
	return true;
}

/*
 *  拡張SDRAMの初期化
 */
void
sdram_init(intptr_t exinf)
{
	uint32_t Bank = (uint32_t)exinf;
	volatile uint32_t tmpr1 = 0;
	volatile uint32_t tmpr2 = 0;

    /*
	 *  GPIO,クロック設定
	 */
    sdram_gpio_init();

	/*
	 *  SDRAM制御タイミング設定
	 */
	if (Bank != FMC_SDRAM_BANK2){	/* Bank1 */
		tmpr1 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR0));

		/* Clear NC, NR, MWID, NB, CAS, WP, SDCLK, RBURST, and RPIPE bits */
		tmpr1 &= ((uint32_t)~(FMC_SDCR1_NC  | FMC_SDCR1_NR | FMC_SDCR1_MWID | \
							FMC_SDCR1_NB  | FMC_SDCR1_CAS | FMC_SDCR1_WP | \
							FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

		tmpr1 |= (uint32_t)(sdinit.ColumnBitsNumber   |
							sdinit.RowBitsNumber      |
							sdinit.MemoryDataWidth    |
							sdinit.InternalBankNumber |
							sdinit.CASLatency         |
							sdinit.WriteProtection    |
							sdinit.SDClockPeriod      |
							sdinit.ReadBurst          |
							sdinit.ReadPipeDelay );
		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR0), tmpr1);

		tmpr1 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR0));
		/* Clear TMRD, TXSR, TRAS, TRC, TWR, TRP and TRCD bits */
		tmpr1 &= ((uint32_t)~(FMC_SDTR1_TMRD  | FMC_SDTR1_TXSR | FMC_SDTR1_TRAS | 
							FMC_SDTR1_TRC  | FMC_SDTR1_TWR | FMC_SDTR1_TRP | FMC_SDTR1_TRCD));

		tmpr1 |= (uint32_t)(((sdtiming.LoadToActiveDelay)-1)           |
							(((sdtiming.ExitSelfRefreshDelay)-1) << 4) |
							(((sdtiming.SelfRefreshTime)-1) << 8)      |
							(((sdtiming.RowCycleDelay)-1) << 12)       |
							(((sdtiming.WriteRecoveryTime)-1) <<16)    |
							(((sdtiming.RPDelay)-1) << 20)             |
							(((sdtiming.RCDDelay)-1) << 24));
		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR0), tmpr1);
	}
	else{ /* Bank 2 */
		tmpr1 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR0));

		/* Clear NC, NR, MWID, NB, CAS, WP, SDCLK, RBURST, and RPIPE bits */
		tmpr1 &= ((uint32_t)~(FMC_SDCR1_NC  | FMC_SDCR1_NR | FMC_SDCR1_MWID | 
							FMC_SDCR1_NB  | FMC_SDCR1_CAS | FMC_SDCR1_WP | 
							FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

		tmpr1 |= (uint32_t)(sdinit.SDClockPeriod      |
							sdinit.ReadBurst          |
							sdinit.ReadPipeDelay);

		tmpr2 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR1));
		/* Clear NC, NR, MWID, NB, CAS, WP, SDCLK, RBURST, and RPIPE bits */
		tmpr2 &= ((uint32_t)~(FMC_SDCR1_NC  | FMC_SDCR1_NR | FMC_SDCR1_MWID |
							FMC_SDCR1_NB  | FMC_SDCR1_CAS | FMC_SDCR1_WP | 
							FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

		tmpr2 |= (uint32_t)(sdinit.ColumnBitsNumber   |
							sdinit.RowBitsNumber      |
							sdinit.MemoryDataWidth    |
							sdinit.InternalBankNumber |
							sdinit.CASLatency         |
							sdinit.WriteProtection);

		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR0), tmpr1);
		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDCR1), tmpr2);

		tmpr1 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR1));
		/* Clear TMRD, TXSR, TRAS, TRC, TWR, TRP and TRCD bits */
		tmpr1 &= ((uint32_t)~(FMC_SDTR1_TMRD  | FMC_SDTR1_TXSR | FMC_SDTR1_TRAS |
							FMC_SDTR1_TRC  | FMC_SDTR1_TWR | FMC_SDTR1_TRP | FMC_SDTR1_TRCD));

		tmpr1 |= (uint32_t)(((sdtiming.LoadToActiveDelay)-1)           |
							(((sdtiming.ExitSelfRefreshDelay)-1) << 4) |
							(((sdtiming.SelfRefreshTime)-1) << 8)      |
							(((sdtiming.WriteRecoveryTime)-1) <<16)    |
							(((sdtiming.RCDDelay)-1) << 24));

		tmpr2 = sil_rew_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR0));

		/* Clear TMRD, TXSR, TRAS, TRC, TWR, TRP and TRCD bits */
		tmpr2 &= ((uint32_t)~(FMC_SDTR1_TMRD  | FMC_SDTR1_TXSR | FMC_SDTR1_TRAS |
							FMC_SDTR1_TRC  | FMC_SDTR1_TWR | FMC_SDTR1_TRP | FMC_SDTR1_TRCD));
		tmpr2 |= (uint32_t)((((sdtiming.RowCycleDelay)-1) << 12) |
							(((sdtiming.RPDelay)-1) << 20)); 

		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR1), tmpr1);
		sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDTR0), tmpr2);
	}

	/*
	 * SDRAM外部デバイス設定
	 */
	/* クロックコンフィギュレーション許可コマンド */
	FMC_SDRAM_SendCommand(FMC_SDRAM_CMD_CLK_ENABLE, FMC_SDCMR_CTB1, 1, 0, SDRAM_TIMEOUT);

	/*  300us待ち */
	sil_dly_nse(300*1000);

	/*  PALLコンフィギュレーションコマンド */
	FMC_SDRAM_SendCommand(FMC_SDRAM_CMD_PALL, FMC_SDCMR_CTB1, 1, 0, SDRAM_TIMEOUT);

	/*  オートリフレッシュ設定コマンド */
	FMC_SDRAM_SendCommand(FMC_SDRAM_CMD_AUTOREFRESH_MODE, FMC_SDCMR_CTB1, 8, 0, SDRAM_TIMEOUT);

	/*  外部メモリプログラミングレジスタ設定 */
	tmpr1 = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1         |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY             |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	FMC_SDRAM_SendCommand(FMC_SDRAM_CMD_LOAD_MODE, FMC_SDCMR_CTB1, 1, tmpr1, SDRAM_TIMEOUT);

	/*  リフレッシュレート設定: (15.62 us x Freq) - 20 */
	sil_wrw_mem((uint32_t *)(TADR_FMC_R_BASE+TOFF_FMC_R_SDRTR), (REFLESH_RATE)<< 1);

	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR), RCC_AHB1ENR_CRCEN);
	tmpr1 = sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_AHB1ENR));
}

