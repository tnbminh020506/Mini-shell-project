#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <windows.h>

// Hàm thiết lập bộ bắt tín hiệu ngắt cho Shell
void setup_signal_handlers();

// Hàm callback xử lý khi người dùng nhấn CTRL+C
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#endif