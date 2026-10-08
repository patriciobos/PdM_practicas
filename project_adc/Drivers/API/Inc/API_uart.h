#ifndef API_UART_H
#define API_UART_H

#include "API_delay.h"

bool_t uartInit(void);
void uartSendString(uint8_t *pstring);
void uartSendStringSize(uint8_t *pstring, uint16_t size);
bool_t uartReceiveByte(uint8_t *pbyte);
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

#endif
