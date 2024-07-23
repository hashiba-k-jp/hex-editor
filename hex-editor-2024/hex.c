/**
* @file hex.c
* @brief This is the main file.
* @author HASHIBA Keishi
*/


// #include <termios.h>
// #include <stdlib.h>
#include <stdio.h>
// #include <stdint.h>
// #include <errno.h>
// #include <string.h>
// #include <ctype.h>
// #include <sys/ioctl.h>
#include <unistd.h>  // getopt()
// #include <getopt.h>
#include <err.h>    // warn



#include <stdbool.h>



int main(int argc, char *argv[]){

    bool test_color = false;
    char* input_file = NULL; // This should be stored in the other var.
    // https://www.mm2d.net/main/prog/c/getopt-02.html
    // refer -> man 3 getopt
    int ch;


    while ((ch = getopt(argc, argv, "i:cm")) != -1) {
        switch (ch) {
            case 'c':
                test_color = true;
                break;
            case 'i':
                if(input_file == NULL){
                    input_file = optarg;
                }else{
                    warn("[WARN]too many input files; run \"%s -m\"", argv[0]);
                }
                break;
            case 'm':
                // usage();
                return 0;
        }
    }

    printf("%s\n", input_file);


     //         switch (ch) {
     //         case 'b':
     //                 bflag = 1;
     //                 break;
     //         case 'f':
     //                 if ((fd = open(optarg, O_RDONLY, 0)) < 0) {
     //                         (void)fprintf(stderr,
     //                             "myname: %s: %s\n", optarg, strerror(errno));
     //                         exit(1);
     //                 }
     //                 break;
     //         case '?':
     //         default:
     //                 usage();
     //         }
     // }


    return 0;


/*
  // initEditor();

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
*/


}
