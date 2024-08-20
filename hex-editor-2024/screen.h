enum msgStatus{
    s_dafault   = 0b00000000,
    saved       = 0b00000001,
    canceled    = 0b00000010,
    quitting    = 0b00000100,
};

void get_winsize(EDITOR *EDITOR){
    int succ_get_winsize;
    if ((succ_get_winsize = ioctl(1, TIOCGWINSZ, EDITOR->ws)) != 0){
        err(succ_get_winsize, "[ERROR] Failed to get window size.");
    }
    return;
}

int _ncpy_buf(char *dest, char *str, int offset){
    strncpy(dest+offset, str, strlen(str));
    return offset+strlen(str);
}

int _ncpy_buf2(char *dest, char *str, int offset, int n){
    strncpy(dest+offset, str, n);
    return offset+n;
}


int _fill_buf(char *dest, char c, int offset, int n){
    memset(dest+offset, c, n);
    return offset+n;
}

void header_msg(EDITOR *EDITOR){
    int offset = 0;
    *(EDITOR->header) = realloc(*(EDITOR->header), sizeof(char)*(EDITOR->ws->ws_col+4+4));

    offset      = _ncpy_buf(*(EDITOR->header), "\x1B[7m",           offset);
    offset      = _ncpy_buf(*(EDITOR->header), "HEX EDITOR",        offset);
    offset      = _fill_buf(*(EDITOR->header), '\x20',              offset, (EDITOR->ws->ws_col/2 - offset));
    if(EDITOR->filename != NULL){
        offset  = _ncpy_buf(*(EDITOR->header), EDITOR->filename,    offset);
    }
    offset      = _fill_buf(*(EDITOR->header), '\x20',              offset, ((EDITOR->ws->ws_col+4) - offset));
    offset      = _ncpy_buf(*(EDITOR->header), "\x1B[0m",           offset);
    // printf("\x1B[7m[debug] {%d} This is a test header\x1B[%dG\x1B[0m\r\n", EDITOR->ws->ws_col, EDITOR->ws->ws_col);
    return;
}


/*
    LIST OF STATUS:
        DEFAULT             : "",               default commands
        SAVED               : "[SAVED!]",       default commands
        QUITTING (No edit)  :                   (JUST QUIT, NO FOTTER MSG NEEDED.)
        QUITTING (edit)     : "Save change?",   Y/N/C
            -> Yes          : "File name:{%s}"  C
                -> Cancel   : "[Canceled]",     default commands
            -> No           :                   (Just quit)
            -> Cancel       : "[Canceled]",     default commands

*/
void footer_msg(EDITOR *EDITOR){
    // [todo] footer msg の作成

    int offset0 = 0;
    *((EDITOR->footer)+0) = realloc(*((EDITOR->footer)+0), sizeof(char)*(EDITOR->ws->ws_col+4+4));
    offset0 = _ncpy_buf(*((EDITOR->footer)+0), "\x1B[7m", offset0);
    offset0 = _fill_buf(*((EDITOR->footer)+0), '\x20',    offset0, (EDITOR->ws->ws_col+4 - offset0));
    offset0 = _ncpy_buf(*((EDITOR->footer)+0), "\x1B[0m", offset0);





    /* SPILED は適当な値であることに留意（特にoverflow!）*/
    int SPILED = 0;
    int offset1 = 0;
    *(EDITOR->footer+1) = realloc(*(EDITOR->footer+1), sizeof(char)*(EDITOR->ws->ws_col+SPILED));
    offset1 = _fill_buf(*(EDITOR->footer+1), '1', offset1, (EDITOR->ws->ws_col+SPILED - offset1));

    int offset2 = 0;
    *(EDITOR->footer+2) = realloc(*(EDITOR->footer+2), sizeof(char)*(EDITOR->ws->ws_col+SPILED));
    offset2 = _fill_buf(*(EDITOR->footer+2), '2', offset2, (EDITOR->ws->ws_col+SPILED - offset2));



    // char *footer_1 = malloc(sizeof(char)*(EDITOR->ws->ws_col+4+4));
    // for(int i = 0; i < EDITOR->ws->ws_col+4+4; i++){ /* init footer_1 */
    //     footer_1[i] = ' ';
    // }
    //
    // if(EDITOR->msgStatus == quitting){
    //     memcpy(footer_1,        "\x1B[7m", 4);
    //     memcpy(footer_1+4,      "File Name to write : ", 21);
    //     memcpy(footer_1+4+21,   EDITOR->filename, strlen(EDITOR->filename));
    //     memcpy(footer_1+EDITOR->ws->ws_col+4, "\x1B[0m", 4);
    // }else{
    //     char *tmp;
    //     switch (EDITOR->msgStatus){
    //         case s_dafault:
    //             tmp = malloc(0);
    //             tmp = "";
    //             break;
    //         case saved:
    //             tmp = malloc(7);
    //             tmp = "[SAVED]";
    //             break;
    //         case canceled:
    //             tmp = malloc(10);
    //             tmp = "[CANCELED]";
    //             break;
    //         default:
    //             err(-1, "unexpected footer msg status.");
    //     }
    //     int tmp_pos = ((EDITOR->ws->ws_col+4+4) + strlen(tmp))/2;
    //     memcpy(footer_1+tmp_pos-4, "\x1B[7m", 4);
    //     memcpy(footer_1+tmp_pos, tmp, strlen(tmp));
    //     memcpy(footer_1+tmp_pos+strlen(tmp), "\x1B[0m", 4);
    // }

    // EDITOR->footer[0] = footer_1;
    // EDITOR->footer[1] = "footer_1";
    // EDITOR->footer[2] = "footer_2";
    // footer[1] = "footer[1]";
    // footer[2] = "footer[2]";

    return;
}

void init_screen(EDITOR *EDITOR){
    // printf("\x1B[%dS", EDITOR->ws->ws_row);
    printf("\x1B[2J");
    return;
}

// void print_screen(EDITOR *EDITOR){
//
//     // [todo] ファイルサイズが大きいものは扱えない（アドレス表示が16進数8桁の固定値）
//     // [todo] bufを減らせる気がする（実行時間の削減に繋がるかは不明）
//     // [todo] まだ実行時間が遅い（画面サイズによっては受け入れ難いレベルに遅いのでやっぱりデータ構造を変えるしかない？）
//     //  -> _ncpy_buf とかの関数呼び出しを消せば若干早くなる？（ミスりそうなので最後にまとめて）
//     // [DONE][todo] カーソルの表示が若干不安定
//     //       -> 画面表示の最中はカーソルを消去（実行時間は遅いままなので微妙）
//
//
//     int line_size = ((EDITOR->ws->ws_col-19) / 24)*8;
//     int line_i = 0;
//     int col_inline = 0;
//     int curr_row, curr_col;
//     bool already_located = false;
//
//     /* 理論値は
//         <header> 14 + strlen(*(EDITOR->header))
//             [カーソル非表示] {6} \x1B[?25l
//             [カーソル修正] {6} \x1B[0;0H
//             [本文] {strlen(*(EDITOR->header))}
//             [改行] {2} \r\n
//         <本体部分> 11+1+line_size*(2+5+1+5+8)+4+3+2+4+9+9 = line_size*21+43
//             [アドレス番号+:] 11
//             [空白] 1
//             [hex]  line_size*(2+5)+9    (5 = \x1b[47m, 9 = cursor)
//             [空白] 4+3 (4 = \x1b[0m)
//             [char] line_size*(1+5+8)+9  (\x1B[1m \x1B[0m 色を変えた文字, 9=cursor)
//             [end] 2+4 \r\n\x1b[0m
//         <footer> strlen(*(EDITOR->footer+0)) + strlen(*(EDITOR->footer+1)) + strlen(*(EDITOR->footer+2)) + 4
//     */
//
//     int all_length = (14+strlen(*EDITOR->header)) + (line_size*21+43)*(EDITOR->ws->ws_row-4) + (strlen(*(EDITOR->footer+0))+strlen(*(EDITOR->footer+1))+strlen(*(EDITOR->footer+2))+4);
//     // char *display_buf_all   = (char *)malloc(sizeof(char)*all_length);
//     char *display_buf_all   = (char *)malloc(sizeof(char)*(line_size*21+43)*(EDITOR->ws->ws_row-4));
//     int offset_all = 0;
//
//     offset_all = _ncpy_buf(display_buf_all, "\x1B[?25l", offset_all);
//     offset_all = _ncpy_buf(display_buf_all, "\x1B[0;0H", offset_all);
//     offset_all = _ncpy_buf(display_buf_all, *(EDITOR->header), offset_all);
//     offset_all = _ncpy_buf(display_buf_all, "\r\n", offset_all);
//
//     // printf("\x1B[0;0H%s\r\n", *(EDITOR->header));
//
//     char *display_buf       = (char *)malloc(sizeof(char)*(line_size*21+43));
//     char *display_char_buf  = (char *)malloc(sizeof(char)*(line_size*14));
//     char *addr_buf = (char *)malloc(sizeof(char)*12);
//     char *hex_buf  = (char *)malloc(sizeof(char)*3);
//     char *char_buf = (char *)malloc(sizeof(char)*2);
//
//     struct t_data *print_curr = EDITOR->head_data;
//     print_curr = print_curr->next;
//
//     if(EDITOR->head_data == EDITOR->cursor->point){
//         // if EDITOR->cursor->point is header;
//         curr_row = 0;
//         curr_col = -1;
//         already_located = true;
//     }
//
//     for(int i = 0; i < EDITOR->ws->ws_row-4; i++){
//         memset(display_buf,      '\0', line_size*21+43);
//         memset(display_char_buf, '\0', line_size*14);
//         memset(addr_buf, '\0', 12);
//         memset(hex_buf,  '\0', 3);
//         memset(char_buf, '\0', 2);
//         int filled_disp_buf = 0;
//         int filled_disp_char_buf = 0;
//
//         asprintf(&addr_buf, "0x%08x:", i*line_size);
//         filled_disp_buf = _ncpy_buf(display_buf, addr_buf, filled_disp_buf);
//         filled_disp_buf = _ncpy_buf(display_buf, " ", filled_disp_buf);
//
//         for(col_inline = 0; col_inline < line_size; col_inline++){
//             if((print_curr == EDITOR->cursor->point) && !already_located){ /* For cursor */
//                 curr_row = i;
//                 curr_col = col_inline;
//             }
//             if(col_inline % 2 == 0){
//                 filled_disp_buf      = _ncpy_buf(display_buf,      "\x1b[47m", filled_disp_buf     );
//                 filled_disp_char_buf = _ncpy_buf(display_char_buf, "\x1b[47m", filled_disp_char_buf);
//             }else{
//                 filled_disp_buf      = _ncpy_buf(display_buf,      "\x1b[0m",  filled_disp_buf     );
//                 filled_disp_char_buf = _ncpy_buf(display_char_buf, "\x1b[0m",  filled_disp_char_buf);
//             }
//
//             if(print_curr == EDITOR->tail_data){
//                 filled_disp_buf      = _ncpy_buf(display_buf,      "  ", filled_disp_buf     );
//                 filled_disp_char_buf = _ncpy_buf(display_char_buf, " ",  filled_disp_char_buf);
//             }else{
//                 if(print_curr == EDITOR->cursor->point && EDITOR->cursor->editing){
//                     // asprintf(&hex_buf, "%1X_", (print_curr->data & 0xF0)>>4);
//                     asprintf(&hex_buf, "\x1B[36m%1X_\x1B[0m", (print_curr->data & 0xF0)>>4);
//                     // char_buf = "\x1B[1m_\x1B[0m";
//                     char_buf = "\x1B[36m_\x1B[0m";
//                 }else{
//                     asprintf(&hex_buf, "%02X", print_curr->data);
//                     if(0x20 <= print_curr->data && print_curr->data <= 0x7E){
//                         asprintf(&char_buf, "%c", print_curr->data);
//                     }else{
//                         char_buf = "\x1B[1m･\x1B[0m";
//                     }
//                 }
//                 filled_disp_buf = _ncpy_buf(display_buf, hex_buf, filled_disp_buf);
//                 filled_disp_char_buf = _ncpy_buf(display_char_buf, char_buf,           filled_disp_char_buf); // strlen(char_buf)
//
//
//                 print_curr = print_curr->next;
//             }
//         }
//
//         filled_disp_buf = _ncpy_buf(display_buf, "   ", filled_disp_buf);
//
//         filled_disp_buf = _ncpy_buf(display_buf, display_char_buf, filled_disp_buf);
//         filled_disp_buf = _ncpy_buf(display_buf, "\r\n", filled_disp_buf);
//
//         offset_all = _ncpy_buf(display_buf_all, display_buf, offset_all);
//     }
//
//     free(display_buf);
//     free(display_char_buf);
//     free(addr_buf);
//     free(hex_buf);
//     free(char_buf);
//
//     offset_all = _ncpy_buf(display_buf_all, *(EDITOR->footer+0), offset_all);
//     offset_all = _ncpy_buf(display_buf_all, "\r\n", offset_all);
//     offset_all = _ncpy_buf(display_buf_all, *(EDITOR->footer+1), offset_all);
//     offset_all = _ncpy_buf(display_buf_all, "\r\n", offset_all);
//     offset_all = _ncpy_buf(display_buf_all, *(EDITOR->footer+2), offset_all);
//
//
//     // printf("%s\r\n", *(EDITOR->footer+0));
//     // printf("%s\r\n", *(EDITOR->footer+1));
//     // printf("%s",     *(EDITOR->footer+2));
//
//     printf("%s", display_buf_all);
//
//     /* display cursor */
//     if(EDITOR->cursor->hex_input){
//         printf("\x1B[%d;%dH\x1B[?25h", (curr_row+1)+1, (curr_col+1)*2+12+1);
//     }else{
//         printf("\x1B[%d;%dH\x1B[?25h", (curr_row+1)+1, (curr_col+1)+(line_size*2)+3+12+1);
//     }
//
//     return;
// }

void print_screen2(EDITOR *EDITOR){
    int line_size = ((EDITOR->ws->ws_col-19) / 24)*8;

    /* 理論値は
        <header> 14 + strlen(*(EDITOR->header))
            [カーソル非表示] {6} \x1B[?25l
            [カーソル修正] {6} \x1B[0;0H
            [本文] {strlen(*(EDITOR->header))}
            [改行] {2} \r\n
        <本体部分> 11+1+line_size*(2+5+1+5+8)+4+3+2+4+9+9 = line_size*21+43
            [アドレス番号+:] 11
            [空白] 1
            [hex]  line_size*(2+5)+9    (5 = \x1b[47m, 9 = cursor)
            [空白] 4+3 (4 = \x1b[0m)
            [char] line_size*(1+5+8)+9  (\x1B[1m \x1B[0m 色を変えた文字, 9=cursor)
            [end] 2+4 \r\n\x1b[0m
        <footer> strlen(*(EDITOR->footer+0)) + strlen(*(EDITOR->footer+1)) + strlen(*(EDITOR->footer+2)) + 4
    */

    int all_length = (14+strlen(*EDITOR->header)) + (line_size*21+43)*(EDITOR->ws->ws_row-4) + (strlen(*(EDITOR->footer+0))+strlen(*(EDITOR->footer+1))+strlen(*(EDITOR->footer+2))+4);
    // char *display_buf_all   = (char *)malloc(sizeof(char)*all_length);
    char *screen   = (char *)malloc(sizeof(char)*(line_size*21+43)*(EDITOR->ws->ws_row-4));
    int offset_all = 0;

    /* print header */
    offset_all = _ncpy_buf(screen, "\x1B[?25l", offset_all);       // make cursor invisible
    offset_all = _ncpy_buf(screen, "\x1B[0;0H", offset_all);       // move cursor to upper-left corner
    offset_all = _ncpy_buf(screen, *(EDITOR->header), offset_all); // print header
    offset_all = _ncpy_buf(screen, "\r\n", offset_all);

    /* body */
    // char *display_buf       = (char *)malloc(sizeof(char)*(line_size*21+43));
    // char *display_char_buf  = (char *)malloc(sizeof(char)*(line_size*14));
    char **line_buf_all     = (char **)malloc(sizeof(char *)*(EDITOR->ws->ws_row-4));
    int *line_offset_all    = (int *)malloc(sizeof(int)*(EDITOR->ws->ws_row-4));

    int ADDR_LENGTH = 11; // [todo] この値は 0x00000000-0xFFFFFFFF (8桁)のアドレスまでしか表示できないことに留意

    for(int row = 0; row < (EDITOR->ws->ws_row-4); row++){
        char *line_buf  = (char *)malloc(sizeof(char)*(line_size*21+43));
        char *hex_buf   = (char *)malloc(sizeof(char)*(line_size*7+9));
        char *ascii_buf = (char *)malloc(sizeof(char)*(line_size*14+9));
        int line_i  = 0;
        int hex_i   = 0;
        int ascii_i = 0;

        memset(line_buf,      '\0', line_size*21+43);
        char *tmp = (char *)malloc(sizeof(char)*(ADDR_LENGTH+1));
        asprintf(&tmp, "0x%08x: ", row*line_size);
        line_i = _ncpy_buf2(line_buf, tmp, line_i, strlen(tmp));

        /* hex,ascii 部分 */
        for(int col = 0; col < line_size; col++){
            if(col % 2 == 0){
                // row_i = _ncpy_buf(*(line_buf_all+row), "\x1b[47m", row_i);
                hex_i   = _ncpy_buf(hex_buf,   "\x1b[47m", hex_i);
                ascii_i = _ncpy_buf(ascii_buf, "\x1b[47m", ascii_i);
            }else{
                // row_i = _ncpy_buf(*(line_buf_all+row), "\x1b[0m", row_i);
                hex_i   = _ncpy_buf(hex_buf,   "\x1b[0m", hex_i);
                ascii_i = _ncpy_buf(ascii_buf, "\x1b[0m", ascii_i);
            }

            char *tmp_hex   = (char *)malloc(sizeof(char)*3);
            char *tmp_ascii = (char *)malloc(sizeof(char)*2);
            if((row*line_size)+col < EDITOR->filesize){
                asprintf(&tmp_hex,   "%02X", *(EDITOR->file_data+(row*line_size)+col));
                // asprintf(&tmp_ascii, "%c",  *(EDITOR->file_data+(row*line_size)+col));
                if(0x20 <= *(EDITOR->file_data+(row*line_size)+col) && *(EDITOR->file_data+(row*line_size)+col) <= 0x7E){
                    asprintf(&tmp_ascii, "%c",  *(EDITOR->file_data+(row*line_size)+col));
                }else{
                    asprintf(&tmp_ascii, "\x1B[1m･\x1B[0m");
                }
            }else{
                asprintf(&tmp_hex,   "  ");
                asprintf(&tmp_ascii, " ");
            }
            hex_i   = _ncpy_buf(hex_buf,   tmp_hex,   hex_i);
            ascii_i = _ncpy_buf(ascii_buf, tmp_ascii, ascii_i);
            free(tmp_hex);
            free(tmp_ascii);
        }


        // int _ncpy_buf2(char *dest, char *str, int offset, int len){
        //     strncpy(dest+offset, str, len);
        //     return offset+len;
        // }

        // hex_buf, ascii_buf may contain '\0', thus strlen may not work
        line_i = _ncpy_buf2(line_buf, hex_buf, line_i, hex_i);
        line_i = _ncpy_buf2(line_buf, "   ", line_i, 3);
        line_i = _ncpy_buf2(line_buf, ascii_buf, line_i, ascii_i);
        line_i = _ncpy_buf2(line_buf, "\r\n", line_i, 2);
        *(line_buf_all+row)     = line_buf;
        *(line_offset_all+row)  = line_i;

        free(hex_buf);
        free(ascii_buf);
        offset_all = _ncpy_buf2(screen, *(line_buf_all+row), offset_all, *(line_offset_all+row));
    }

    offset_all = _ncpy_buf(screen, *(EDITOR->footer+0), offset_all);
    offset_all = _ncpy_buf(screen, "\r\n", offset_all);
    offset_all = _ncpy_buf(screen, *(EDITOR->footer+1), offset_all);
    offset_all = _ncpy_buf(screen, "\r\n", offset_all);
    offset_all = _ncpy_buf(screen, *(EDITOR->footer+2), offset_all);

    printf("%s", screen);
    //[todo] カーソル表示がまだ おそらくデータ構造にint型のカーソルを示すデータを持つ必要があるしそれが早そう

    return;
}
