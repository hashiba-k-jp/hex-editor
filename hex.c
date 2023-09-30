/**
* @file hex.c
* @brief This is the main file.
* @author HASHIBA Keishi
*/


#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
// #include <time.h>
// #include <sys/types.h>
#include <sys/ioctl.h>
// #include <sys/time.h>
#include <unistd.h>
// #include <stdarg.h>
// #include <fcntl.h>
// #include <signal.h>
#include <getopt.h>

#include "terminalmode.h"  // require no other files.
#include "editorconfig.h"  // require no other files.
#include "terminalprint.h" // require editorconfig.h
#include "cursor.h"        // require editorconfig.h
#include "editdata.h"      // require editorconfig.h and cursor.h

enum KEYS{
    ESC    = 0x1B,
    CTRL_Q = 0x11,
    CTRL_S = 0x13,
    CTRL_X = 0x18,
    DEL    = 0x7F,
    DEL_KEY, HOME_KEY, END_KEY,
    PAGE_UP, PAGE_DW,
};

/**
* この関数は編集するファイルを開き、メインデータを構築する。一度だけ実行される。\n
* This function will open the target file, and construct the main data array. This is called only once.
*
* @param filename 編集するファイル名。\n The name of target file.
* @return 成功したら0を返す。\n 0 when the file is opening and the main data construcing was successed.
*/
int editorOpen(char *filename){
    printf("[called] editorOpen\n");
    FILE *fp;

    free(E.filename);
    size_t filename_len = strlen(filename) + 1;
    E.filename = malloc(filename_len);
    memcpy(E.filename,filename,filename_len);

    fp = fopen(filename, "r");
    if(!fp){
        // failed to open the file ...?
        return 1;
    }

    // printf("[debug] successed to open the file.\n");

    unsigned int c;
    while((c = fgetc(fp)) != EOF){
        editorInsertByte(E.numbytes, c);
    }
    fclose(fp);
    // E.dirty = 0;
    return 0;
}

/**
* この関数はメインデータの変更をファイルに書き込む。\n
* This function will save changes into the file.
*/
int editorSave(void){
    int i;
    FILE *fp;
    fp = fopen(E.filename,"w");

    for(i = 0; i < E.numbytes; i++){
        fwrite(&E.bytes[i].c, sizeof(char), 1, fp);
        printf("\a");
    }
    fclose(fp);
    return 0;
}

/* 200 ======================= Low level terminal handling ====================== */

/**
* この関数はキー入力を受け取る。rawモードで実行されるため、Ctrl-Cなどは無効化されていることに留意。\n
* This function gets key inputs. Note that inputs such as Ctrl-C will NOT work.
* @param fd STDIN_FILENO
*/
int keyInput(int fd){
    // printf("[called] keyInput\r\n");
    int reads; // s means Status
    char c, seq[3];
    while((reads = read(fd, &c, 1)) == 0){
        // pass; This while waits any input.
    }
    if(reads == -1){
        // failed to get input from key.
        exit(1);
    }

    while(1){
        switch(c){
            case ESC:
                // just an ESC
                if((reads = read(fd, seq, 1)) == 0){return ESC;}
                if((reads = read(fd, seq+1, 1)) == 0){return ESC;}

                // ESC [ ... sequence
                if(seq[0] == '['){
                    if('0' <= seq[1] && seq[1] <= '9'){
                        if((reads = read(fd, seq+2, 1)) == 0){return ESC;}
                        if(seq[2] == '~'){
                            switch (seq[1]) {
                                case '3':
                                    return DEL_KEY;
                                case '5':
                                    return PAGE_UP;
                                case '6':
                                    return PAGE_DW;
                            }
                        }
                    }else{
                        switch (seq[1]) {
                            case 'A': return ARR_UP;
                            case 'B': return ARR_DW;
                            case 'C': return ARR_RI;
                            case 'D': return ARR_LE;
                            case 'F': return HOME_KEY;
                            case 'H': return END_KEY;
                        }
                    }
                }
                break;
            default:
                return c;
        }
    }
}


/* 553 ======================= Editor rows implementation ======================= */
/* 854 ============================= Terminal update ============================ */
/* 1109 ========================= Editor events handling  ======================== */

/**
* この関数はキー入力を処理する。\n
* This function handles key inputs.
* @param fd STDIN_FILENO
* @param [out] msg ステータスメッセージを格納する変数。\n The variable which store the status message.
*/
void keyProcess(int fd, char* msg){
    // printf("[called] keyProcess\r\n");
    int c = keyInput(fd);
    if(E.dirty){
        memcpy(msg, "\x1b[7mCtrl-S\x1b[0m to save | \x1b[7mCtrl-Q\x1b[0m to quit (EDITED)", sizeof(char)*57);
    }else{
        memcpy(msg, "\x1b[7mCtrl-S\x1b[0m to save | \x1b[7mCtrl-Q\x1b[0m to quit", sizeof(char)*48);
    }
    switch (c) {
        case CTRL_Q: // ^Q
            if(E.dirty == 0){
                exit(0);
            }else{
                if(E.quittimes == 1){
                    exit(0);
                }else{
                    E.quittimes = 1;
                    memcpy(msg, "Press Ctrl-Q again to quit. \x1b[1m(The changes have not been saved !!!)\x1b[0m", sizeof(char)*74);
                }
            }
            break;
        case CTRL_S: // ^S
            editorSave();
            E.dirty = 0;
            memcpy(msg, "Saved!", sizeof(char)*7);
            break;
        case CTRL_X: // ^X
            break;
        case DEL:
            E.dirty = 1;
            deleteChar();
            break;
        case ARR_UP:
        case ARR_DW:
        case ARR_RI:
        case ARR_LE:
            moveCursor(c);
        default:
            if((0x30 <= c && c <= 0x39) || (0x61 <= c && c <= 0x66)){
                // 0-9, a-f (A-F is not included.)
                E.dirty = 1;
                insertChar(c);
            }
    }
    if(c != CTRL_Q){
        E.quittimes = 0;
    }
}

int main(int argc, char *argv[]){
    printf("[called] main\r\n");

    initEditor();

    int opt, longindex;
    struct option longopts[] = {
        {"ascii", no_argument, NULL, 'a'},
        {"color", no_argument, NULL, 'c'},
    };

    while((opt = getopt_long(argc, argv, "ac", longopts, &longindex)) != -1){
        // printf("%d %s\n", longindex, longopts[longindex].name);
        switch (opt) {
            case 'a':
                // E.dispascii = 1;
                break;
            case 'c':
                E.iscolored = 1;
                break;
        }
    }

    if(argc < 2){
        fprintf(stderr,"Usage: ./hex <filename>\n");
        exit(1);
    }

    editorOpen(argv[argc-1]);
    enableRawMode(STDIN_FILENO);
    // editorSetStatusMessage()

    char *msg = NULL;
    msg = (char *)malloc(100);

    memcpy(msg, "\x1b[7mCtrl-S\x1b[0m to save | \x1b[7mCtrl-Q\x1b[0m to quit", sizeof(char)*48);

    displayScreen(msg);

    while(1){
        keyProcess(STDIN_FILENO, msg);
        displayScreen(msg);
    }

    return EXIT_SUCCESS;
}
