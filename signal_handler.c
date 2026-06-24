#include "signal_handler.h"
#include <stdio.h>
#include <windows.h>

// Kept private to this file only
static volatile int internal_ctrl_c_flag = 0;

BOOL WINAPI windows_ctrl_handler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        internal_ctrl_c_flag = 1;
        return TRUE; // Tell Windows we handled it; don't terminate the process
    }
    return FALSE;
}

void setup_signal_handler() {
    SetConsoleCtrlHandler(windows_ctrl_handler, TRUE);
}

int check_and_clear_ctrl_c() {
    if (internal_ctrl_c_flag) {
        internal_ctrl_c_flag = 0; // Reset it for the next command input
        return 1;
    }
    return 0;
}