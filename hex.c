#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
// #include <errno.h>
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

typedef struct BYTE {
    unsigned int c;
}BYTE;


struct EDITORCONFIG {
    // int cx,cy;  /* Cursor x and y position in characters */
    int rowoff;     /* Offset of row displayed. */
    // int coloff;     /* Offset of column displayed. */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numbytes;    /* Number of bytes */
    int max_disp_rows;   /* Number of rows */ // := numrows
    // int rawmode;    /* Is terminal raw mode enabled? */
    // erow *row;      /* Rows */
    // int dirty;      /* File modified but not saved. */
    char *filename; /* Currently open filename */
    // char statusmsg[80];
    // time_t statusmsg_time;
    // struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    BYTE *bytes; /* This is array. */

};

enum KEYS{
    ESC = 0x1B,
    DEL_KEY, HOME_KEY, END_KEY,
    PAGE_UP, PAGE_DW,
    ARR_UP, ARR_DW, ARR_RI, ARR_LE,
};

const char HEXSTRING[16][1] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};

static struct EDITORCONFIG E;

// used to switch to raw mode, and store the cooked mode.
struct termios COOKEDMODE;
struct termios RAWMODE;


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
    E.screenrows -= 2; /* Get room for status bar. */
}

void initEditor(void){
    printf("[called] initEditor\n");

    // E.cx = 0;
    // E.cy = 0;
    E.rowoff = 0;
    // E.coloff = 0;
    E.max_disp_rows = 0;
    // E.row = NULL;
    // E.dirty = 0;
    // E.filename = NULL;
    // E.syntax = NULL;
    // E.numbytes = 0;
    updateWindowSize();
}

void deinitEditor(void){
    free(E.bytes);
}

void editorInsertByte(int at, unsigned int ca){
    // printf("[called] editorInsertByte\n");

    // THIS FUNCTION IS JUST ADDING AT THE END !!! (TENPORARY)
    E.bytes = realloc(E.bytes, sizeof(BYTE) * (E.numbytes + 1));

    E.bytes[at].c = ca;
    // printf("%c", ca);
    E.numbytes++;
    // E.dirty++;
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

    printf("[debug] successed to open the file.\n");

    unsigned int c;
    while((c = fgetc(fp)) != EOF){
        editorInsertByte(E.numbytes, c);
    }
    fclose(fp);
    // E.dirty = 0;
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

void enableRawMode(void){
    printf("[called] enableRawMode\n");
    tcgetattr(STDIN_FILENO, &COOKEDMODE);
    cfmakeraw(&RAWMODE);
    tcsetattr(STDIN_FILENO, 0, &RAWMODE);
    printf("\x1B[1m[debug] switched to RAWMODE.\x1B[0m\r\n");
}

void disableRawMode(void){
    tcsetattr(STDIN_FILENO, 0, &COOKEDMODE);
    printf("\x1B[0K\x1B[1m[debug] switched to cooked mode.\x1B[0m\n");
}

void editorSetStatusMessage(const char *msg){

}

/* === display === */
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
    const int width = 16; // This might be changed.
    const int block = 8;  // This must be fixed.
    struct DISP_BUF dbuf = DBUF_INIT;
    struct BYTE row[width];

    dbufAppend(&dbuf, "\x1b[?25l", 6); /* Hide cursor. */
    dbufAppend(&dbuf, "\x1b[H", 3); /* Go home. */
    int row_block = E.numbytes / width;
    int row_block_remainder = E.numbytes % width;
    int i, j;

    for(int b = 0; b < row_block; b++){
        int disp_row = E.rowoff + b;

        if(disp_row >= E.screenrows){
            continue;
        }

        char *address = malloc(sizeof(char) * 8);
        snprintf(address, 10, "%08X ", b);
        dbufAppend(&dbuf, address, 9);
        free(address);

        memcpy(row, E.bytes+(b*width), sizeof(BYTE)*width);

        // hexstring (2 chars)
        for(i = 0; i < width; i++){
            char *tmphex = malloc(sizeof(char) * 2);
            snprintf(tmphex, 4, "%02x ", row[i].c);
            dbufAppend(&dbuf, tmphex, 3);
            free(tmphex);
        }

        for(i = 0; i < width; i++){
            char *tmp = malloc(sizeof(char) * 2);
            if(0x20 <= row[i].c && row[i].c <= 0x7E){
                // writable characters
                snprintf(tmp, 2, "%c", row[i].c);
            }else{
                snprintf(tmp, 2, ".");
            }
            dbufAppend(&dbuf, tmp, 2);
            free(tmp);
        }

        dbufAppend(&dbuf, "\x1b[39m", 5);
        dbufAppend(&dbuf, "\x1b[0K", 4);
        dbufAppend(&dbuf, "\r\n", 2);
        // printf("[debug] [successed!] add overall row\r\n");
        // free(row);
    }
    /*
     *  HERE : handle the row_block_remainder.
     */

    /* status rows (1 of 2 rows) */
    dbufAppend(&dbuf, "\x1b[0K", 4);
    dbufAppend(&dbuf, "\x1b[7m", 4);
    dbufAppend(&dbuf, "test status (THIS MESSAGE SHOULD BE REPLACED)", 45);
    dbufAppend(&dbuf, "\x1b[0m\r\n", 6);

    /* status rows (1 of 2 rows) */
    // just pass

    // printf("\r\n\r\n === write S === \r\n");
    write(STDOUT_FILENO, dbuf.disp, dbuf.len);
    // printf("\r\n === write E === \r\n\r\n");

    dbufFree(&dbuf);
}

int keyInput(int fd){
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
                    }
                }

                // ESC O ... sequence
                else if(seq[0] == 'O'){
                    switch (seq[1]) {
                        case 'A': return ARR_UP;
                        case 'B': return ARR_DW;
                        case 'C': return ARR_RI;
                        case 'D': return ARR_LE;
                        case 'F': return HOME_KEY;
                        case 'H': return END_KEY;
                    }
                }
                break;
            default:
                return c;
        }
    }
}

void moveCursor(int dir){
    switch (dir) {
        case ARR_UP:
            printf("[debug] UP\r\n");
            break;
        case ARR_DW:
            printf("[debug] DW\r\n");
            break;
        case ARR_RI:
            printf("[debug] RI\r\n");
            break;
        case ARR_LE:
            printf("[debug] LE\r\n");
            break;
        default:
            printf("\x1b[1m[!!!] Unexpected problem : E01\x1b[0m\r\n");
    }
}

void keyProcess(int fd){
    int c = keyInput(fd);
    switch (c) {
        case ARR_UP:
        case ARR_DW:
        case ARR_RI:
        case ARR_LE:
            moveCursor(c);
    }
}

int main(int argc, char *argv[]){
    printf("[called] main\n");
    if (argc != 2) {
        fprintf(stderr,"Usage: kilo <filename>\n");
        exit(1);
    }

    initEditor();
    editorOpen(argv[1]);
    enableRawMode();
    // editorSetStatusMessage()

    // test_print();

    while(1){
        displayScreen();
        keyProcess(STDIN_FILENO);
    }



    disableRawMode();
    deinitEditor();

    printf("%d\n", E.numbytes);
}
