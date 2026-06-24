#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <windows.h>

void setup_signal_handler();

int check_and_clear_ctrl_c();

#endif