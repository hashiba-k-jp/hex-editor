
#include "func.h"
// Here, the functions which are run only once.

typedef struct {
    char *filename;
    bool isColored;
    struct winsize ws; // defined in <sys/ioctl.h>
    struct winsize cpos; // Cursor POSition
    bool isEdited;
    // struct termios COOKED_MODE;
    // struct termios RAW_MODE;
}EDITOR;

EDITOR init_editor(void){
    EDITOR EDITOR;

    EDITOR.filename     = NULL;
    EDITOR.isColored    = NULL;
    EDITOR.isEdited     = false;

    int c_row = 0;
    int c_col = 0;

    int len;

    char buf[32];

    char *cpos;
    if((len = write(STDOUT_FILENO, "\x1b[6n", 4)) == -1){
        err(len, "Failed to get current cursor position.");
    }
    // cpos has format or "ESC[#;#R".

    int i = 0;
    int read_len;
    for (i = 0; i < len; i++){
        printf("[DEBUG] c_row is %d, %d\n", c_row, i);
        if((read_len = read(STDOUT_FILENO, buf+i, 1)) != 1){
            errno = read_len;
            err(errno, "Failed to parse string to int (current cursor row position).");
        }
        // if(buf[i] >= 0x30 && buf[i] <= 0x39){
        //     c_row *= 0x0a;
        //     c_row += (buf[i] - 0x30);
        // }else if(buf[i] == ';'){
        //     break;
        // }else{
        //     i++;
        // }
    }
    printf("[DEBUG] c_row is %d\n", c_row);
    // for (i = 0; ; i++){
    //     if((read_len = read(STDIN_FILENO, buf+i, 1)) != 1){
    //         errno = read_len;
    //         err(errno, "Failed to parse string to int (current cursor row position).");
    //     }
    //     if(buf[i] >= 0x30 && buf[i] <= 0x39){
    //         c_col *= 0x0a;
    //         c_col += (buf[i] - 0x30);
    //     }else if(buf[i] == ';'){
    //         break;
    //     }else{
    //         i++;
    //     }
    // }

    // printf("[debug] %d, %d\n\n", c_row, c_col);


    int succ_get_winsize;
    if ((succ_get_winsize = ioctl(1, TIOCGWINSZ, &EDITOR.ws)) != 0){
        err(succ_get_winsize, "[ERROR] Failed to get window size.");
    }
    return EDITOR;
}

// void init_screen(void){
//     initscr();
// }

void print_usage(void){
    printf("usage:\n");
}

void fin_program(void){
    switch_cooked_mode();
}
