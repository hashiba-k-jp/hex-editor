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

typedef struct BYTE {
    unsigned int c;
}BYTE;


struct EDITORCONFIG {
    int cx,cy;  /* Cursor x and y position in characters */
    int rowoff;     /* Offset of row displayed. */ // ex. if rowoff = 3, the first 3 rows will not be displayed.
    // int coloff;     /* Offset of column displayed. */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numbytes;    /* Number of bytes */
    int max_disp_rows;   /* Number of rows */ // := numrows
    // int rawmode;    /* Is terminal raw mode enabled? */
    // erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */ // 0 means the file has not beem modified, otherwise means modified.
    char *filename; /* Currently open filename */
    // char statusmsg[80];
    // time_t statusmsg_time;
    // struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    BYTE *bytes; /* This is array. */

    int width; /* number of bytes which are displayed in 1 row. */
};

enum KEYS{
    ESC = 0x1B,
    DEL_KEY, HOME_KEY, END_KEY,
    PAGE_UP, PAGE_DW,
    ARR_UP, ARR_DW, ARR_RI, ARR_LE,
};

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
    E.screenrows -= 4; /* Get room for status bar. */
}

void initEditor(void){
    printf("[called] initEditor\n");

    E.cx = 10;
    E.cy = 3;
    E.rowoff = 0;
    // E.coloff = 0;
    E.max_disp_rows = 0;
    // E.row = NULL;
    E.dirty = 0;
    // E.filename = NULL;
    // E.syntax = NULL;
    // E.numbytes = 0;

    E.width = 0x10;
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

    // printf("[debug] successed to open the file.\n");

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

/* 200 ======================= Low level terminal handling ====================== */

static struct termios orig_termios; /* In order to restore at exit.*/

void disableRawMode(int fd){
    // tcsetattr(STDIN_FILENO, 0, &COOKEDMODE);
    tcsetattr(fd, TCSAFLUSH, &orig_termios);
    printf("\x1B[0K\x1B[1m[debug] switched to cooked mode.\x1B[0m\n");
}

void editorAtExit(void) {
    disableRawMode(STDIN_FILENO);
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
    struct BYTE row[E.width];

    dbufAppend(&dbuf, "\x1b[?25l", 6); /* Hide cursor. */
    dbufAppend(&dbuf, "\x1b[H", 3); /* Go home. */

    /* Header Lines */
    dbufAppend(&dbuf, "\x1b[36mAddress  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ascii\x1b[0m\r\n", 73);
    dbufAppend(&dbuf, "\x1b[36m-------- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ----------------\x1b[0m\r\n", 84);

    int row_block = E.numbytes / E.width;
    int row_block_remainder = E.numbytes % E.width;
    int i;

    for(int b = 0; b < row_block; b++){
        int disp_row = E.rowoff + b;

        if(b >= E.screenrows){
            continue;
        }

        char *address = malloc(sizeof(char) * 8);
        snprintf(address, 19, "\x1b[37m%08X\x1b[0m ", disp_row*E.width);
        dbufAppend(&dbuf, address, 18);
        free(address);

        memcpy(row, E.bytes+(disp_row*E.width), sizeof(BYTE)*E.width);

        // hexstring (2 chars)
        for(i = 0; i < E.width; i++){
            char *tmphex = malloc(sizeof(char) * 2);
            snprintf(tmphex, 4, "%02x ", row[i].c);
            dbufAppend(&dbuf, tmphex, 3);
            free(tmphex);
        }

        for(i = 0; i < E.width; i++){
            char *tmp = malloc(sizeof(char) * 2);
            if(0x20 <= row[i].c && row[i].c <= 0x7E){
                // writable characters
                snprintf(tmp, 2, "%c", row[i].c);
                dbufAppend(&dbuf, tmp, 2);
            }else{
                dbufAppend(&dbuf, "\x1b[32m", 5);
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

    /* put cursor */
    char *tmpcursor = malloc(sizeof(char) * 8);
    // "\x1b[Y;X" will make cursor positioned (X, Y).
    // The coordinate of the origin (upper left) will be (0, 0), not(1, 1)
    snprintf(tmpcursor, 9, "\x1b[%d;%dH", E.cy+3, E.cx+10);
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
    int tmpRow = E.rowoff + E.cy;
    switch (dir) {
        case ARR_UP:
            // printf("[called] moveCursor -> ARR_UP\r\n");
            if(E.cy == 0){
                if(E.rowoff != 0){
                    E.rowoff--;
                } // else otherwise, do nothing.
            }else{
                E.cy--;
            }
            break;
        case ARR_DW:
            if(tmpRow < (E.numbytes / E.width) + 1){
                if(E.cy == E.screenrows - 1){
                    E.rowoff++; // Scroll 1 line.
                    // cursor position will NOT be changed.
                }else{
                    E.cy++;
                }
            }
            break;
        case ARR_RI:
            break;
        case ARR_LE:
            break;
        default:
            printf("\x1b[1m[!!!] Unexpected problem : E01\x1b[0m\r\n");
    }
}

void keyProcess(int fd){
    // printf("[called] keyProcess\r\n");
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
    enableRawMode(STDIN_FILENO);
    // editorSetStatusMessage()

    // test_print();

    while(1){
        displayScreen();
        keyProcess(STDIN_FILENO);
    }



    disableRawMode(STDIN_FILENO);
    deinitEditor();

    printf("%d\n", E.numbytes);
}
