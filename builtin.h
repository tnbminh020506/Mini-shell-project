#ifndef BUILTIN_H
#define BUILTIN_H

#include <windows.h>
// Trả về 1 nếu là lệnh nội trú, 0 nếu không phải
int execute_builtin(char **args);

void add_process(DWORD pid, HANDLE hProcess, char *name);

#endif