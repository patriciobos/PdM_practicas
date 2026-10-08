#include "main.h"

#define BLOCK_SIZE 100U
#define BUFFER_SIZE (2U * BLOCK_SIZE)

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

uint16_t adc_buffer[BUFFER_SIZE];

volatile uint8_t first_half_ready = 0U;
volatile uint8_t second_half_ready = 0U;

uint16_t first_minimum;
uint16_t first_maximum;
uint16_t first_average;
uint16_t second_minimum;
uint16_t second_maximum;
uint16_t second_average;
uint32_t processed_blocks = 0U;

static void MX_ADC1_Init(void);
static void MX_DMA_Init(void);
static void ProcessSamples(uint16_t *buffer,
                           uint32_t length,
                           uint16_t *minimum,
                           uint16_t *maximum,
                           uint16_t *average);

int main(void)
{
    HAL_Init();
    MX_ADC1_Init();
    MX_DMA_Init();

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, BUFFER_SIZE) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        if (first_half_ready != 0U)
        {
            first_half_ready = 0U;
            ProcessSamples(&adc_buffer[0],
                           BLOCK_SIZE,
                           &first_minimum,
                           &first_maximum,
                           &first_average);
            processed_blocks++;
        }

        if (second_half_ready != 0U)
        {
            second_half_ready = 0U;
            ProcessSamples(&adc_buffer[BLOCK_SIZE],
                           BLOCK_SIZE,
                           &second_minimum,
                           &second_maximum,
                           &second_average);
            processed_blocks++;
        }
    }
}

static void MX_ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.NbrOfDiscConversion = 0U;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1U;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1U;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_DMA_Init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_adc1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_adc1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_adc1.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0U, 0U);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        first_half_ready = 1U;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        second_half_ready = 1U;
    }
}

static void ProcessSamples(uint16_t *buffer,
                           uint32_t length,
                           uint16_t *minimum,
                           uint16_t *maximum,
                           uint16_t *average)
{
    uint32_t sum = 0U;

    *minimum = buffer[0];
    *maximum = buffer[0];

    for (uint32_t i = 0U; i < length; i++)
    {
        if (buffer[i] < *minimum)
        {
            *minimum = buffer[i];
        }

        if (buffer[i] > *maximum)
        {
            *maximum = buffer[i];
        }

        sum += buffer[i];
    }

    *average = (uint16_t)(sum / length);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    while (1)
    {
    }
}
#endif
