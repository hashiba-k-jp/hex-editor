/**
* @file terminalprint.h
* @brief ターミナル表示に関するもの。\n For terminal print.
* @author HASHIBA Keishi
*/

/**
* ターミナルに表示される文字とエスケープシーケンスからなる文字列データ。\n
* The char array data consisted of characters and escape sequences which will be printed on the terminal.
*/
typedef struct DISP_BUF{
    char *disp; /**> 文字列データ。\n The char array. */
    int len; /**> 文字列データの長さ。\n The length of char array. */
}DISP_BUF;

/**
* 空の文字列(DISP_BUF)データ。\n
* An empty char array (DISP_BUF) data.
*/
#define DBUF_INIT {NULL, 0}

/**
* DISP_BUF に文字またはエスケープシーケンスを追加する。\n
* Add characters and/or escape sequences into the DISP_BUF.
* @param dbuf データはdbufに追加される。\n Add data into dbuf.
* @param s 追加するデータ。\n The data which will be added.
* @param len 追加するデータの長さ。\n The length of being added data.
*/
void dbufAppend(struct DISP_BUF *dbuf, const char *s, int len){
    char *new = realloc(dbuf->disp, dbuf->len+len);
    if(new != NULL){
        memcpy(new+dbuf->len, s, len);
        dbuf->disp = new;
        dbuf->len += len;
    }
}

/**
* DISP_BUF の値を削除する。\n
* Dalete the data of DISP_BUF.
* @param dbuf 削除するデータ。\n The data which will be deleted.
*/
void dbufFree(struct DISP_BUF *dbuf){
    free(dbuf->disp);
    dbuf->len = 0;
}

/**
* 表示する画面を構成する。\n
* Construct the display.
* @param msg 画面下に表示されるメッセージ文。\n The message which will be displayed buttom of the screen.
*/
void displayScreen(char* msg){

    struct DISP_BUF dbuf = DBUF_INIT;
    struct BYTE row[E.width];

    int idx = E.idx - 1;
    // dbufAppend(&dbuf, "\x1b[2", 3);

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
                    // NOT writable characters
                    if(E.iscolored){
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
                    }else{
                        dbufAppend(&dbuf, ".", 1);
                    }
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
        snprintf(status, sizeof(char)*(lenFilename + 32 + 1), "%.20s - at %08X of %08X lines", E.filename, currentRow, (E.numbytes / E.width));
        dbufAppend(&dbuf, status, (lenFilename + 32));
        for(i = 0; i < E.screencols - (lenFilename + 32); i++){
            dbufAppend(&dbuf, " ", 1);
        }
    }else{
        // just current position and number of lines.
        dbufAppend(&dbuf, "XX", 2);
    }
    dbufAppend(&dbuf, "\r\n\x1b[0m\x1b[K", 9);
    dbufAppend(&dbuf, msg, strlen(msg));
    dbufAppend(&dbuf, "\x1b[0m", 6);

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
