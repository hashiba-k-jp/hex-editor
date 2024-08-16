/**
* @file hex.c
* @brief This is the main file.
* @author HASHIBA Keishi
*/


#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>  // getopt(), write(), read(), STD{IN, OUT}_FILENO, access()
#include <err.h>    // warn(), err(),
#include <errno.h>  // errno,
#include <sys/ioctl.h>  // struct insize,
#include <termios.h>    // struct termios,
#include <stdlib.h> // atexit(),
#include <string.h> // memcmp(),
#include <fcntl.h> // open(), O_RDONLY,...

#include "header.h"

#include "config.h"
#include "terminalMode.h"
#include "keys.h"
#include "file.h"
#include "screen.h"

int main(int argc, char *argv[]){

    /* PHASE 1-1 : SAVE COOKED MODE AND SWITCH TO RAW MODE */
    // struct termios COOKED_MODE;
    atexit(fin_program);
    save_cooked_mode();
    switch_raw_mode();

    /* PHASE 1-2 : DISABLE BUFFER OF PRINTF (This makes termial to print anything immediately) */
    /* _IONBF : unbufferd */
    int vbuf_status = setvbuf(stdout, NULL, _IONBF, 0);
    if(vbuf_status != 0){
        err(vbuf_status, "Failed to disable buffer of standard output.");
    }

    /* PHASE 2 : INITIALIZE THE EDITOR */
    EDITOR EDITOR = init_editor();

    /* PHASE 3 : PARSE OPTIONS */
    char ch;
    while ((ch = getopt(argc, argv, "i:ch")) != -1) {
        /* if the optarg is NULL, getput will return '?' */
        switch (ch) {
            case 'c':
                EDITOR.isColored = true;
                break;
            case 'i':
                if(EDITOR.filename != NULL){
                    warn("[WARN] The former input file path will overwritten.\r\n");
                }
                EDITOR.filename = optarg;
                break;
            case 'h':
                print_usage();
                return 0;
            case '?':
                printf("\r\nInvalid option {%c}, option \x1b[1m\x1b[31m-h\x1b[m to show usage.\r\n", ch);
                break;
            default:
                err(-1, "Failed to load options...\r\n");
        }
    }

    /* PHASE 4 : CHECK WHETEHR THE INPUT FILE EXISTS OR NOT */
    /*
        - If the file exists ->
            - File reading is permitted -> just read the file.
            - NOT permitted             -> raise error.
        - does NOT exist                -> create new file.
    */
    if(EDITOR.filename != NULL){
        int file_check;
        /* Return 0 if the file exists, even if some permissions are denied. */
        file_check = access(EDITOR.filename, F_OK);
        switch (file_check) {
            case 0: /* file exists */
                if((file_check = access(EDITOR.filename, R_OK)) == -1){
                    err(file_check, "Cannot read the input file %s", EDITOR.filename);
                }else{
                    EDITOR.fd = open(EDITOR.filename, O_RDONLY);
                }
                break;
            case -1: /* file does NOT exist */
                /* S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH -> chmod 644 */
                EDITOR.fd = open(EDITOR.filename, O_CREAT, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
                break;
        }
    }

    /* PHASE 5 (optional) : READ THE FILE (if fd is NOT null.) */
    /*
        separete the file into several blocks.
    */
    /* at first, get file zise. (This may not needed.) */
    // const int BLOCK_SIZE = 0x800;
    if(EDITOR.fd != -1){
        EDITOR.filesize = lseek(EDITOR.fd, 0, SEEK_END);
        lseek(EDITOR.fd, 0, SEEK_SET); /* Set the offset at the head of the file. */
    }else{
        EDITOR.filesize = 0;
    }
    // printf("[debug] file size is %d\r\n", EDITOR.filesize);

    if(read_file(&EDITOR) == -1){
        err(-1, "Failed to read files or construct the data structres...");
    }

    init_screen(&EDITOR);

    char *header = header_msg(&EDITOR);
    char **footer = footer_msg(&EDITOR);
    // char **footer;

    char *msg;
    int input_c;
    int res_process;

    while(1){
        get_winsize(&EDITOR);
        print_screen(&EDITOR, header, footer);
        input_c = keyInput();
        // printf("[debug] input_c = %x\r\n", input_c);
        res_process = keyProcess(input_c, msg, &EDITOR, footer);
    }

    return 0;
}


enum move_dir{ DIR_NEXT, DIR_PREV };
void _move_cursor(EDITOR *EDITOR, int diff, int move_dir){
    T_DATA *tmp;
    if(move_dir == DIR_NEXT){
        for(int i = 0; i < diff; i++){
            if((tmp = EDITOR->cursor->point->next) == NULL){
                break;
            }else{
                EDITOR->cursor->point = tmp;
            }
        }
    }else if(move_dir == DIR_PREV){
        for(int i = 0; i < diff; i++){
            if((tmp = EDITOR->cursor->point->prev) == NULL){
                break;
            }else{
                EDITOR->cursor->point = tmp;
            }
        }
    }
};

int keyProcess(int c, char* msg, EDITOR *EDITOR, char **footer){
    int res = -99;
    if(EDITOR->cursor->editing){
        if(KEY0 <= c && c <= KEYF){ /* [0-9a-fA-F] */
            // EDITOR->cursor->point->data = EDITOR->cursor->point->data|(0x000F & c);
            EDITOR->cursor->point->data = EDITOR->cursor->point->data|(0x00FF & c);
            EDITOR->cursor->editing = false;
        }else{ /* other key input */
        }
    }else{
        if(KEY0 <= c && c <= KEYF){

            T_DATA *insert = malloc(sizeof(T_DATA));
            // insert->data = (c&0x000F)<<4;
            insert->data = (c&0x00FF)<<4;

            EDITOR->cursor->point->next->prev = insert;
            insert->next = EDITOR->cursor->point->next;
            EDITOR->cursor->point->next = insert;
            insert->prev = EDITOR->cursor->point->next;
            EDITOR->cursor->point = insert;
            EDITOR->cursor->editing = true;
        }else{
            switch (c) {
                case CTRL_Q:
                    exit(EXIT_SUCCESS);
                    break;
                case CTRL_S:
                    res = save_file(EDITOR);
            }

        }
    }

    switch (c) {
        case CTRL_Q:
            // [Todo] Save or not.
            if(EDITOR->isEdited){

            }else{
                exit(EXIT_SUCCESS);
            }
            break;

        case ARR_UP:
            _move_cursor(EDITOR, ((EDITOR->ws->ws_col-19) / 24)*8, DIR_PREV);
            break;
        case ARR_DW:
            _move_cursor(EDITOR, ((EDITOR->ws->ws_col-19) / 24)*8, DIR_NEXT);
            break;
        case ARR_RI:
            _move_cursor(EDITOR, 1, DIR_NEXT);
            break;
        case ARR_LE:
            _move_cursor(EDITOR, 1, DIR_PREV);
            break;
    }
    return 1;
}

int keyInput(){
    // input : key input
    // output : key number defined in keys.h (NOT ascii code)

    int read_len; // s means Status
    char seq[4];
    int c;

    while((read_len = read(STDIN_FILENO, &seq, 4)) == 0){
        // pass; This while waits any input.
    }
    if(read_len == -1){
        // failed to get input from key.
        exit(3);
    }
    // printf("[[%d %x %x %x %x]]", read_len, seq[0], seq[1], seq[2], seq[3]);

    if(read_len == 1){
        if(0x30 <= seq[0] && seq[0] <= 0x39){ /* 0-9 */
            c = seq[0]+0xD0;
            return c;
        }else if(0x61 <= seq[0] && seq[0] <= 0x66){ /* a-f */
            c = seq[0]+0xA9;
            return c;
        }else if(0x41 <= seq[0] && seq[0] <= 0x46){ /* A-F */
            c = seq[0]+0xC9;
            return c;
        }else{
            switch(seq[0]){
                case TAB:
                case CTRL_G:
                case CTRL_Q:
                case CTRL_S:
                case CTRL_X:
                case ESC:
                    return seq[0];
                    break;
                default:
                    printf("[debug]UNEXPECTED KEY\r\n");
                    return UNEXPECTED;
            }
        }
    }else if(read_len == 3){
        if(seq[0] == 0x1b && seq[1] == 0x5b){ // arrows
            switch(seq[2]){
                case 0x41:
                    // printf("[debug]ARR_UP↑ KEY\r\n");
                    return ARR_UP;
                    break;
                case 0x42:
                    // printf("[debug]ARR_DW↓ KEY\r\n");
                    return ARR_DW;
                    break;
                case 0x43:
                    // printf("[debug]ARR_RI→ KEY\r\n");
                    return ARR_RI;
                    break;
                case 0x44:
                    // printf("[debug]ARR_LE← KEY\r\n");
                    return ARR_LE;
                    break;
            }
        }

    }

    return 0;
}
