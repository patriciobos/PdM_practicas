#include "API_debounce.h"
#include "main.h"

typedef enum
{
    BUTTON_UP,
    BUTTON_FALLING,
    BUTTON_DOWN,
    BUTTON_RAISING,
} debounceState_t;

static debounceState_t debounceState = BUTTON_UP;
static bool_t keyPressedFlag = false;
static delay_t debounceDelay;

static void buttonPressed(void)
{
    keyPressedFlag = true;
}

static void buttonReleased(void)
{
    keyPressedFlag = false;
}

void debounceFSM_init(void)
{
    debounceState = BUTTON_UP;
    keyPressedFlag = false;
    delayInit(&debounceDelay, 40U);
}

void debounceFSM_update(void)
{
    bool_t buttonState = (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);

    switch (debounceState)
    {
        case BUTTON_UP:
            if (buttonState)
            {
                debounceState = BUTTON_FALLING;
                delayInit(&debounceDelay, 40U);
            }
            break;

        case BUTTON_FALLING:
            if (delayRead(&debounceDelay))
            {
                if (buttonState)
                {
                    debounceState = BUTTON_DOWN;
                    buttonPressed();
                }
                else
                {
                    debounceState = BUTTON_UP;
                }
            }
            break;

        case BUTTON_DOWN:
            if (!buttonState)
            {
                debounceState = BUTTON_RAISING;
                delayInit(&debounceDelay, 40U);
            }
            break;

        case BUTTON_RAISING:
            if (delayRead(&debounceDelay))
            {
                if (!buttonState)
                {
                    debounceState = BUTTON_UP;
                    buttonReleased();
                }
                else
                {
                    debounceState = BUTTON_DOWN;
                }
            }
            break;

        default:
            debounceState = BUTTON_UP;
            break;
    }
}

bool_t readKey(void)
{
    bool_t pressed = false;

    if (keyPressedFlag)
    {
        keyPressedFlag = false;
        pressed = true;
    }

    return pressed;
}
