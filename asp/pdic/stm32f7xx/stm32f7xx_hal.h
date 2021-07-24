
#ifndef __STM32F7xx_HAL_H
#define __STM32F7xx_HAL_H

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>

#define _GCC_WRAP_STDINT_H
#include "device.h"
#include "kernel_cfg.h"
#include "i2c.h"
#include "ltdc.h"
#include "sai.h"

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

#ifdef TOPPERS_STM32F769_DISCOVERY

#include "dsi.h"
#include "dfsdm.h"

#include "stm32f769xx.h"

#else	/* TOPPERS_STM32F769_DISCOVERY */

/* Include LCD component Driver */
/* LCD RK043FN48H-CT672B 4,3" 480x272 pixels */
#include "rk043fn48h.h"

/* Include SDRAM Driver */
#include "fonts.h"
  
#include "sdmmc.h"

#define TS_I2C_ADDRESS                   ((uint16_t)0x70)

#include "stm32f756xx.h"

#endif	/* TOPPERS_STM32F769_DISCOVERY */

#if  defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif /* __weak */
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif /* __packed */
#endif /* __GNUC__ */

/* Macro to get variable aligned on 4-bytes, for __ICCARM__ the directive "#pragma data_alignment=4" must be used instead */
#if defined   (__GNUC__)        /* GNU Compiler */
  #ifndef __ALIGN_END
    #define __ALIGN_END    __attribute__ ((aligned (4)))
  #endif /* __ALIGN_END */
  #ifndef __ALIGN_BEGIN  
    #define __ALIGN_BEGIN
  #endif /* __ALIGN_BEGIN */
#else
  #ifndef __ALIGN_END
    #define __ALIGN_END
  #endif /* __ALIGN_END */
  #ifndef __ALIGN_BEGIN      
    #if defined   (__CC_ARM)      /* ARM Compiler */
      #define __ALIGN_BEGIN    __align(4)  
    #elif defined (__ICCARM__)    /* IAR Compiler */
      #define __ALIGN_BEGIN 
    #endif /* __CC_ARM */
  #endif /* __ALIGN_BEGIN */
#endif /* __GNUC__ */

typedef enum 
{
  DISABLE = 0, 
  ENABLE = !DISABLE
} FunctionalState;

/** 
  * @brief  GPIO Bit SET and Bit RESET enumeration 
  */
typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
}GPIO_PinState;

#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__)               \
                        do{                                                      \
                              (__HANDLE__)->__PPP_DMA_FIELD__ = &(__DMA_HANDLE__); \
                              (__DMA_HANDLE__).Parent = (__HANDLE__);             \
                          } while(0)

/** @addtogroup Exported_macro
  * @{
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define READ_REG(REG)         ((REG))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define UNUSED(x) ((void)(x))

#define HAL_StatusTypeDef   ER
#define HAL_OK	            E_OK

/** @defgroup RCCEx_PLLSAI_DIVR RCCEx PLLSAI DIVR
  * @{
  */
#define RCC_PLLSAIDIVR_2                ((uint32_t)0x00000000U)
#define RCC_PLLSAIDIVR_4                RCC_DCKCFGR1_PLLSAIDIVR_0
#define RCC_PLLSAIDIVR_8                RCC_DCKCFGR1_PLLSAIDIVR_1
#define RCC_PLLSAIDIVR_16               RCC_DCKCFGR1_PLLSAIDIVR

/** @addtogroup STM32F7xx_HAL_Driver
  * @{
  */

#define GPIO_AF3_DFSDM1        ((uint8_t)0x03U)  /* DFSDM1 Alternate Function mapping */

/** 
  * @brief   AF 4 selection  
  */ 
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping */
#define GPIO_AF4_I2C3          ((uint8_t)0x04)  /* I2C3 Alternate Function mapping */
#define GPIO_AF4_I2C4          ((uint8_t)0x04)  /* I2C4 Alternate Function mapping */
#define GPIO_AF4_CEC           ((uint8_t)0x04)  /* CEC Alternate Function mapping */

#define RCC_DFSDM1AUDIOCLKSOURCE_SAI1        ((uint32_t)0x00000000U)
#define RCC_DFSDM1AUDIOCLKSOURCE_SAI2        RCC_DCKCFGR1_ADFSDM1SEL

#define __HAL_RCC_GPIOB_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_GPIOC_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_GPIOD_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_GPIOH_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_GPIOI_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOIEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOIEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_GPIOJ_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOJEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOJEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_I2C1_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_I2C3_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C3EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C3EN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_DMA2_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);\
                                        UNUSED(tmpreg); \
                                      } while(0)  



#define __HAL_RCC_DMA2D_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2DEN);\
                                        UNUSED(tmpreg); \
                                      } while(0) 

#define __HAL_RCC_LTDC_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_LTDCEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_DFSDM1_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DFSDM1EN);\
                                        UNUSED(tmpreg); \
                                      } while(0)    

#define __HAL_RCC_DSI_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_DSIEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)

#define __HAL_RCC_DMA2D_CLK_DISABLE()           (RCC->AHB1ENR &= ~(RCC_AHB1ENR_DMA2DEN))
#define __HAL_RCC_LTDC_CLK_DISABLE()   (RCC->APB2ENR &= ~(RCC_APB2ENR_LTDCEN))
#define __HAL_RCC_DSI_CLK_DISABLE()    (RCC->APB2ENR &= ~(RCC_APB2ENR_DSIEN))

#define __HAL_RCC_I2C1_FORCE_RESET()     (RCC->APB1RSTR |= (RCC_APB1RSTR_I2C1RST))
#define __HAL_RCC_I2C3_FORCE_RESET()     (RCC->APB1RSTR |= (RCC_APB1RSTR_I2C3RST))
#define __HAL_RCC_I2C1_RELEASE_RESET()   (RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C1RST))
#define __HAL_RCC_I2C3_RELEASE_RESET()   (RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C3RST))
#define __HAL_RCC_DMA2D_FORCE_RESET()    (RCC->AHB1RSTR |= (RCC_AHB1RSTR_DMA2DRST))
#define __HAL_RCC_DMA2D_RELEASE_RESET()  (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_DMA2DRST))
#define __HAL_RCC_LTDC_FORCE_RESET()     (RCC->APB2RSTR |= (RCC_APB2RSTR_LTDCRST))
#define __HAL_RCC_LTDC_RELEASE_RESET()   (RCC->APB2RSTR &= ~(RCC_APB2RSTR_LTDCRST))
#define __HAL_RCC_DSI_FORCE_RESET()      (RCC->APB2RSTR |= (RCC_APB2RSTR_DSIRST))
#define __HAL_RCC_DSI_RELEASE_RESET()    (RCC->APB2RSTR &= ~(RCC_APB2RSTR_DSIRST))

#define HAL_Delay(a)                dly_tsk(a)

#define UART_HandleTypeDef          uint32_t

#define GPIO_InitTypeDef            GPIO_Init_t

#define DMA_HandleTypeDef           DMA_Handle_t

#define HAL_DMA_Init                dma_init
#define HAL_DMA_DeInit              dma_deinit
#define HAL_DMA_Start_IT            dma_start
#define HAL_DMA_Abort               dma_end
#define HAL_DMA_IRQHandler          dma_inthandler

#define DMA2D_INPUT_ARGB8888        DMA2D_ARGB8888
#define DMA2D_INPUT_RGB888          DMA2D_RGB888
#define DMA2D_INPUT_RGB565          DMA2D_RGB565
#define DMA2D_INPUT_ARGB1555        DMA2D_ARGB1555
#define DMA2D_INPUT_ARGB4444        DMA2D_ARGB4444

#define DMA2D_OUTPUT_ARGB8888       CM_ARGB8888
#define DMA2D_OUTPUT_RGB888         CM_RGB888
#define DMA2D_OUTPUT_RGB565         CM_RGB565
#define DMA2D_OUTPUT_ARGB1555       CM_ARGB1555
#define DMA2D_OUTPUT_ARGB4444       CM_ARGB4444

#define DMA2D_HandleTypeDef         DMA2D_Handle_t

#define HAL_DMA2D_Init              dma2d_init
#define HAL_DMA2D_ConfigLayer       dma2d_configlayer
#define HAL_DMA2D_Start             dma2d_start
#define HAL_DMA2D_PollForTransfer   dma2d_waittransfar

#define I2C_HandleTypeDef           I2C_Handle_t

#define HAL_I2C_STATE_RESET         I2C_STATE_RESET
#define HAL_I2C_STATE_READY         I2C_STATE_READY
#define HAL_I2C_STATE_BUSY          I2C_STATE_BUSY
#define HAL_I2C_STATE_MASTER_BUSY_TX I2C_STATE_MASTER_BUSY_TX
#define HAL_I2C_STATE_MEM_BUSY_TX   I2C_STATE_MEM_BUSY_TX
#define HAL_I2C_STATE_MEM_BUSY_RX   I2C_STATE_MEM_BUSY_RX

#define HAL_I2C_Init                i2c_init
#define HAL_I2C_DeInit              i2c_deinit
#define HAL_I2C_Mem_Read            i2c_memread
#define HAL_I2C_Mem_Write           i2c_memwrite
#define HAL_I2C_IsDeviceReady       i2c_ready
#define HAL_I2C_GetState(handle)    (handle->State)

#define LTDC_FLAG_LI                LTDC_ISR_LIF
#define LTDC_FLAG_FU                LTDC_ISR_FUIF
#define LTDC_FLAG_TE                LTDC_ISR_TERRIF
#define LTDC_FLAG_RR                LTDC_ISR_RRIF

#define LTDC_InitTypeDef            LTDC_Init_t
#define LTDC_LayerCfgTypeDef        LTDC_LayerCfg_t
#define LTDC_HandleTypeDef          LTDC_Handle_t

#define HAL_LTDC_Init               ltdc_init
#define HAL_LTDC_ConfigLayer        ltdc_configlayer
#define HAL_LTDC_SetAlpha           ltdc_setalpha
#define HAL_LTDC_SetAddress         ltdc_setaddress
#define HAL_LTDC_SetWindowSize      ltdc_setwindowsize
#define HAL_LTDC_SetWindowPosition  ltdc_setwindowposition
#define HAL_LTDC_ConfigColorKeying  ltdc_configcolorkeying
#define HAL_LTDC_EnableColorKeying  ltdc_enablecolorkeying
#define HAL_LTDC_DisableColorKeying ltdc_disablecolorkeying

#define DSI_HandleTypeDef           DSI_Handle_t
#define DSI_VidCfgTypeDef           DSI_VideoConfig_t
#define DSI_CmdCfgTypeDef           DSI_CommandConfig_t
#define DSI_PHY_TimerTypeDef        DSI_PHY_Time_t
#define DSI_HOST_TimeoutTypeDef     DSI_HostTimeout_t

#define HAL_DSI_DeInit              dsi_deinit
#define HAL_DSI_ConfigVideoMode     dsi_configvideo
#define HAL_DSI_Start               dsi_start
#define HAL_DSI_Stop                dsi_stop
#define HAL_DSI_ShortWrite          dsi_swrite
#define HAL_DSI_LongWrite           dsi_lwrite
#define HAL_LTDC_StructInitFromVideoConfig dci_configltdc

#define SD_HandleTypeDef            SDMMC_Handle_t
#define HAL_SD_ErrorTypedef         ER
#define HAL_SD_CardInfoTypedef      SDMMC_CardInfo_t
#define HAL_SD_TransferStateTypedef uint32_t
#define SD_OK                       E_OK

#define STD_CAPACITY_SD_CARD_V1_1   SD_CARD_V11
#define STD_CAPACITY_SD_CARD_V2_0   SD_CARD_V20
#define HIGH_CAPACITY_SD_CARD       SD_CARD_HC
#define MULTIMEDIA_CARD             MMC_CARD
#define HIGH_CAPACITY_MMC_CARD      MMC_CARD_HC
#define SECURE_DIGITAL_IO_CARD      SD_IO_CARD
#define SECURE_DIGITAL_IO_COMBO_CARD SD_IO_COMBO_CARD
#define HIGH_SPEED_MULTIMEDIA_CARD  HS_MM_CARD

#define HAL_SD_DeInit               sdmmc_close
#define HAL_SD_Erase                sdmmc_erase
#define HAL_SD_ReadBlocks_DMA       sdmmc_blockread
#define HAL_SD_WriteBlocks_DMA      sdmmc_blockwrite
#define HAL_SD_CheckReadOperation   sdmmc_wait_transfar
#define HAL_SD_CheckWriteOperation  sdmmc_wait_transfar
#define HAL_SD_IRQHandler           sdmmc_checkint
#define HAL_SD_Get_CardInfo         sdmmc_getcardinfo
#define HAL_SD_WideBusOperation_Config sdmmc_set_widebus
#define HAL_SD_GetStatus            sdmmc_getstatus

#define SAI_HandleTypeDef AUDIO_Handle_t

#define SAI_AUDIO_FREQUENCY_192K          ((uint32_t)192000U)
#define SAI_AUDIO_FREQUENCY_96K           ((uint32_t)96000U)
#define SAI_AUDIO_FREQUENCY_48K           ((uint32_t)48000U)
#define SAI_AUDIO_FREQUENCY_44K           ((uint32_t)44100U)
#define SAI_AUDIO_FREQUENCY_32K           ((uint32_t)32000U)
#define SAI_AUDIO_FREQUENCY_22K           ((uint32_t)22050U)
#define SAI_AUDIO_FREQUENCY_16K           ((uint32_t)16000U)
#define SAI_AUDIO_FREQUENCY_11K           ((uint32_t)11025U)
#define SAI_AUDIO_FREQUENCY_8K            ((uint32_t)8000U)
#define SAI_AUDIO_FREQUENCY_MCKDIV        ((uint32_t)0U)

#define SAI_OUTPUTDRIVE_DISABLED          SAI_OUTPUTDRIVE_DISABLE
#define SAI_OUTPUTDRIVE_ENABLED           SAI_OUTPUTDRIVE_ENABLE
#define SAI_MASTERDIVIDER_ENABLED         SAI_MASTERDIVIDER_ENABLE

#define HAL_SAI_StateTypeDef    uint32_t

#define HAL_SAI_STATE_RESET     AUDIO_STATUS_RESET
#define HAL_SAI_STATE_READY     AUDIO_STATUS_READY
#define HAL_SAI_STATE_BUSY      AUDIO_STATUS_BUSY
#define HAL_SAI_STATE_BUSY_TX   AUDIO_STATUS_BUSY_TX
#define HAL_SAI_STATE_BUSY_RX   AUDIO_STATUS_BUSY_RX
#define HAL_SAI_STATE_TIMEOUT   AUDIO_STATUS_TIMEOUT
#define HAL_SAI_STATE_ERROR     AUDIO_STATUS_ERROR

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai);

#define DFSDM_Channel_HandleTypeDef DFSDM_Channel_Handle_t
#define DFSDM_Filter_HandleTypeDef  DFSDM_Filter_Handle_t

#define HAL_DFSDM_ChannelInit       dfsdm_channel_init
#define HAL_DFSDM_ChannelDeInit     dfsdm_channel_deinit
#define HAL_DFSDM_FilterInit        dfsdm_filter_init
#define HAL_DFSDM_FilterDeInit      dfsdm_filter_deinit
#define HAL_DFSDM_FilterConfigRegChannel dfsdm_filter_config_reg
#define HAL_DFSDM_FilterConfigInjChannel dfsdm_filter_config_inj
#define HAL_DFSDM_FilterRegularStart_DMA dfsdm_filterRegularStart
#define HAL_DFSDM_FilterRegularStop_DMA  dfsdm_filterRegularStop

#define __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->state = DFSDM_CHANNEL_STATE_RESET)
#define __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->state = DFSDM_FILTER_STATE_RESET)

uint32_t HAL_GetTick(void);

#endif /* __STM32F7xx_HAL_H */

