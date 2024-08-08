/**
* @file hex.c
* @brief This is the main file.
* @author HASHIBA Keishi
*/


#include <stdio.h>
#include <stdbool.h>

// #include <stdint.h>
// #include <string.h>
// #include <ctype.h>
// #include <getopt.h>

#include <unistd.h>  // getopt(), write(), read(), STD{IN, OUT}_FILENO, access()
#include <err.h>    // warn(), err(),
#include <errno.h>  // errno,
#include <sys/ioctl.h>  // struct insize,
#include <termios.h>    // struct termios,
#include <stdlib.h> // atexit(),
#include <string.h> // memcmp(),
#include <fcntl.h> // open(), O_RDONLY,...
// #include <ncurses.h>    // ncurses:initscr(),


#include "config.h"
#include "terminalMode.h"
#include "keys.h"

int keyProcess(int c, char* msg, EDITOR EDITOR);
int keyInput();

int main(int argc, char *argv[]){

    /* PHASE 1 : SAVE COOKED MODE AND SWITCH TO RAW MODE */
    // struct termios COOKED_MODE;
    atexit(fin_program);
    save_cooked_mode();
    switch_raw_mode();

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

    /* PHASE 4 (optional) : CHECK WHETEHR THE INPUT FILE EXISTS OR NOT */
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
                };
                EDITOR.fd = open(EDITOR.filename, O_RDONLY);
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
    /* get file zise. */
    const int BLOCK_SIZE = 0x800;
    int filesize = -1;
    if(EDITOR.fd != -1){
        filesize = lseek(EDITOR.fd, 0, SEEK_END);
    }else{
        filesize = 0;
    }
    printf("[debug] file size is %d\r\n", filesize);






    char *msg;
    int input_c;
    int res_process;

    while(1){
        input_c = keyInput();
        // printf("[DEBUG:input_c]%d\r\n", input_c);
        res_process = keyProcess(input_c, msg, EDITOR);
    }

    return 0;
}

int keyProcess(int c, char* msg, EDITOR EDITOR){
    switch (c) {
        case CTRL_Q:
            // [Todo] Save or not.
            if(EDITOR.isEdited){

            }else{
                exit(EXIT_SUCCESS);
            }
            break;

    }
    return 1;
}

int keyInput(){
    // input : key input
    // output : key number defined in keys.h (NOT ascii code)

    int read_len; // s means Status
    char c, seq[4];

    while((read_len = read(STDIN_FILENO, &seq, 4)) == 0){
        // pass; This while waits any input.
    }
    if(read_len == -1){
        // failed to get input from key.
        exit(3);
    }
    // printf("[[%d %x %x %x %x]]", read_len, seq[0], seq[1], seq[2], seq[3]);

    if(read_len == 1){
        if(0x20 <= seq[0] && seq[0] <= 0x7F){
            printf("%c\n", seq[0]);
            fflush(stdout); // THIS IS NECESSARY IN ORDER TO PRINT IMMEDIATELY
            return seq[0];
        }else{
            switch(seq[0]){
                case TAB:
                    // printf("[debug]TAB\r\n");
                    // return seq[0];
                case CTRL_G:
                    // printf("[debug]CTRL_G\r\n");
                    // return seq[0];
                case CTRL_Q:
                    // printf("[debug]CTRL_Q\r\n");
                    // return seq[0];
                case CTRL_S:
                    // printf("[debug]CTRL_S\r\n");
                    // return seq[0];
                case CTRL_X:
                    // printf("[debug]CTRL_X\r\n");
                    // return seq[0];
                case ESC:
                    // printf("[debug]ESC\r\n");
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
