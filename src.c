#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h> // Quay trở lại với Windows nguyên bản

#include "signal_handler.h"
#include "batch_exec.h"
#include "builtin.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

int handle_input(char *input, char **args) {
    int idx = 0;
    args[idx] = strtok(input, " \t\r\n");
    
    while(args[idx] != NULL && idx < MAX_ARGS - 1) {
        args[++idx] = strtok(NULL, " \t\r\n");
    }
    args[idx] = NULL;
    return idx;
}

// Hàm chạy lệnh bằng WinAPI
void handle_external_command(char *command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Tạo tiến trình con bằng WinAPI
    if (!CreateProcess(
        NULL,           // Không dùng tên module
        command,        // Dòng lệnh cần thực thi (vd: "ping google.com")
        NULL,           // Process handle không kế thừa
        NULL,           // Thread handle không kế thừa
        FALSE,          // Cơ chế kế thừa handle = FALSE
        0,              // No creation flags
        NULL,           // Dùng chung biến môi trường của cha
        NULL,           // Dùng chung thư mục hiện hành của cha
        &si,            // Con trỏ tới cấu trúc STARTUPINFO
        &pi             // Con trỏ tới cấu trúc PROCESS_INFORMATION
    )) {
        printf("Lỗi hệ thống: Không thể thực thi lệnh '%s'\n", command);
        return;
    }

    // Chờ cho đến khi tiến trình con chạy xong (Tương đương waitpid bên Linux)
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Bắt buộc phải đóng Handle để tránh rò rỉ tài nguyên hệ thống Windows
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(int argc, char *argv[]) {
    // Chế độ 1: Chạy file Batch nếu truyền tham số đầu vào
    if (argc == 2) {
        execute_bat_file(argv[1]);
        return 0;
    }

    // Chế độ 2: Chạy tương tác trực tiếp
    setup_signal_handlers(); // Kích hoạt chặn CTRL+C bằng WinAPI
    printf("Mini-shell Windows đã sẵn sàng!\n");

    char input[MAX_INPUT]; 
    char input_backup[MAX_INPUT]; // Bản sao để giữ nguyên chuỗi cho CreateProcess
    char *args[MAX_ARGS];

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
            break; 
        }
        
        get_input[strcspn(get_input, "\t\r\n")] = '\0';
        if (strlen(get_input) == 0) continue;

        // Sao lưu lại chuỗi gốc trước khi bị hàm strtok băm nhỏ
        strcpy(input_backup, get_input);
        
        int idx = handle_input(input, args);
        
        if (idx > 0) {
            // Kiểm tra lệnh nội trú trước (exit, help...)
            if (execute_builtin(args) == 1) {
                continue;
            }
            
            // Nếu không phải, chuyển sang gọi tiến trình con của Windows để thực thi
            handle_external_command(input_backup);
        }
    }

    return 0;
}