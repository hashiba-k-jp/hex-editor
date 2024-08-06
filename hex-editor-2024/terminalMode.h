
// Here, the functions used to change terminal mode.
// STDIN_FILENO := 0

static struct termios orig_termios;

void switch_cooked_mode(void){
    if((errno = tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)) != 0){
        err(errno, "[ERROR] Failed to get current terminal mode.");
    }
    // printf("[DEBUG] successed to save the cooked mode!\n");
}

void save_cooked_mode(void){
    if((errno = tcgetattr(STDIN_FILENO, &orig_termios)) != 0){
        err(errno, "[ERROR] Failed to get current terminal mode.");
    }
    // printf("[DEBUG] successed to save the cooked mode!\n");
}

void switch_raw_mode(void){
    struct termios raw;
    cfmakeraw(&raw);
    if((errno = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)) != 0){
        err(errno, "[ERROR] Failed to change terminal to RAW mode.");
        //[TODO] ここでエラーが生じるとrawモードのまま異常終了してしまう。
    }
    // printf("[DEBUG] successed to change to raw mode!\n");
    // "return 0;" OR "exit(3)" called this switch_cooked_mode.
}
