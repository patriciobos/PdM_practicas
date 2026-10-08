#include "API_delay.h"
#include "main.h"

void delayInit(delay_t *delay, tick_t duration)
{
    if (delay != NULL)
    {
        delay->duration = duration;
        delay->startTime = 0;
        delay->running = false;
    }
}

bool_t delayRead(delay_t *delay)
{
    tick_t currentTick;
    bool_t elapsed = false;

    if (delay != NULL)
    {
        if (delay->running == false)
        {
            delay->startTime = HAL_GetTick();
            delay->running = true;
        }
        else
        {
            currentTick = HAL_GetTick();
            if ((currentTick - delay->startTime) >= delay->duration)
            {
                delay->running = false;
                elapsed = true;
            }
        }
    }

    return elapsed;
}

void delayWrite(delay_t *delay, tick_t duration)
{
    if (delay != NULL)
    {
        delay->duration = duration;
    }
}

bool_t delayIsRunning(delay_t *delay)
{
    bool_t running = false;

    if (delay != NULL)
    {
        running = delay->running;
    }

    return running;
}
