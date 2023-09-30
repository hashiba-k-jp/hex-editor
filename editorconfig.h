/**
* @file editorconfig.h
* @brief エディタそのものや画面の設定に関するもの。\n For configure editor settings and displays.
* @author HASHIBA Keishi
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

/**
* 本体データ配列を構成する基本データ。1文字を示す。\n
* The basic data comprising the main data array. Includes only 1 character.
*/
typedef struct BYTE {
    unsigned int c; /**< 文字データ。\n 1 character data. */
}BYTE;

/**
* エディタ全般に関する。画面設定やデータ本体を含む。\n
* Being concerned with whole this editor, including display settings and main data.
*/
struct EDITORCONFIG {
    int cx;             /**< カーソルのx座標。\n Cursor x position in characters. */
    int cy;             /**< カーソルのy座標。\n Cursor y position in characters. */
    int rowoff;         /**< 行オフセット。\n Offset of row displayed. */// ex. if rowoff = 3, the first 3 rows will not be displayed.
    int coloff;         /**< 列オフセット。\n Offset of column displayed. */
    int screenrows;     /**< 画面が表示できる最大行数。\n Number of rows that we can show */
    int screencols;     /**< 画面が表示できる最大列数。\n Number of columns that we can show */
    int numbytes;       /**< 本体データの長さ。bytesに含まれる BYTE の数に一致するべきである。\n The length of main data. This should equal to the number of BYTE in bytes. */
    // int rawmode;     /* Is terminal raw mode enabled? */
    // erow *row;       /* Rows */
    int dirty;          /**< ファイルが編集されているが保存されてない場合は1、そうでなければ0。\n 1 means that the file is modified but not saved; otherwise 0. */ // 0 means the file has not beem modified, otherwise means modified.
    char *filename;     /**< 現在編集しているファイルの名前。\n The filename which is currently being edited. */
    // char statusmsg[80];
    // time_t statusmsg_time;
    // struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    BYTE *bytes;        /**< メインデータ。 BYTE の列。\n The main data; Array of BYTE. */

    int quittimes;      /**< Ctrl-Q が押された回数。\n The number of times Ctrl-Q pressed. */
    int width;          /**< 1行に表示するバイト(文字)数。\n number of bytes(characters) which are displayed in 1 row. */
    int editing;        /**< あるバイトが編集中であれば1、そうでなければ0。\n 1 means that a byte is being edited; otherwise 0. */ // 0 is false, otherwise are true;
    int idx;            /**< カーソルがある(画面ではなくメインデータ中の)位置。\n The position of cursor (of main data, not display) */

    char *statusmsg;    /**< ステータスメッセージ。\n Status message.*/

    int dispascii; // 0 NO header, 1 header (list of ascii characters)
    int iscolored; // 0 NOT colored, 1 colored
};

struct EDITORCONFIG E;


// -----functions-----
/**
* ターミナル画面の行数と列数を求める。\n Get numbers of columns and rows.
* @param ifd STDIN_FILENO
* @param ofd STDOUT_FILENO
* @param *rows &E.screenrows.
* @param *cols &E.screencols.
*/
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

/**
* ターミナル画面の行数と列数を求め、表示可能な行数を設定する。\n Set the number of printables rows.
*/
void updateWindowSize(void) {
    // printf("[called] updateWindowSize\n");
    if (getWindowSize(STDIN_FILENO, STDOUT_FILENO, &E.screenrows, &E.screencols) == -1) {
        perror("Unable to query the screen for size (columns / rows)");
        exit(1);
    }
    E.screenrows -= 4; /* Get space for status bar. */

    // Why does NOT this work ?
    if(E.dispascii){
        E.screenrows -= 8;
    }
}

/**
* この関数は初期化時に一度だけ実行される。\n
* This function is called only once; The first initialization.
*/
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

    E.quittimes = 0;
    E.editing   = 0;
    E.idx       = 0;
    E.dispascii = 0;
    E.iscolored = 0;

    updateWindowSize();
    printf("\x1B[%dS", E.screenrows);
}
