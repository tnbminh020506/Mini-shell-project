#include<stdio.h>
#include<unistd.h>
#include<Windows.h>

#include <sys/types.h> // Thư viện chứa kiểu dữ liệu pid_t
#include <string.h>    // Thư viện để dùng hàm strcpy

#define MAX_PROCESSES 100

// Định nghĩa cấu trúc lưu thông tin của một tiến trình
typedef struct {
    pid_t pid;           // Process ID do hệ điều hành cấp phát
    char name[100];      // Tên lệnh (VD: "sleep 50", "gedit")
    char status[20];     // Trạng thái: "Running", "Stopped", "Killed"
} ProcessInfo;

// Khai báo mảng toàn cục để lưu danh sách các tiến trình
ProcessInfo process_list[MAX_PROCESSES];
int process_count = 0;   // Biến đếm số lượng tiến trình đang có trong danh sách

#define MAX_INPUT 1024
#define MAX_ARGS 64

int handle_input(char *input, char **args) {
    int idx = 0;
    printf("Handle input successful\n");
    args[idx] = strtok(input, " \t\r\n");
    
    while(args[idx] != NULL && idx < MAX_ARGS - 1) {
        args[++idx] = strtok(NULL, " \t\r\n");
    }
    args[idx] = NULL;
    return idx;
}

void handle_external_command(char *command) {
    
}


void add_process(pid_t pid, char *name) {
    if (process_count < MAX_PROCESSES) {
        process_list[process_count].pid = pid;
        strcpy(process_list[process_count].name, name);
        strcpy(process_list[process_count].status, "Running"); // Mặc định khi mới tạo là đang chạy
        process_count++;
    } else {
        printf("Cảnh báo: Danh sách tiến trình đã đầy!\n");
    }
}


void update_process_status(pid_t pid, const char *new_status) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            strcpy(process_list[i].status, new_status);
            return; // Cập nhật xong thì thoát vòng lặp
        }
    }
    printf("Không tìm thấy tiến trình có PID: %d\n", pid);
}


void list_processes() {
    printf("--------------------------------------------------\n");
    printf("%-10s | %-20s | %-10s\n", "PID", "Command Name", "Status");
    printf("--------------------------------------------------\n");
    
    if (process_count == 0) {
        printf("Khong co tien trinh nao dang chay ngam.\n");
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

int main() {
    char input[MAX_INPUT]; 
    char *args[MAX_ARGS];

    while(1) {
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        printf("%s $ ", cwd);
        fflush(stdout);

        char *get_input = fgets(input, MAX_INPUT, stdin);
        if(get_input == NULL) {
            perror("get input failed");
            continue;
        }
        get_input[strcspn(get_input, "\t\r\n")] = '\0';
        
        int idx = handle_input(input, args);
        for(int i = 0; i < idx; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        // Handle user request
    }

    return 0;
}

// cd "D:\Truong Nguyen Binh Minh\code\New folder\src"