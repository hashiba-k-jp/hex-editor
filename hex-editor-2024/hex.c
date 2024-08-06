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

#include <unistd.h>  // getopt(),
#include <err.h>    // warn(), err(),
#include <errno.h>  // errno,
#include <sys/ioctl.h>  // struct insize,
#include <termios.h>    // struct termios,
#include <stdlib.h> // atexit(),
#include <string.h> // memcmp(),
// #include <ncurses.h>    // ncurses:initscr(),


#include "config.h"
#include "terminalMode.h"
#include "keys.h"

int keyProcess(int c, char* msg, EDITOR EDITOR);
int keyInput();

int main(int argc, char *argv[]){

    struct termios COOKED_MODE;
    EDITOR EDITOR = init_editor();

    int ch;
    while ((ch = getopt(argc, argv, "i:cm")) != -1) {
        switch (ch) {
            case 'c':
                EDITOR.isColored = true;
                break;
            case 'i':
                if(EDITOR.filename == NULL){
                    EDITOR.filename = optarg;
                }else{
                    warn("[WARN]too many input files; run \"%s -m\" to read manual.", argv[0]);
                }
                break;
            case 'm':
                print_usage();
                return 0;
        }
    }

    atexit(fin_program);
    save_cooked_mode();
    switch_raw_mode();

    // printf("[DEBUG] FILENAME : %s\n", EDITOR.filename);
    // printf("[DEBUG] ROW : %d\n", EDITOR.ws.ws_row);
    // printf("[DEBUG] COL : %d\n", EDITOR.ws.ws_col);

    char *msg;
    int input_c;
    int res_process;

    while(1){
        input_c = keyInput();
        res_process = keyProcess(input_c, msg, EDITOR);
        // if(input_c == CTRL_Q){
        //     return 0;
        // }else{
        //     printf(" -> %c\r\n", input_c);
        // }
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
            return seq[0];
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
                    return 0x00;
            }
        }
    }else if(read_len == 2){
        switch(seq[2]){
            case 0x41:
                return ARR_UP;
                break;
            case 0x42:
                return ARR_DW;
                break;
            case 0x43:
                return ARR_RI;
                break;
            case 0x44:
                return ARR_LE;
                break;
        }
    }


    return 0;
}
