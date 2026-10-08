# ADC1 por polling

Etapa inicial de la práctica para la NUCLEO-F446RE. El firmware adquiere 100
muestras de ADC1, guarda cada resultado en `adc_buffer` y luego calcula
`adc_min`, `adc_max` y `adc_average`. Esta etapa no utiliza DMA ni interrupciones.

## Flujo

1. `HAL_Init()` prepara HAL. Se suspende el tick de SysTick para no depender de
   una interrupción de sistema; la espera de conversión usa `HAL_MAX_DELAY`.
2. `MX_ADC1_Init()` configura el pin analógico PA0 y ADC1 con resolución de
   12 bits, un canal y disparo por software.
3. `AcquireSamples()` repite 100 veces `HAL_ADC_Start()`,
   `HAL_ADC_PollForConversion()` y `HAL_ADC_GetValue()`. La CPU espera ocupada
   en el polling y no realiza otra tarea durante cada conversión.
4. `ProcessSamples()` recorre el buffer para obtener mínimo, máximo y suma de
   32 bits. El promedio es entero y descarta la parte fraccionaria.
5. Los resultados quedan en variables globales para inspeccionarlos con el
   depurador.

El polling sirve como línea base para compararlo más adelante con DMA: aquí la
CPU espera cada conversión y copia el dato al buffer; con DMA podrá atender
otras tareas mientras la transferencia llena el buffer.

## Potenciómetro

Conecta los extremos del potenciómetro a `3V3` y `GND`, y el cursor central a
`A0` (PA0, ADC1_IN0) del conector Arduino de la NUCLEO-F446RE. Mantén la señal
entre 0 V y 3,3 V; no conectes el cursor a 5 V.

## HAL ADC

La copia original del proyecto no incluía el módulo HAL de ADC. Se añadieron
los archivos ADC, ADCEx y LL ADC de STM32F4 HAL v1.8.1, manteniendo la versión
de HAL que ya utiliza el proyecto, y se habilitó `HAL_ADC_MODULE_ENABLED` en
`Core/Inc/stm32f4xx_hal_conf.h`.
