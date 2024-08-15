static struct termios cooked_termios;

// "return 0;" of main() OR "exit(3)" called this switch_cooked_mode.
void switch_cooked_mode(void){
    if((errno = tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked_termios)) != 0){
        err(errno, "[ERROR] Failed to get current terminal mode.");
    }
    // printf("[DEBUG] successed to save the cooked mode!\n");
}

void save_cooked_mode(void){
    if((errno = tcgetattr(STDIN_FILENO, &cooked_termios)) != 0){
        err(errno, "[ERROR] Failed to get current terminal mode.");
    }
    // printf("[DEBUG] successed to save the cooked mode!\n");
}

void switch_raw_mode(void){
    struct termios raw;
    cfmakeraw(&raw); // giving a “raw I/O path”


    raw.c_lflag = 0; // [todo] It seems odd...
    // raw.c_lflag &= ~(ICANON | ECHO);

    /* disable ICANON (0x100); raw mode. */
    /* disable ECHO (0x8); "loaclecho mode" displays any strings sent to other computers (in this case, other file). */

    if((errno = tcsetattr(STDIN_FILENO, TCSANOW, &raw)) != 0){
        err(errno, "[ERROR] Failed to change terminal to RAW mode.");
        //[TODO] ここでエラーが生じるとrawモードのまま異常終了してしまう。
    }
    // printf("[DEBUG] successed to change to raw mode!\n");
}
