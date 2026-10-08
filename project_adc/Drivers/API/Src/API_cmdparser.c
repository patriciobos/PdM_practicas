#include "API_cmdparser.h"
#include "API_uart.h"
#include "main.h"

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define CMD_BUFFER_SIZE 64U

static struct
{
    cmd_state_t state;
    char buffer[CMD_BUFFER_SIZE];
    uint8_t index;
    bool_t ledState;
} parser = {CMD_STATE_IDLE, {0}, 0U, false};

static void cmdSendString(const char *msg)
{
    if (msg != NULL)
    {
        uartSendString((uint8_t *)msg);
    }
}

static void cmdSendError(const char *message)
{
    cmdSendString(message);
}

static bool_t isSpace(unsigned char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

static void trimTrailingCRLF(char *str)
{
    size_t length = 0U;

    if (str != NULL)
    {
        while ((length < CMD_BUFFER_SIZE) && (str[length] != '\0'))
        {
            length++;
        }

        while ((length > 0U) && ((str[length - 1U] == '\r') || (str[length - 1U] == '\n')))
        {
            str[length - 1U] = '\0';
            length--;
        }
    }
}

static void trimLeadingSpaces(char **str)
{
    uint8_t skipped = 0U;

    if ((str != NULL) && (*str != NULL))
    {
        while ((skipped < CMD_BUFFER_SIZE) && (**str != '\0') && isSpace((unsigned char)**str))
        {
            (*str)++;
            skipped++;
        }
    }
}

static void toUpperCase(char *str)
{
    size_t index;

    if (str != NULL)
    {
        for (index = 0U; (index < CMD_BUFFER_SIZE) && (str[index] != '\0'); index++)
        {
            str[index] = (char)toupper((unsigned char)str[index]);
        }
    }
}

static void setLedState(bool_t newState)
{
    parser.ledState = newState;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                      (newState == true) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void sendHelp(void)
{
    cmdSendString("Available commands:\r\n");
    cmdSendString("HELP\r\n");
    cmdSendString("LED ON\r\n");
    cmdSendString("LED OFF\r\n");
    cmdSendString("LED TOGGLE\r\n");
    cmdSendString("STATUS\r\n");
    cmdSendString("BAUD?\r\n");
    cmdSendString("BAUD=115200\r\n");
}

static void handleLedCommand(char *argument)
{
    if (argument == NULL)
    {
        cmdSendError("ERROR: bad arguments\r\n");
    }
    else if (strcmp(argument, "ON") == 0)
    {
        setLedState(true);
        cmdSendString("LED is ON\r\n");
    }
    else if (strcmp(argument, "OFF") == 0)
    {
        setLedState(false);
        cmdSendString("LED is OFF\r\n");
    }
    else if (strcmp(argument, "TOGGLE") == 0)
    {
        setLedState(!parser.ledState);
        cmdSendString((parser.ledState == true) ? "LED is ON\r\n" : "LED is OFF\r\n");
    }
    else
    {
        cmdSendError("ERROR: bad arguments\r\n");
    }
}

static void handleBaudCommand(const char *token)
{
    unsigned long baud = 0UL;
    char *endPtr = NULL;

    baud = strtoul(token + 5, &endPtr, 10);
    if ((endPtr != token + 5) && (*endPtr == '\0') &&
        (baud >= 9600UL) && (baud <= 921600UL))
    {
        cmdSendString("BAUD updated\r\n");
    }
    else
    {
        cmdSendError("ERROR: bad arguments\r\n");
    }
}

static bool_t handleCommand(char *token)
{
    char *argument;
    bool_t handled = true;

    if (strcmp(token, "HELP") == 0)
    {
        sendHelp();
    }
    else if (strcmp(token, "LED") == 0)
    {
        argument = strtok(NULL, " \t");
        handleLedCommand(argument);
    }
    else if (strcmp(token, "STATUS") == 0)
    {
        cmdSendString((parser.ledState == true) ? "LED is ON\r\n" : "LED is OFF\r\n");
    }
    else if (strcmp(token, "BAUD?") == 0)
    {
        cmdSendString("BAUD=115200\r\n");
    }
    else if (strncmp(token, "BAUD=", 5U) == 0)
    {
        handleBaudCommand(token);
    }
    else
    {
        handled = false;
    }

    return handled;
}

static void processCommand(char *line)
{
    char upperLine[CMD_BUFFER_SIZE] = {0};
    char *token = NULL;
    size_t length = 0U;

    if (line != NULL)
    {
        trimTrailingCRLF(line);
        trimLeadingSpaces(&line);
        if ((*line != '\0') && (line[0] != '#') && !((line[0] == '/') && (line[1] == '/')))
        {
            length = strlen(line);
            if (length >= CMD_BUFFER_SIZE)
            {
                cmdSendError("ERROR: line too long\r\n");
            }
            else
            {
                memcpy(upperLine, line, length + 1U);
                toUpperCase(upperLine);
                token = strtok(upperLine, " \t");
                if ((token != NULL) && (handleCommand(token) == false))
                {
                    cmdSendError("ERROR: unknown command\r\n");
                }
            }
        }
    }
}

void cmdParserInit(void)
{
    parser.state = CMD_STATE_IDLE;
    parser.index = 0U;
    memset(parser.buffer, 0, sizeof(parser.buffer));
    parser.ledState = false;
    setLedState(false);
}

void cmdPoll(void)
{
    uint8_t c = 0U;

    if (uartReceiveByte(&c) == true)
    {
        switch (parser.state)
        {
            case CMD_STATE_IDLE:
                if ((c != '\r') && (c != '\n'))
                {
                    parser.buffer[0] = (char)c;
                    parser.index = 1U;
                    parser.state = CMD_STATE_RECEIVING;
                }
                break;

            case CMD_STATE_RECEIVING:
                if ((c == '\r') || (c == '\n'))
                {
                    parser.buffer[parser.index] = '\0';
                    processCommand(parser.buffer);
                    memset(parser.buffer, 0, sizeof(parser.buffer));
                    parser.index = 0U;
                    parser.state = CMD_STATE_IDLE;
                }
                else if (parser.index < (CMD_BUFFER_SIZE - 1U))
                {
                    parser.buffer[parser.index++] = (char)c;
                }
                else
                {
                    cmdSendError("ERROR: line too long\r\n");
                    memset(parser.buffer, 0, sizeof(parser.buffer));
                    parser.index = 0U;
                    parser.state = CMD_STATE_IDLE;
                }
                break;

            case CMD_STATE_LINE_READY:
            case CMD_STATE_ERROR:
                parser.state = CMD_STATE_IDLE;
                break;

            case CMD_STATE_COMMENT:
                if ((c == '\r') || (c == '\n'))
                {
                    parser.state = CMD_STATE_IDLE;
                }
                break;

            default:
                parser.state = CMD_STATE_IDLE;
                break;
        }
    }
}
