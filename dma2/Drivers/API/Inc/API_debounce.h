#ifndef API_DEBOUNCE_H
#define API_DEBOUNCE_H

#include <stdbool.h>
#include "API_delay.h"

void debounceFSM_init(void);
void debounceFSM_update(void);
bool_t readKey(void);

#endif
