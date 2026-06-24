#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <string.h>  
#include <unistd.h>

#include "builtin.h"
#include "signal_handler.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

static volatile int ctrl_c_pressed = 0;

int handle_input(char *input, char **args) {
    int idx = 0;
    // printf("Handle input successful\n");
    args[idx] = strtok(input, " \t\r\n");
    
    while(args[idx] != NULL && idx < MAX_ARGS - 1) {
        args[++idx] = strtok(NULL, " \t\r\n");
    }
    args[idx] = NULL;
    return idx;
}

void handle_external_command(char **args, int is_background) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD creation_flag = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (is_background) {
        // Sleep(3000);
        // Background mode settings
        creation_flag = CREATE_NO_WINDOW | DETACHED_PROCESS;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    } else {
        // Foreground mode settings
        creation_flag = 0; 
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
    }

    char command[1024];
    int i = 0;
    while(args[i] != NULL) {
        strcat(command, args[i]);
        strcat(command, " ");
        i++;
    }
    // Giả sử args_string là chuỗi lệnh bạn muốn chạy (ví dụ: "cmd.exe /c dir")

    if (CreateProcessA(
            NULL,           // Ứng dụng tên file (để NULL nếu truyền vào command line)
            command,        // Tham số lệnh
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            creation_flag,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi             // Pointer to PROCESS_INFORMATION structure
        )) {
        
        if(!is_background) {
            WaitForSingleObject(pi.hProcess, INFINITE);            
            // Đóng các handle sau khi dùng xong
            CloseHandle(pi.hProcess);            
        }
        else {
            add_process((DWORD)pi.dwProcessId, pi.hProcess, command);
        }

        CloseHandle(pi.hThread);
        
        printf("Child process completed.\n");
    } else {
        printf("Error: Cannot create child process (Error code: %lu)\n", GetLastError());
    }
}

int main() {
    char input[MAX_INPUT]; 
    char *args[MAX_ARGS];

    setup_signal_handler();

    printf("=========================================================\n");
    printf("=========== Welcome to our tiny/mini shell! =============\n");
    printf("=========================================================\n");

    rewind(stdin);
    while(1) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s $ ", cwd);
        } else {
            printf("minishell $ ");
        }
        fflush(stdout);

        char *get_input = fgets(input, MAX_INPUT, stdin);
        if(get_input == NULL) {
            Sleep(10);
            if (check_and_clear_ctrl_c()) {
                clearerr(stdin);
                printf("\nShell terminated due to user's Ctrl+C signal\n");
                continue;     // Safely loop back to draw the new prompt
            }

            // If it wasn't Ctrl+C, check if it's a true exit signal (like Ctrl+Z)
            if (feof(stdin)) {
                printf("\nExiting shell...\n");
                break; 
            }

            clearerr(stdin);
            continue;   
        }
        get_input[strcspn(get_input, "\t\r\n")] = '\0';
        if(strlen(get_input) == 0) continue;

        int idx = handle_input(input, args);

        if (idx == 0 || args[0] == NULL) {
            continue;
        }

        int is_background = 0;
        if (idx > 0) {
            if (execute_builtin(args) == 1) {
                continue;
            }
            if(strcmp(args[idx - 1], "&") == 0) {
                is_background = 1;
                args[idx - 1] = NULL;
            }
            handle_external_command(args, is_background);
        }
    }

    return 0;
}

// cd "D:\Truong Nguyen Binh Minh\code\Mini shell"
// gcc main.c builtin.c signal_handler.c -o minishell.exe