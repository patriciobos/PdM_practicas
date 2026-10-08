#ifndef API_CMDPARSER_H
#define API_CMDPARSER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    CMD_STATE_IDLE = 0,
    CMD_STATE_RECEIVING = 1,
    CMD_STATE_LINE_READY = 2,
    CMD_STATE_COMMENT = 3,
    CMD_STATE_ERROR = 4
} cmd_state_t;

void cmdParserInit(void);
void cmdPoll(void);

#endif
