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
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>





int main(int argc, char *argv[]){

  initEditor();

  // int pot, longindex

  //getopt_long is standard c library
  //getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);

  // 色をつけるかどうかのコマンドライン引数？
  // while((opt = getopt_long(argc, argv, "ac", longopts, &longindex)) != -1){
  //   switch (opt) {
  //       case 'a':
  //           // E.dispascii = 1;
  //           break;
  //       case 'c':
  //           E.iscolored = 1;
  //           break;
  //   }
  // }

  // currently, the input file is required.
  // [todo] when run without any input, the program should make new file.
  if(argc < 2){
      fprintf(stderr,"Usage: ./hex <filename>\n");
      exit(1);
  }

  editorOpen();

  // [todo] when run without any input, the program should make new file.
  readfile(argv[argc-1]);



}
