/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HAL MSP.
 */
#include STM32_HAL_H
#include "omv_boardconfig.h"

/* GPIO struct */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} gpio_t;

/* DCMI GPIOs */
static const gpio_t dcmi_pins[] = {
    {DCMI_D0_PORT, DCMI_D0_PIN},
    {DCMI_D1_PORT, DCMI_D1_PIN},
    {DCMI_D2_PORT, DCMI_D2_PIN},
    {DCMI_D3_PORT, DCMI_D3_PIN},
    {DCMI_D4_PORT, DCMI_D4_PIN},
    {DCMI_D5_PORT, DCMI_D5_PIN},
    {DCMI_D6_PORT, DCMI_D6_PIN},
    {DCMI_D7_PORT, DCMI_D7_PIN},
    {DCMI_HSYNC_PORT, DCMI_HSYNC_PIN},
    {DCMI_VSYNC_PORT, DCMI_VSYNC_PIN},
    {DCMI_PXCLK_PORT, DCMI_PXCLK_PIN},
};

#define NUM_DCMI_PINS   (sizeof(dcmi_pins)/sizeof(dcmi_pins[0]))

void SystemClock_Config(void);

void HAL_MspInit(void)
{
    /* Set the system clock */
    SystemClock_Config();

    #if defined(OMV_DMA_REGION_BASE)
    __DSB(); __ISB();
    HAL_MPU_Disable();

    /* Configure the MPU attributes to disable caching DMA buffers */
    MPU_Region_InitTypeDef MPU_InitStruct;
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = OMV_DMA_REGION_BASE;
    MPU_InitStruct.Size             = OMV_DMA_REGION_SIZE;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    __DSB(); __ISB();
    #endif

    /* Enable I/D cache */
    #if defined(MCU_SERIES_F7) ||\
        defined(MCU_SERIES_H7)
    // Invalidate CPU cache
    SCB_InvalidateICache();
    SCB_InvalidateDCache();

    // Enable the CPU Cache
    SCB_EnableICache();
    SCB_EnableDCache();
    #endif

    /* Config Systick */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    /* Enable GPIO clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    #ifdef ENABLE_GPIO_BANK_F
    __GPIOF_CLK_ENABLE();
    #endif
    #ifdef ENABLE_GPIO_BANK_G
    __GPIOG_CLK_ENABLE();
    #endif
    #ifdef ENABLE_GPIO_BANK_H
    __GPIOH_CLK_ENABLE();
    #endif
    #ifdef ENABLE_GPIO_BANK_I
    __GPIOI_CLK_ENABLE();
    #endif

    #if defined(STM32F769xx)
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();
    __GPIOI_CLK_ENABLE();
    __GPIOJ_CLK_ENABLE();
    __GPIOK_CLK_ENABLE();

    /* Enable JPEG clock */
    __HAL_RCC_JPEG_CLK_ENABLE();
    #endif

    /* Enable DMA clocks */
    __DMA1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();

    #if defined(MCU_SERIES_H7)
    // MDMA clock
    __HAL_RCC_MDMA_CLK_ENABLE();
    #endif

    #if defined(OMV_HARDWARE_JPEG)
    // Enable JPEG clock
    __HAL_RCC_JPGDECEN_CLK_ENABLE();
    #endif

    /* Configure DCMI GPIO */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = DCMI_RESET_PIN;
    HAL_GPIO_Init(DCMI_RESET_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PWDN_PIN;
    HAL_GPIO_Init(DCMI_PWDN_PORT, &GPIO_InitStructure);

    #if defined(DCMI_FSIN_PIN)
    GPIO_InitStructure.Pin = DCMI_FSIN_PIN;
    HAL_GPIO_Init(DCMI_FSIN_PORT, &GPIO_InitStructure);
    #endif
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        /* Enable I2C clock */
        SCCB_CLK_ENABLE();

        /* Configure SCCB GPIOs */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_LOW;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStructure.Alternate = SCCB_AF;

        GPIO_InitStructure.Pin = SCCB_SCL_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin = SCCB_SDA_PIN;
        HAL_GPIO_Init(SCCB_PORT, &GPIO_InitStructure);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == SCCB_I2C) {
        SCCB_CLK_DISABLE();
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    #if (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
    if (htim->Instance == DCMI_TIM) {
        /* Enable DCMI timer clock */
        DCMI_TIM_CLK_ENABLE();

        /* Timer GPIO configuration */
        GPIO_InitTypeDef  GPIO_InitStructure;
        GPIO_InitStructure.Pin       = DCMI_TIM_PIN;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &GPIO_InitStructure);
    }
    #endif // (OMV_XCLK_SOURCE == OMV_XCLK_TIM)
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    /* DCMI clock enable */
    __DCMI_CLK_ENABLE();

    /* DCMI GPIOs configuration */
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF13_DCMI;

    /* Enable VSYNC EXTI */
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pin  = DCMI_VSYNC_PIN;
    HAL_GPIO_Init(DCMI_VSYNC_PORT, &GPIO_InitStructure);

    /* Configure DCMI pins */
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    for (int i=0; i<NUM_DCMI_PINS; i++) {
        GPIO_InitStructure.Pin = dcmi_pins[i].pin;
        HAL_GPIO_Init(dcmi_pins[i].port, &GPIO_InitStructure);
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    #if defined(LEPTON_SPI)
    if (hspi->Instance == LEPTON_SPI) {
        LEPTON_SPI_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pull      = GPIO_PULLUP;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Alternate = LEPTON_SPI_AF;
        GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

        GPIO_InitStructure.Pin       = LEPTON_SPI_SCLK_PIN;
        HAL_GPIO_Init(LEPTON_SPI_SCLK_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin       = LEPTON_SPI_MISO_PIN;
        HAL_GPIO_Init(LEPTON_SPI_MISO_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin       = LEPTON_SPI_MOSI_PIN;
        HAL_GPIO_Init(LEPTON_SPI_MOSI_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.Pin       = LEPTON_SPI_SSEL_PIN;
        HAL_GPIO_Init(LEPTON_SPI_SSEL_PORT, &GPIO_InitStructure);
    }
    #endif
}

void HAL_MspDeInit(void)
{

}

#define DACx                            DAC1
#define DACx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA1_CLK_ENABLE()

#define DACx_CLK_ENABLE()               __HAL_RCC_DAC12_CLK_ENABLE()
#define DACx_FORCE_RESET()              __HAL_RCC_DAC12_FORCE_RESET()
#define DACx_RELEASE_RESET()            __HAL_RCC_DAC12_RELEASE_RESET()

/* Definition for DACx's DMA_STREAM */
#define DACx_DMA_INSTANCE               DMA1_Stream5

/* Definition for DACx's NVIC */
#define DACx_DMA_IRQn                   DMA1_Stream5_IRQn
#define DACx_DMA_IRQHandler             DMA1_Stream5_IRQHandler

void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  static DMA_HandleTypeDef  hdma_dac1;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock ****************************************/
  DACx_CHANNEL_GPIO_CLK_ENABLE();
  /* DAC Periph clock enable */
  DACx_CLK_ENABLE();
  /* DMA1 clock enable */
  DMAx_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##########################################*/
  /* Set the parameters to be configured for DACx_DMA_STREAM */
  hdma_dac1.Instance = DACx_DMA_INSTANCE;

  hdma_dac1.Init.Request  = DMA_REQUEST_DAC1;

  hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
  hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_dac1.Init.Mode = DMA_CIRCULAR;
  hdma_dac1.Init.Priority = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&hdma_dac1);

  /* Associate the initialized DMA handle to the DAC handle */
  __HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac1);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* Enable the DMA1_Stream5 IRQ Channel */
  HAL_NVIC_SetPriority(DACx_DMA_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DACx_DMA_IRQn);

}


