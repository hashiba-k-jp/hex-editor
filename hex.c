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

typedef struct BYTE {
    unsigned int c;
}BYTE;


struct EDITORCONFIG {
    int cx,cy;  /* Cursor x and y position in characters */
    int rowoff;     /* Offset of row displayed. */ // ex. if rowoff = 3, the first 3 rows will not be displayed.
    int coloff;     /* Offset of column displayed. */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numbytes;    /* Number of bytes */
    // int rawmode;    /* Is terminal raw mode enabled? */
    // erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */ // 0 means the file has not beem modified, otherwise means modified.
    char *filename; /* Currently open filename */
    // char statusmsg[80];
    // time_t statusmsg_time;
    // struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    BYTE *bytes; /* This is array. */

    int width; /* number of bytes which are displayed in 1 row. */
    int prevScroll;
    int editing; // 0 is false, otherwise are true;
    int editingIdx; // editing index.
    int idx;

    int dispascii; // 0 NO header, 1 header (list of ascii characters)
    int iscolored; // 0 NOT colored, 1 colored
};

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

static struct EDITORCONFIG E;

// used to switch to raw mode, and store the cooked mode.
struct termios COOKEDMODE;
struct termios RAWMODE;

void moveCursor(int dir);



int getWindowSize(int ifd, int ofd, int *rows, int *cols){
    struct winsize ws;
    if(ioctl(1, TIOCGWINSZ, &ws) == 0){
        // successed to get window size
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }else{
        // failed
        return -1;
    }
}

void updateWindowSize(void) {
    printf("[called] updateWindowSize\n");
    if (getWindowSize(STDIN_FILENO, STDOUT_FILENO, &E.screenrows, &E.screencols) == -1) {
        perror("Unable to query the screen for size (columns / rows)");
        exit(1);
    }
    E.screenrows -= 4; /* Get room for status bar. */

    // Why does NOT this work ?
    if(E.dispascii){
        E.screenrows -= 8;
    }
}

void initEditor(void){
    printf("[called] initEditor\n");

    E.cx = 0;
    E.cy = 0;
    E.width = 0x10;
    E.rowoff = 0; // length of address might be 8, and additional space.
    E.coloff = 10;
    // E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    // E.syntax = NULL;
    E.numbytes = 0;

    E.editing   = 0;
    E.idx       = 0;
    E.dispascii = 0;
    E.iscolored = 0;

    updateWindowSize();
    printf("\x1B[%dS", E.screenrows);
}

void deinitEditor(void){
    free(E.bytes);
}

void editorInsertByte(int at, unsigned int ca){
    // printf("[called] editorInsertByte\n");

    E.bytes = realloc(E.bytes, sizeof(BYTE) * (E.numbytes + 1));

    E.bytes[at].c = ca;
    E.numbytes++;
}

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

void test_print(void){
    int i;
    printf("\r\n==== Test Print Start ====\r\n");
    for(i = 0; i <= E.numbytes; i++){
        switch (E.bytes[i].c) {
            case 0x0A:
                printf("\r\n");
                break;
            default:
                printf("%c", E.bytes[i].c);
                break;
        }
    }
    printf("\r\n==== Test Print End ====\r\n");
}

/* 200 ======================= Low level terminal handling ====================== */

static struct termios orig_termios; /* In order to restore at exit.*/

void disableRawMode(int fd){
    // tcsetattr(STDIN_FILENO, 0, &COOKEDMODE);
    tcsetattr(fd, TCSAFLUSH, &orig_termios);
    printf("\x1B[0K\x1B[1m[debug] switched to cooked mode.\x1B[0m\n");
}

void editorAtExit(void) {
    disableRawMode(STDIN_FILENO);
    printf("\x1B[2J\x1B[0;0H");
}

int enableRawMode(int fd){
    printf("[called] enableRawMode\n");
    // tcgetattr(STDIN_FILENO, &COOKEDMODE);
    // cfmakeraw(&RAWMODE);
    // tcsetattr(STDIN_FILENO, 0, &RAWMODE);

    struct termios raw;

    // if (E.rawmode) return 0; /* Already enabled. */
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(editorAtExit);
    if (tcgetattr(fd, &orig_termios) == -1) goto fatal;

    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    // E.rawmode = 1;
    printf("\x1B[1m[debug] switched to RAWMODE.\x1B[0m\r\n");
    return 0;

fatal:
    errno = ENOTTY;
    printf("\x1B[1m[ !!! ] FAILED TO SWITCH TO RAWMODE !!!\x1B[0m\r\n");
    return -1;
}

int keyInput(int fd){
    // printf("[called] keyInput\r\n");
    int reads; // s means Status
    char c, seq[3];
    while((reads = read(fd, &c, 1)) == 0){
        // pass
        // This while waits any input.
    }
    if(reads == -1){
        // failed to get input from key.
        exit(1);
    }

    // printf("[debug], c = %c", c);

    while(1){
        switch(c){
            case ESC:
                // printf("[debug] keyInput -> ESC\r\n");

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

void deleteChar(){
    if(E.editing){
        // editing.
        memcpy(E.bytes+E.idx-1, E.bytes+E.idx, sizeof(BYTE)*(E.numbytes-E.idx));
        E.numbytes--;
        E.editing = 0;
        moveCursor(ARR_LE);
    }else{
        // not editing.
        if(E.idx == 0){return;}
        // moveCursor(ARR_LE);
        E.bytes[E.idx-1].c = (E.bytes[E.idx-1].c / 0x10) * 0x10;
        E.editing = 1;
    }
}

/* 854 ============================= Terminal update ============================ */

typedef struct DISP_BUF{
    char *disp;
    int len;
}DISP_BUF;

#define DBUF_INIT {NULL, 0}

void dbufAppend(struct DISP_BUF *dbuf, const char *s, int len){
    char *new = realloc(dbuf->disp, dbuf->len+len);
    if(new != NULL){
        memcpy(new+dbuf->len, s, len);
        dbuf->disp = new;
        dbuf->len += len;
    }
}

void dbufFree(struct DISP_BUF *dbuf){
    free(dbuf->disp);
}

void displayScreen(void){
    struct DISP_BUF dbuf = DBUF_INIT;
    struct BYTE row[E.width];

    int idx = E.idx - 1;

    dbufAppend(&dbuf, "\x1b[?25l", 6); /* Hide cursor. */
    dbufAppend(&dbuf, "\x1b[H", 3); /* Go home. */

    /* Header Lines */
    dbufAppend(&dbuf, "\x1b[36mAddress  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ascii\x1b[0m\r\n", 73);
    dbufAppend(&dbuf, "\x1b[36m-------- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ----------------\x1b[0m\r\n", 84);

    int i;

    for(int b = 0; b < E.screenrows; b++){
        int disp_row = E.rowoff + b;
        char *address = malloc(sizeof(char) * 8);

        snprintf(address, 19, "\x1b[37m%08X\x1b[0m ", disp_row*E.width);
        dbufAppend(&dbuf, address, 18);
        free(address);

        memcpy(row, E.bytes+(disp_row*E.width), sizeof(BYTE)*E.width);
        // hexstring (2 chars)
        if(E.editing){
            // now editing
            for(i = 0; i < E.width; i++){
                char *tmphex = malloc(sizeof(char) * 2);
                if(disp_row*E.width + i >= E.numbytes){
                    dbufAppend(&dbuf, "   ", 3);
                }else if(disp_row*E.width + i == idx){
                    snprintf(tmphex, 13, "\x1B[45m%1x_\x1B[0m ", row[i].c/0x10);
                    dbufAppend(&dbuf, tmphex, 12);
                }else{
                    snprintf(tmphex, 4, "%02x ", row[i].c);
                    dbufAppend(&dbuf, tmphex, 3);
                }
                free(tmphex);
            }
            for(i = 0; i < E.width; i++){
                char *tmp = malloc(sizeof(char) * 2);
                if(disp_row*E.width + i >= E.numbytes){
                    dbufAppend(&dbuf, "\x1B[38;5;66m.\x1B[0m", 17);
                }else if(disp_row*E.width + i == idx){
                    dbufAppend(&dbuf, "\x1B[45m \x1B[0m", 10);
                }else if(0x20 <= row[i].c && row[i].c <= 0x7E){
                    // writable characters
                    snprintf(tmp, 2, "%c", row[i].c);
                    dbufAppend(&dbuf, tmp, 2);
                }else{
                    if(E.iscolored){
                        dbufAppend(&dbuf, "\x1b[32m", 5);
                    }
                    switch (row[i].c) {
                        case 0x0A: // LF \n
                            snprintf(tmp, 2, "n");
                            break;
                        case 0x0D: // CR \r
                            snprintf(tmp, 2, "r");
                            break;
                        default:
                            snprintf(tmp, 2, ".");
                            break;
                    }
                    dbufAppend(&dbuf, tmp, 2);
                    dbufAppend(&dbuf, "\x1b[0m", 4);
                }
                free(tmp);
            }

        }else{
            // not editing
            for(i = 0; i < E.width; i++){
                char *tmphex = malloc(sizeof(char) * 2);
                if(disp_row*E.width + i >= E.numbytes){
                    dbufAppend(&dbuf, "   ", 3);
                }else{
                    snprintf(tmphex, 4, "%02x ", row[i].c);
                    dbufAppend(&dbuf, tmphex, 3);
                }
                free(tmphex);
            }

            for(i = 0; i < E.width; i++){
                char *tmp = malloc(sizeof(char) * 2);
                if(disp_row*E.width + i >= E.numbytes){
                    dbufAppend(&dbuf, "\x1B[38;5;66m.\x1B[0m", 17);
                }else if(0x20 <= row[i].c && row[i].c <= 0x7E){
                    // writable characters
                    snprintf(tmp, 2, "%c", row[i].c);
                    dbufAppend(&dbuf, tmp, 2);
                }else{
                    if(E.iscolored){
                        dbufAppend(&dbuf, "\x1b[32m", 5);
                    }
                    switch (row[i].c) {
                        case 0x0A: // LF \n
                            snprintf(tmp, 2, "n");
                            break;
                        case 0x0D: // CR \r
                            snprintf(tmp, 2, "r");
                            break;
                        default:
                            snprintf(tmp, 2, ".");
                            break;
                    }
                    dbufAppend(&dbuf, tmp, 2);
                    dbufAppend(&dbuf, "\x1b[0m", 4);
                }
                free(tmp);
            }
        }

        dbufAppend(&dbuf, "\x1b[39m", 5);
        dbufAppend(&dbuf, "\x1b[0K", 4);
        dbufAppend(&dbuf, "\r\n", 2);
    }
    /*
     *  HERE : handle the row_block_remainder.
     *  No thing to do ...?
     */

    /* status rows (1 of 2 rows) */
    char status[E.screencols+1];
    int currentRow = E.rowoff + E.cy;
    int lenFilename;

    dbufAppend(&dbuf, "\x1b[0K\x1b[7m", 8);
    if((lenFilename = strlen(E.filename)) <= E.screencols - 36){
        // eg. hex.c - at 0x0000329x of 0x00000329 lines

        /*
        snprintf(status, sizeof(char)*(lenFilename + 32 + 1), "%.20s - at %08X of %08X lines", E.filename, currentRow, (E.numbytes / E.width));
        dbufAppend(&dbuf, status, (lenFilename + 32));
        for(i = 0; i < E.screencols - (lenFilename + 32); i++){
            dbufAppend(&dbuf, " ", 1);
        }
        */

        snprintf(status, sizeof(char)*2, "%d", E.dispascii);
        dbufAppend(&dbuf, status, 1);
        for(i = 0; i < E.screencols - 1; i++){
            dbufAppend(&dbuf, " ", 1);
        }


    }else{
        // just current position and number of lines.
        dbufAppend(&dbuf, "XX", 2);
    }
    dbufAppend(&dbuf, "\r\n\x1b[0m", 6);

    /* status rows (1 of 2 rows) */
    // just pass

    /* put cursor */
    char *tmpcursor = malloc(sizeof(char) * 8);
    // "\x1b[Y;X" will make cursor positioned (X, Y).
    // The coordinate of the origin (upper left) will be (0, 0), not(1, 1)
    int dcx = E.cx+E.coloff; // d stands for display

    if(E.editing != 0){
        dcx -= 1;
    }
    snprintf(tmpcursor, 9, "\x1b[%d;%dH", E.cy+3, dcx);
    dbufAppend(&dbuf, tmpcursor, 8);
    dbufAppend(&dbuf,"\x1b[?25h",6); /* Show cursor. */
    free(tmpcursor);

    // printf("\r\n\r\n === write S === \r\n");
    write(STDOUT_FILENO, dbuf.disp, dbuf.len);
    // printf("\r\n === write E === \r\n\r\n");

    dbufFree(&dbuf);
}

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
