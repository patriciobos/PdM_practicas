#include "main.h"

#define SAMPLE_COUNT 100U

ADC_HandleTypeDef hadc1;

uint16_t adc_buffer[100];
uint16_t adc_min;
uint16_t adc_max;
uint32_t adc_sum;
uint32_t adc_average;

static void MX_ADC1_Init(void);
static void AcquireSamples(void);
static void ProcessSamples(void);

int main(void)
{
    HAL_Init();
    /* Se suspende SysTick: la espera de conversion usa HAL_MAX_DELAY. */
    HAL_SuspendTick();

    MX_ADC1_Init();
    AcquireSamples();
    ProcessSamples();

    while (1)
    {
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
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.NbrOfDiscConversion = 0;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

static void AcquireSamples(void)
{
    for (uint32_t i = 0; i < SAMPLE_COUNT; i++)
    {
        if (HAL_ADC_Start(&hadc1) != HAL_OK)
        {
            Error_Handler();
        }

        /*
         * La CPU comprueba repetidamente el estado del ADC hasta que termina
         * la conversion; mientras espera no ejecuta otra tarea de la aplicacion.
         */
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK)
        {
            Error_Handler();
        }

        adc_buffer[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }
}

static void ProcessSamples(void)
{
    adc_min = adc_buffer[0];
    adc_max = adc_buffer[0];
    adc_sum = 0U;

    for (uint32_t i = 0; i < SAMPLE_COUNT; i++)
    {
        if (adc_buffer[i] < adc_min)
        {
            adc_min = adc_buffer[i];
        }

        if (adc_buffer[i] > adc_max)
        {
            adc_max = adc_buffer[i];
        }

        adc_sum += adc_buffer[i];
    }

    adc_average = adc_sum / SAMPLE_COUNT;
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
