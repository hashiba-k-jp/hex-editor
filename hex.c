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

#include "terminalmode.h"
#include "editorconfig.h"
#include "terminalprint.h"

enum KEYS{
    ESC    = 0x1B,
    CTRL_Q = 0x11,
    CTRL_S = 0x13,
    CTRL_X = 0x18,
    DEL    = 0x7F,
    DEL_KEY, HOME_KEY, END_KEY,
    PAGE_UP, PAGE_DW,
    ARR_UP, ARR_DW, ARR_RI, ARR_LE,
};


void moveCursor(int dir);



/**
* この関数はデータに文字を追加する。\n
* This function is used to insert characters into the data.
*
* @param at 追加する位置を示す。\n The location of inserting.
* @param ca 追加する文字バイト。\n The byte data of inserting character.
*/
void editorInsertByte(int at, unsigned int ca){
    // printf("[called] editorInsertByte\n");

    E.bytes = realloc(E.bytes, sizeof(BYTE) * (E.numbytes + 1));

    E.bytes[at].c = ca;
    E.numbytes++;
}

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

void editorSetStatusMessage(const char *msg){
}

/* 553 ======================= Editor rows implementation ======================= */

/**
* この関数はメインデータにデータを挿入する。挿入位置は EDITORCONFIG.idx から計算される。\n
* This function will insert data. The position is calculated by EDITORCONFIG.idx.
* @param c 挿入データ。\n Inserting data.
*/
void insertChar(int c){
    /*

     * idx 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     *     01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ef
     *            ~~
     *     if cursor is located right after '5', idx should be 3.

     * idx 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     *     01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ef

     * cursor 0 : E.dx = 0
     * cursor 1 : E.dx = 2 or 3
     * cursor 2 : E.dx = 5 or 6
     * cursor = (E.dx+1)/3 ...?
     * E.dx max = 47
     */

    int idxCX = (E.rowoff + E.cy) * 3 * E.width + E.cx;
    int idx = (idxCX+1)/3;

    int hexHalf;

    if(0x30 <= c && c <= 0x39){
        hexHalf = c - 0x30;
    }else if(0x61 <= c && c <= 0x66){
        hexHalf = c - 0x57;
    }else if(0x41 <= c && c <= 0x46){
        hexHalf = c - 0x37;
    }

    if(E.idx > E.numbytes){
        // This case should not be happned.
        // ... and I think this case can not happen.
        E.idx = E.numbytes;
    }

    if(E.editing){
        // editing
        int hexFull = E.bytes[E.idx-1].c + hexHalf;
        E.bytes[E.idx-1].c = hexFull;
        E.editing = 0;
    }else{
        // not editing.
        memcpy(E.bytes+E.idx+1, E.bytes+E.idx, sizeof(BYTE)*(E.numbytes-idx));
        editorInsertByte(E.idx, hexHalf*0x10);
        if(E.cx == (3*E.width) - 1){
            moveCursor(ARR_RI);
        }
        moveCursor(ARR_RI);
        E.editing = 1;
    }
}

/**
* この関数はメインデータからデータを削除する。削除されるデータの位置は EDITORCONFIG.idx から計算される。\n
* This function will delete data. The delete data will be calculated by EDITORCONFIG.idx.
*/
void deleteChar(void){
    if(E.editing){
        // editing.
        memcpy(E.bytes+E.idx-1, E.bytes+E.idx, sizeof(BYTE)*(E.numbytes-E.idx));
        E.numbytes--;
        E.editing = 0;
        moveCursor(ARR_LE);
        if(E.idx % E.width == 0){
            moveCursor(ARR_LE);
        }
    }else{
        // not editing.
        if(E.idx == 0){return;}
        // moveCursor(ARR_LE);
        E.bytes[E.idx-1].c = (E.bytes[E.idx-1].c / 0x10) * 0x10;
        E.editing = 1;
    }
}

/* 854 ============================= Terminal update ============================ */



/* 1109 ========================= Editor events handling  ======================== */

void moveCursor(int dir){
    // printf("[called] moveCursor\r\n");

    if(E.editing != 0){
        return;
    }

    /*

     * idx 0  1  2  3
     *     01 23 45 67

     * cursor 0 : E.dx = 0 or 1
     * cursor 1 : E.dx = 2 or 3 or 4
     * cursor 2 : E.dx = 5 or 6 or 7
     * cursor 3 : E.dx = 8 or 9 or 10
     * cursor = (E.dx+1)/3 ...?
     * if(E.dx == 11){cursor will be 4; DO NOT MOVE THE CORSOR.}
     * else{MOVE.}
     */

    int idxCX = (E.rowoff + E.cy) * 3 * E.width + E.cx;
    int idx = (idxCX+1)/3;
    int tmpRow = E.rowoff + E.cy;

    switch (dir) {
        case ARR_UP:
            // printf("[called] moveCursor -> ARR_UP\r\n");
            if(E.cy == 0){
                if(E.rowoff != 0){
                    E.idx -= E.width;
                    E.rowoff--;
                }else{
                    E.idx = 0;
                    E.rowoff = 0;
                    E.cx = 0;
                    E.cy = 0;
                }
            }else{
                E.idx -= E.width;
                E.cy--;
            }
            break;
        case ARR_DW:
            if(tmpRow == ((E.numbytes - 1) / E.width)){
                E.idx = E.numbytes;
                if(E.numbytes % E.width != 0){
                    E.cx = 3 * (E.numbytes % E.width) - 1;
                }else{
                    E.cx = 3 * E.width - 1;
                }
            }else if(E.numbytes <= idx+E.width){
                E.idx = E.numbytes;
                E.cy++;
                E.cx = 3 * (E.numbytes % E.width) - 1;
            }else{
                if(tmpRow < (E.numbytes / E.width)){
                    E.idx += E.width;
                    if(E.cy == E.screenrows - 1){
                        E.rowoff++; // Scroll 1 line.
                        // cursor position will NOT be changed.
                    }else{
                        E.cy++;
                    }
                }
            }
            break;
        case ARR_RI:
            // EOF
            if(E.numbytes <= idx){
                // EOF
                // do NOTHING; do NOT allow to move cursor into the EOF or later bytes.
            }else{
                if(E.cx == (E.width*3)-1){
                    // end of the line
                    if(tmpRow == E.numbytes/E.width){
                        // end of the file; the last row is filled.
                    }else{
                        // otherwise; the next (last) row has some bytes, but not filled.
                        // E.idx will not be changed.
                        E.cx = 0;
                        E.cy++;
                    }
                }else{
                    E.idx += 1;
                    if(E.cx == 0){
                        E.cx += 2;
                    }else{
                        E.cx += 3;
                    }
                }
            }
            break;
        case ARR_LE:
            if(idx <= 0){
                // begin of the file; cursor can not move to left.
            }else{
                if(E.cx == 0){
                    // begin of the line.
                    if(tmpRow == 0){
                        // begin of the file. (Can this case happen?)
                    }else{
                        // E.idx will not be changed.
                        E.cx = (E.width*3)-1;
                        E.cy--;
                    }
                }else{
                    E.idx -= 1;
                    E.cx -= 3;
                    if(E.cx < 0){
                        E.cx = 0;
                    }
                }
            }
            break;
        default:
            printf("\x1b[1m[!!!] Unexpected problem : E01\x1b[0m\r\n");
    }
}

void keyProcess(int fd){
    // printf("[called] keyProcess\r\n");
    int c = keyInput(fd);
    switch (c) {
        case CTRL_Q: // ^Q
            exit(0);
        case CTRL_S: // ^S
            editorSave();
            break;
        case CTRL_X: // ^X
            break;
        case DEL:
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
                insertChar(c);
            }
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
        fprintf(stderr,"Usage: kilo <filename>\n");
        exit(1);
    }

    editorOpen(argv[argc-1]);
    enableRawMode(STDIN_FILENO);
    // editorSetStatusMessage()

    while(1){
        displayScreen();
        keyProcess(STDIN_FILENO);
    }

    return EXIT_SUCCESS;
}
