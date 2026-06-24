#include "builtin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MAX_INPUT 1024
#define MAX_ARGS 64

typedef LONG (NTAPI *pfnNtSuspendProcess)(HANDLE);
typedef LONG (NTAPI *pfnNtResumeProcess)(HANDLE);

// Định nghĩa cấu trúc lưu thông tin của một tiến trình
typedef struct {
    DWORD pid;           // Process ID do hệ điều hành cấp phát
    HANDLE hProcess;
    char name[100];      // Tên lệnh (VD: "sleep 50", "gedit")
    char status[20];     // Trạng thái: "Running", "Stopped", "Killed"
} ProcessInfo;

// Khai báo mảng toàn cục để lưu danh sách các tiến trình
ProcessInfo process_list[MAX_PROCESSES];
int process_count = 0;   // Biến đếm số lượng tiến trình đang có trong danh sách


void add_process(DWORD pid, HANDLE hProcess, char *name) {
    if (process_count < MAX_PROCESSES) {
        process_list[process_count].pid = pid;
        process_list[process_count].hProcess = hProcess;
        strcpy(process_list[process_count].name, name);
        strcpy(process_list[process_count].status, "Running"); // Mặc định khi mới tạo là đang chạy
        process_count++;
    } else {
        printf("The maximum number of processes has reached\n");
    }
}


void update_process_status(DWORD pid, const char *new_status) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            strcpy(process_list[i].status, new_status);
            return; // Cập nhật xong thì thoát vòng lặp
        }
    }
    printf("Cannot find process with PID: %d\n", pid);
}

void check_process() {
    DWORD exitCode;

    for (int i = 0; i < process_count; i++) {
        if (strcmp(process_list[i].status, "Running") == 0 || 
            strcmp(process_list[i].status, "Stopped") == 0) {
            
            // Validate that the handle isn't NULL or invalid before checking
            if (process_list[i].hProcess != NULL) {
                if (GetExitCodeProcess(process_list[i].hProcess, &exitCode)) {
                    
                    // 🔍 DEBUG LINE: Uncomment this to see what Windows is returning in real-time
                    // printf("[Debug] PID %lu Exit Code: %lu\n", process_list[i].pid, exitCode);

                    if (exitCode != STILL_ACTIVE) { // 259 is the literal value of STILL_ACTIVE
                        strcpy(process_list[i].status, "Finished");
                        
                        // Clean up the kernel resource now that we are completely done tracking it
                        CloseHandle(process_list[i].hProcess);
                        process_list[i].hProcess = NULL; 
                        
                        printf("\n[Background process notification] Process execution '%s' (PID: %lu) completes.\n", 
                               process_list[i].name, process_list[i].pid);
                    }
                    else {
                        printf("\n[Background process notification] Process execution '%s' (PID: %lu) is still running.\n", 
                               process_list[i].name, process_list[i].pid);
                    }
                } 
                else {
                    // If GetExitCodeProcess returns FALSE, the handle you passed is dead/invalid
                    printf("[Error] Process PID %lu invalid.\n", 
                           process_list[i].pid);
                }
            }
        }
    }
}

void list_processes() {
    check_process();
    printf("--------------------------------------------------\n");
    printf("%-10s | %-20s | %-10s\n", "PID", "Command Name", "Status");
    printf("--------------------------------------------------\n");
    
    if (process_count == 0) {
        printf("No process is running in the background mode.\n");
    } else {
        for (int i = 0; i < process_count; i++) {
            printf("%-10d | %-20s | %-10s\n", 
                   process_list[i].pid, 
                   process_list[i].name, 
                   process_list[i].status);
        }
    }
    printf("--------------------------------------------------\n");
}

int execute_builtin(char **args) {
    if (args[0] == NULL) {
        return 0; 
    }


    if (strcmp(args[0], "Exit") == 0) {
        printf("Good bye. Terminating tiny shell!\n");
        exit(0); 
    }


    if (strcmp(args[0], "List") == 0) {
        list_processes();
        return 1;
    }


    if (strcmp(args[0], "Kill") == 0) {
        if (args[1] == NULL) {
            printf("No process is provided. Follow syntax: Kill <pid>\n");
        } else {
            DWORD target_pid = (DWORD)atoi(args[1]);
            
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, target_pid);
            
            if (hProcess != NULL) {
                if (TerminateProcess(hProcess, 0)) {
                    printf("Process terminated succesful %lu\n", target_pid);
                    update_process_status((DWORD)target_pid, "Killed"); // Cập nhật sổ tay của bạn
                } else {
                    printf("Error: Process %lu cannot be terminated\n", target_pid);
                }
                CloseHandle(hProcess);
            } else {
                printf("Error: Cannot track process %lu (Error code: %lu)\n", target_pid, GetLastError());
            }
        }
        return 1;
    }


    if (strcmp(args[0], "Stop") == 0) {
        if (args[1] == NULL) {
            printf("No process is provided. Follow syntax: Stop <pid>\n");
        } else {
            DWORD target_pid = (DWORD)atoi(args[1]);
            
            HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE, FALSE, target_pid);
            
            if (hProcess != NULL) {
                typedef LONG (NTAPI *pfnNtSuspendProcess)(HANDLE);
                HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
                pfnNtSuspendProcess NtSuspendProcess = (pfnNtSuspendProcess)GetProcAddress(hNtDll, "NtSuspendProcess");
                
                if (NtSuspendProcess != NULL && NtSuspendProcess(hProcess) >= 0) {
                    printf("Process stopped successful %d\n", target_pid);
                    update_process_status((DWORD)target_pid, "Stopped");
                } else {
                    printf("Error: Process %d cannot be stoped\n", target_pid);
                }
                CloseHandle(hProcess);
            } else {
                printf("Error: Cannot track process %lu (Error code: %lu)\n", target_pid, GetLastError());
            }
        }
        return 1;
    }


    if (strcmp(args[0], "Resume") == 0) {
        if (args[1] == NULL) {
            printf("No process is provided. Follow syntax: Resume <pid>\n");
        } else {
            DWORD target_pid = (DWORD)atoi(args[1]);
            
            HANDLE hProcess = OpenProcess(0x0800, FALSE, target_pid);
            
            if (hProcess != NULL) {
                HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
                pfnNtResumeProcess NtResumeProcess = (pfnNtResumeProcess)GetProcAddress(hNtDll, "NtResumeProcess");
                
                if (NtResumeProcess != NULL && NtResumeProcess(hProcess) >= 0) {
                    printf("Process resumed successful %lu\n", target_pid);
                    update_process_status((DWORD)target_pid, "Running");
                } else {
                    printf("Error: Process %lu cannot be resumed\n", target_pid);
                }
                CloseHandle(hProcess);
            } else {
                printf("Error: Cannot track process %lu (Error code: %lu)\n", target_pid, GetLastError());
            }
        }
        return 1;
    }

    // ----------------------------------------------------
    // Lệnh help - Hướng dẫn sử dụng Shell
    if (strcmp(args[0], "Help") == 0) {
        printf("=== MINI SHELL HELP ===\n");
        printf("- List: In ra danh sach tien trinh ngam\n");
        printf("- Kill/Stop/Resume <pid>: Quan ly tien trinh ngam\n");
        printf("- Date: Xem ngay gio he thong\n");
        printf("- Dir: Liet ke cac file trong thu muc hien tai\n");
        printf("- Path: Xem bien moi truong PATH\n");
        printf("- Addpath <duong_dan>: Them duong dan vao bien PATH\n");
        printf("- Exit: Thoat chuong trinh\n");
        return 1;
    }


    if (strcmp(args[0], "Date") == 0) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("Thoi gian hien tai: %02d-%02d-%d %02d:%02d:%02d\n", 
                tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
                tm.tm_hour, tm.tm_min, tm.tm_sec);
        return 1;
    }


    if (strcmp(args[0], "Dir") == 0) {
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_name[0] != '.') {
                    printf("%s  ", dir->d_name);
                }
            }
            printf("\n");
            closedir(d);
        } else {
            perror("Error while searching directory");
        }
        return 1;
    }


    if (strcmp(args[0], "Path") == 0) {
        char *current_path = getenv("PATH");
        if (current_path != NULL) {
            printf("PATH hien tai: %s\n", current_path);
        }
        return 1;
    }

    if (strcmp(args[0], "Addpath") == 0) {
        if (args[1] == NULL) {
            printf("Path not found. Follow syntax: addpath <duong_dan>\n");
        } else {
            char *current_path = getenv("PATH");
            // printf("%s\n", current_path);
            char new_path[2048];
            
            char full_target_path[2048] = "";
            int i = 1;

            // 1. Concatenate args[1] through the last argument index back together with spaces
            while (args[i] != NULL) {
                strcat(full_target_path, args[i]);
            
                // If there's another argument after this one, append a space back
                if (args[i + 1] != NULL) {
                    strcat(full_target_path, " ");
                }
                i++;
            }

            if (current_path != NULL) {
                // Join using the Windows semicolon ';' separator
                snprintf(new_path, sizeof(new_path), "%s;%s", current_path, full_target_path);
            } else {
                snprintf(new_path, sizeof(new_path), "%s", full_target_path);
            }
            
            if (SetEnvironmentVariableA("PATH", new_path)) {
                printf("Added '%s' to PATH.\n", args[1]);
            } else {
                printf("Error: %s cannot be added to PATH.\n", args[1]);
            }
        }
        return 1;
    }

    return 0; 
}