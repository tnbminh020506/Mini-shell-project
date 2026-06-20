#include "signal_handler.h"
#include <stdio.h>

void setup_signal_handlers() {
    // Đăng ký hàm CtrlHandler với hệ điều hành Windows
    // TRUE nghĩa là kích hoạt bộ lọc tín hiệu này
    if (SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        // Hàm này thành công thì Shell cha sẽ kiểm soát được CTRL+C
    } else {
        printf("Loi: Khong the thiet lap Ctrl Handler.\n");
    }
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        // Khi người dùng bấm CTRL+C
        case CTRL_C_EVENT: 
            printf("\n[Shell] Ban vua bam CTRL+C! Nhung Shell se khong bi tat.\n");
            
            // TRẢ VỀ TRUE: Báo với Windows là "Tao đã xử lý tín hiệu này rồi, đừng tắt ứng dụng của tao".
            // Đây chính là cách Shell cha sống sót.
            return TRUE; 

        // Các sự kiện khác (bấm nút X tắt cửa sổ, logoff...) thì để Windows tự xử lý
        default:
            return FALSE;
    }
}