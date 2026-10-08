#include "API_uart.h"
#include "main.h"
#include <string.h>

static UART_HandleTypeDef huart2;

bool_t uartInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    bool_t initialized = false;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) == HAL_OK)
    {
        uartSendString((uint8_t *)"UART2 init OK: 115200 8N1\r\n");
        initialized = true;
    }

    return initialized;
}

void uartSendString(uint8_t *pstring)
{
    uint16_t len;

    if (pstring != NULL)
    {
        len = (uint16_t)strlen((const char *)pstring);
        uartSendStringSize(pstring, len);
    }
}

void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
    if ((pstring != NULL) && (size > 0U) && (size <= 256U))
    {
        if (HAL_UART_Transmit(&huart2, pstring, size, HAL_MAX_DELAY) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

bool_t uartReceiveByte(uint8_t *pbyte)
{
    HAL_StatusTypeDef status;
    bool_t received = false;

    if (pbyte != NULL)
    {
        status = HAL_UART_Receive(&huart2, pbyte, 1U, 1U);
        if (status == HAL_OK)
        {
            received = true;
        }
        else if (status != HAL_TIMEOUT)
        {
            Error_Handler();
        }
    }

    return received;
}

void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
    uint16_t i;
    bool_t received = true;

    if ((pstring != NULL) && (size > 0U) && (size <= 256U))
    {
        for (i = 0U; (i < size) && (received == true); i++)
        {
            if (uartReceiveByte(&pstring[i]) == false)
            {
                received = false;
            }
        }
    }
}
