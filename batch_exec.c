#include "batch_exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Báo cho trình biên dịch biết hàm này nằm ở file src.c, chỉ mượn để dùng
extern void handle_external_command(char *command);

void execute_bat_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Loi: Khong the mo file batch '%s'\n", filename);
        return;
    }

    char line[1024];
    printf("--- Bat dau chay file Batch: %s ---\n", filename);

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xóa ký tự xuống dòng của Windows
        line[strcspn(line, "\r\n")] = 0;

        // Bỏ qua dòng trống
        if (strlen(line) == 0) continue;

        printf("[Batch] Dang thuc thi: %s\n", line);
        
        // Gọi hàm thực thi tiến trình con từ src.c
        handle_external_command(line);
    }

    fclose(file);
    printf("--- Hoan thanh file Batch: %s ---\n", filename);
}