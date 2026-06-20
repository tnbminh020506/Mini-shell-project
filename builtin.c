#include "builtin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int execute_builtin(char **args) {
    if (args[0] == NULL) {
        return 0; // Lệnh rỗng, trả về 0 để bỏ qua
    }

    // 1. Xử lý lệnh "exit"
    if (strcmp(args[0], "exit") == 0) {
        printf("Tam biet! Dang thoat Mini-shell...\n");
        exit(0); // Thoát chương trình thành công
    }

    // 2. Xử lý lệnh "help" (Bạn có thể bổ sung sau)
    if (strcmp(args[0], "help") == 0) {
        printf("--- Goc Huong Dan ---\n");
        printf("Cac lenh ho tro: exit, help, date...\n");
        return 1; // Báo cho src.c biết là đã xử lý xong
    }

    // Nếu không khớp với bất kỳ lệnh nội trú nào
    return 0; // Trả về 0 để src.c tự mang đi chạy fork()
}