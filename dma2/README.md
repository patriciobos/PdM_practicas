# ADC1 con DMA circular y procesamiento por mitades

Tercera etapa de la práctica para la NUCLEO-F446RE. ADC1 convierte
continuamente la entrada PA0 (`A0`, ADC1_IN0), y DMA2 Stream 0, Channel 0,
transfiere cada resultado a un buffer circular de 200 muestras en RAM. Cada
mitad contiene 100 muestras.

## Flujo continuo

1. `MX_ADC1_Init()` configura ADC1 a 12 bits, con una secuencia de un canal,
   conversiones continuas iniciadas por software y solicitudes DMA continuas.
2. `MX_DMA_Init()` configura DMA2 Stream 0, Channel 0, de periférico a memoria,
   con incremento de memoria, tamaños half-word, modo circular y FIFO apagada.
   `__HAL_LINKDMA()` conecta el handle DMA con ADC1.
3. `HAL_ADC_Start_DMA()` se llama una sola vez con longitud `BUFFER_SIZE`.
   DMA escribe la mitad A y luego la mitad B; al final del buffer vuelve al
   inicio, sin que el programa tenga que reiniciar la adquisición.
4. La interrupción de half-transfer activa `first_half_ready` y la de
   transfer-complete activa `second_half_ready`. Sus callbacks solo comprueban
   ADC1 y establecen el flag correspondiente.
5. En `main()`, cuando un flag está activo, se limpia y se procesan las 100
   muestras de esa mitad. Mínimo, máximo y promedio se guardan en variables
   separadas para cada bloque. `processed_blocks` cuenta los bloques procesados.

```text
                  adc_buffer

          mitad A             mitad B
     ┌────────────────┬────────────────┐
     │ 100 muestras   │ 100 muestras   │
     └────────────────┴────────────────┘
             ▲                ▲
             │                │
       Half Complete     Complete
             │                │
             ▼                ▼
          CPU procesa      CPU procesa
           mitad A          mitad B

DMA recorre A → B → A → B ... mientras ADC convierte continuamente.
```

Al procesar una mitad, DMA está llenando la otra. Sin embargo, CPU y DMA operan
concurrentemente y el procesamiento de una mitad debe terminar antes de que DMA
vuelva a escribirla:

**tiempo de procesamiento < tiempo necesario para adquirir `BLOCK_SIZE` muestras**.

Si la CPU tarda demasiado, DMA puede sobrescribir una mitad antes de que se
termine de procesar. El resultado puede mezclar muestras de instantes distintos
o hacer que se pierdan bloques. Esta etapa solo explica esa condición; no agrega
una solución de sincronización más compleja.

## Comparación de las etapas

- **Etapa 1 — polling:** ADC → CPU → RAM. La CPU espera cada conversión, lee
  cada valor y lo almacena.
- **Etapa 2 — DMA normal:** ADC → DMA → RAM. Se transfiere un bloque y DMA se
  detiene al completarlo.
- **Etapa 3 — DMA circular:** ADC → DMA → buffer circular. La adquisición
  continúa indefinidamente y la CPU procesa una mitad mientras DMA escribe la
  otra.

La conexión del potenciómetro es entre `3V3` y `GND`, con el cursor central a
`A0` (PA0). La tensión de entrada debe permanecer entre 0 V y 3,3 V.
