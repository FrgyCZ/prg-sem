#include <complex.h>

#include "messages.h"

#ifndef __MAIN_MODULE_H__
#define __MAIN_MODULE_H__

void *main_module_thread(void *d);
void send_startup_message(message *msg);

#endif
