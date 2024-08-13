
#include "func.h"
// Here, the functions which are run only once.


struct editor{
    char *filename;
    int fd;             // file descriptor for the input file.
    bool isColored;
    struct winsize *ws;  // defined in <sys/ioctl.h>
    int curr_row;       // CURRent row
    int curr_col;       // CURRent row
    int filesize;
    bool isEdited;
    int line_size;

    // struct datablock *head_block;
    struct t_data *head_data;
    struct t_cursor *cursor;

    int curr_footer_status;
    int prev_footer_status;

    unsigned int msgStatus;

};

// typedef struct datablock{
//     struct datablock *next_block;
//     struct datablock *prev_block;
//     struct t_data *head;
//     struct t_data *tail;
//     int block_size;
// }DATABLOCK;

// enum EDITOR_STATUS{
//     EDITING,
//     SAVING,
// };

EDITOR init_editor(void){
    EDITOR EDITOR;

    EDITOR.filename     = NULL;
    EDITOR.fd           = -1;
    EDITOR.ws           = malloc(sizeof(struct winsize));
    EDITOR.isColored    = NULL;
    EDITOR.filesize     = -1;
    EDITOR.isEdited     = false;
    EDITOR.curr_row     = 0;
    EDITOR.curr_col     = 0;
    EDITOR.msgStatus    = 0b00000000;

    int len = 0;

    char buf[32];
    int i = 0;


    if((len = write(STDOUT_FILENO, "\x1b[6n", 4)) == 4){ // successed to get cursor position.
        // e.g. [23;3R
        while (i < sizeof(buf) - 1) {
            if(read(STDIN_FILENO, buf+i, 1) != 1){
                err(-1, "Unexpected character.");
            }
            if(buf[i] == 'R'){
                 break;
            }
            i++;
        }
        buf[i] = '\0';
        if (buf[0] != '\x1b' || buf[1] != '['){
            err(buf[0], "Invalid value of current cursor position.");
        }
        if(sscanf(buf+2,"%d;%d", &EDITOR.curr_row, &EDITOR.curr_col) != 2){
            err(-1, "Failed to parse the value of current cursor position.");
        }
    }else{
        err(-1, "Failed to get current cursor position.");
    }

    // printf("[debug] %d, %d\r\n", EDITOR.curr_row, EDITOR.curr_col);

    int succ_get_winsize;
    if ((succ_get_winsize = ioctl(1, TIOCGWINSZ, EDITOR.ws)) != 0){
        err(succ_get_winsize, "[ERROR] Failed to get window size.");
    }
    return EDITOR;
}

void print_usage(void){
    printf("usage:\n");
}

void fin_program(){
    // printf("\x1B[0;0H");
    printf("\x1B[%d;%dH", 40, 0);
    // printf("")
    // printf("\x1B[%dT", 30);
    switch_cooked_mode();
}