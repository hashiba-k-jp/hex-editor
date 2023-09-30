/**
* @file terminalmode.h
* @brief ターミナルモードに関するもの。\n For changeing terminal modes.
* @author HASHIBA Keishi
*/

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

/** In order to restore at exit.*/
static struct termios orig_termios;

/**
* この関数はRawモードを無効にしてプログラム実行時に戻す。Cookedモードにする関数ではないことに留意。\n
* This function will disable raw mode and return it to as it was run. Note that it will NOT change it to cooked mode.
* @param fd STDIN_FILENO
*/
void disableRawMode(int fd){
    tcsetattr(fd, TCSAFLUSH, &orig_termios);
    printf("\x1B[0K\x1B[1m\n");
    // printf("[debug] switched to cooked mode.\x1B[0m\n");
}

/**
* この関数は終了時に実行される。 disableRawMode() を実行する。\n
* This function will be called when the programm has been exited, and will call disableRawMode().
*/
void editorAtExit(void) {
    disableRawMode(STDIN_FILENO);
    printf("\x1B[2J\x1B[0;0H");
}

/**
* この関数は現在のモード (raw / cooked) を orig_termios に保存し、rawモードを有効化する。\n
* This function will save current mode (raw / cooked) to variable orig_termios, and enable raw mode.
* @param fd STDIN_FILENO
*/
int enableRawMode(int fd){
    // printf("[called] enableRawMode\n");

    struct termios raw;

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
    // printf("\x1B[1m[debug] switched to RAWMODE.\x1B[0m\r\n");
    return 0;

fatal:
    errno = ENOTTY;
    // printf("\x1B[1m[ !!! ] FAILED TO SWITCH TO RAWMODE !!!\x1B[0m\r\n");
    return -1;
}
