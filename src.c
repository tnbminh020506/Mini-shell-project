#include<stdio.h>
#include<unistd.h>
#include <sys/wait.h>
#include <signal.h> // Để dùng SIGKILL, SIGSTOP, SIGCONT
#include <stdlib.h> // Để dùng hàm atoi() chuyển chuỗi (args[1]) thành số (PID)

#include <time.h>
#include <dirent.h>

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

        
        // Tránh lỗi nếu người dùng chỉ nhấn Enter (không nhập gì)
        if (idx == 0 || args[0] == NULL) {
            continue;
        }

        // PHẦN 1: XỬ LÝ LỆNH BUILT-IN (Các lệnh Shell tự làm)
        if (strcmp(args[0], "exit") == 0) {
            printf("Thoát Mini Shell...\n");
            break; // Thoát khỏi vòng lặp while(1)
        }
        
        if (strcmp(args[0], "List") == 0 || strcmp(args[0], "list") == 0) {
            list_processes(); // Gọi hàm thứ 3 bạn vừa viết
            continue; // Chạy xong thì quay lại đầu vòng lặp chờ lệnh mới
        }

        // ... (Code cũ xử lý lệnh List) ...

        // Lệnh Kill <pid> - Tiêu diệt tiến trình
        if (strcmp(args[0], "Kill") == 0 || strcmp(args[0], "kill") == 0) {
            if (args[1] == NULL) {
                printf("Lỗi: Thiếu PID. Cách dùng: Kill <pid>\n");
            } else {
                pid_t target_pid = atoi(args[1]); // Chuyển chuỗi người dùng nhập thành số
                // Gửi tín hiệu SIGKILL để ép buộc kết thúc
                if (kill(target_pid, SIGKILL) == 0) { 
                    printf("Đã tiêu diệt tiến trình %d\n", target_pid);
                    update_process_status(target_pid, "Killed"); // Cập nhật sổ tay
                } else {
                    perror("Lỗi khi Kill");
                }
            }
            continue;
        }

        // Lệnh Stop <pid> - Tạm dừng tiến trình
        if (strcmp(args[0], "Stop") == 0 || strcmp(args[0], "stop") == 0) {
            if (args[1] == NULL) {
                printf("Lỗi: Thiếu PID. Cách dùng: Stop <pid>\n");
            } else {
                pid_t target_pid = atoi(args[1]);
                // Gửi tín hiệu SIGSTOP để tạm dừng
                if (kill(target_pid, SIGSTOP) == 0) {
                    printf("Đã tạm dừng tiến trình %d\n", target_pid);
                    update_process_status(target_pid, "Stopped");
                } else {
                    perror("Lỗi khi Stop");
                }
            }
            continue;
        }

        // Lệnh Resume <pid> - Tiếp tục tiến trình đã dừng
        if (strcmp(args[0], "Resume") == 0 || strcmp(args[0], "resume") == 0) {
            if (args[1] == NULL) {
                printf("Lỗi: Thiếu PID. Cách dùng: Resume <pid>\n");
            } else {
                pid_t target_pid = atoi(args[1]);
                // Gửi tín hiệu SIGCONT để tiếp tục chạy
                if (kill(target_pid, SIGCONT) == 0) {
                    printf("Đã tiếp tục tiến trình %d\n", target_pid);
                    update_process_status(target_pid, "Running");
                } else {
                    perror("Lỗi khi Resume");
                }
            }
            continue;
        }


        // ----------------------------------------------------
        // Lệnh help - Hướng dẫn sử dụng Shell
        if (strcmp(args[0], "help") == 0) {
            printf("=== MINI SHELL HELP ===\n");
            printf("- List: In ra danh sach tien trinh ngam\n");
            printf("- Kill/Stop/Resume <pid>: Quan ly tien trinh ngam\n");
            printf("- date / time: Xem ngay gio he thong\n");
            printf("- dir: Liet ke cac file trong thu muc hien tai\n");
            printf("- path: Xem bien moi truong PATH\n");
            printf("- addpath <duong_dan>: Them duong dan vao bien PATH\n");
            printf("- exit: Thoat chuong trinh\n");
            continue;
        }

        // ----------------------------------------------------
        // Lệnh date và time - In thời gian hiện tại
        if (strcmp(args[0], "date") == 0 || strcmp(args[0], "time") == 0) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            printf("Thoi gian hien tai: %02d-%02d-%d %02d:%02d:%02d\n", 
                   tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
                   tm.tm_hour, tm.tm_min, tm.tm_sec);
            continue;
        }

        // ----------------------------------------------------
        // Lệnh dir - Liệt kê file trong thư mục hiện tại (Giống lệnh ls)
        if (strcmp(args[0], "dir") == 0) {
            DIR *d;
            struct dirent *dir;
            d = opendir("."); // Mở thư mục hiện tại (".")
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    // Bỏ qua các file ẩn (bắt đầu bằng dấu chấm)
                    if (dir->d_name[0] != '.') {
                        printf("%s  ", dir->d_name);
                    }
                }
                printf("\n");
                closedir(d);
            } else {
                perror("Loi doc thu muc");
            }
            continue;
        }

        // ----------------------------------------------------
        // Lệnh path - Xem đường dẫn biến môi trường PATH
        if (strcmp(args[0], "path") == 0) {
            char *current_path = getenv("PATH");
            if (current_path != NULL) {
                printf("PATH hien tai: %s\n", current_path);
            }
            continue;
        }

        // ----------------------------------------------------
        // Lệnh addpath - Thêm đường dẫn vào PATH
        if (strcmp(args[0], "addpath") == 0) {
            if (args[1] == NULL) {
                printf("Loi: Thieu duong dan. Cach dung: addpath <duong_dan>\n");
            } else {
                char *current_path = getenv("PATH");
                char new_path[2048]; // Mảng chứa chuỗi PATH mới
                
                // Gộp PATH cũ và đường dẫn mới (cách nhau bởi dấu ':' trên Linux)
                snprintf(new_path, sizeof(new_path), "%s:%s", current_path, args[1]);
                
                // Cập nhật lại biến PATH
                if (setenv("PATH", new_path, 1) == 0) {
                    printf("Da them '%s' vao PATH.\n", args[1]);
                } else {
                    perror("Loi addpath");
                }
            }
            continue;
        }


        // PHẦN 2: KIỂM TRA CHẠY NGẦM (Background Process - dấu '&')
        int is_background = 0;
        // Nếu tham số cuối cùng là dấu "&"
        if (idx > 0 && strcmp(args[idx - 1], "&") == 0) {
            is_background = 1;
            args[idx - 1] = NULL; // Xóa dấu '&' khỏi mảng để hàm execvp() không bị lỗi
        }

        // PHẦN 3: TẠO TIẾN TRÌNH MỚI CHO CÁC LỆNH BÊN NGOÀI
        pid_t pid = fork(); // Nhân bản tiến trình hiện tại thành tiến trình cha và con

        if (pid < 0) {
            // Lỗi khi tạo tiến trình
            perror("Lỗi fork");
        } 
        else if (pid == 0) {
            // ĐÂY LÀ TIẾN TRÌNH CON: Dùng để chạy lệnh bên ngoài
            // execvp sẽ thay thế nội dung tiến trình con bằng chương trình người dùng nhập
            if (execvp(args[0], args) == -1) {
                printf("Lỗi: Không tìm thấy lệnh '%s'\n", args[0]);
                exit(1); // Nếu lỗi, tiến trình con tự sát để không ảnh hưởng shell chính
            }
        } 
        else {
            // ĐÂY LÀ TIẾN TRÌNH CHA (Shell của bạn)
            if (is_background) {
                // Nếu chạy ngầm: Lưu PID của tiến trình con vào danh sách quản lý
                add_process(pid, args[0]); // Gọi hàm thứ 1 bạn vừa viết
                printf("[Chạy ngầm] Tiến trình '%s' có PID: %d\n", args[0], pid);
                // Không gọi waitpid() ở đây để shell không bị "treo" chờ đợi
            } else {
                // Nếu chạy bình thường (Foreground): Đợi tiến trình con chạy xong
                waitpid(pid, NULL, 0); 
            }
        }
    }

    return 0;
}

// cd "D:\Truong Nguyen Binh Minh\code\New folder\src"