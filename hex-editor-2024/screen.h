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

char *header_msg(EDITOR *EDITOR){
    char *tmp;
    int len = EDITOR->ws->ws_col;
    char *header = malloc(sizeof(char)*(len+4+4));
    int i;

    memcpy(header, "\x1B[7m", 4);

    // header = realloc(header, sizeof(header) + len);
    tmp = "hex editor";
    memcpy(header+4, tmp, strlen(tmp));

    for(i = 4+strlen(tmp); i < len/2; i++){
        memcpy(header+i, " ", 1);
    }

    if(EDITOR->filename != NULL){
        memcpy(header+i, EDITOR->filename, strlen(EDITOR->filename));
        for(i+=strlen(EDITOR->filename); i < len+4; i++){
            memcpy(header+i, " ", 1);
        }
    }else{
        for(; i < len+4; i++){
            memcpy(header+i, " ", 1);
        }
    }

    memcpy(header+i, "\x1B[0m", 4);


    // printf("\x1B[7m[debug] {%d} This is a test header\x1B[%dG\x1B[0m\r\n", EDITOR->ws->ws_col, EDITOR->ws->ws_col);
    return header;
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
char **footer_msg(EDITOR *EDITOR){
    char **footer;
    footer = malloc(3); /* addr is 1 byte */

    char *footer_1 = malloc(sizeof(char)*(EDITOR->ws->ws_col+4+4));
    for(int i = 0; i < EDITOR->ws->ws_col+4+4; i++){ /* init footer_1 */
        footer_1[i] = ' ';
    }

    if(EDITOR->msgStatus == quitting){
        memcpy(footer_1,        "\x1B[7m", 4);
        memcpy(footer_1+4,      "File Name to write : ", 21);
        memcpy(footer_1+4+21,   EDITOR->filename, strlen(EDITOR->filename));
        memcpy(footer_1+EDITOR->ws->ws_col+4, "\x1B[0m", 4);
    }else{
        char *tmp;
        switch (EDITOR->msgStatus){
            case s_dafault:
                tmp = malloc(0);
                tmp = "";
                break;
            case saved:
                tmp = malloc(7);
                tmp = "[SAVED]";
                break;
            case canceled:
                tmp = malloc(10);
                tmp = "[CANCELED]";
                break;
            default:
                err(-1, "unexpected footer msg status.");
        }
        int tmp_pos = ((EDITOR->ws->ws_col+4+4) + strlen(tmp))/2;
        memcpy(footer_1+tmp_pos-4, "\x1B[7m", 4);
        memcpy(footer_1+tmp_pos, tmp, strlen(tmp));
        memcpy(footer_1+tmp_pos+strlen(tmp), "\x1B[0m", 4);
    }

    footer[0] = footer_1;
    footer[1] = "tmp2";
    footer[2] = "tmp3";

    return footer;
}

void init_screen(EDITOR *EDITOR){
    // printf("\x1B[%dS", EDITOR->ws->ws_row);
    printf("\x1B[2J");
    return;
}

int _fill_buf(char *dest, char *str, int offset, int n){
    strncpy(dest+offset, str, n);
    return offset+n;
}

void print_screen(EDITOR *EDITOR, char *header, char **footer){

    // [todo] ファイルサイズが大きいものは扱えない（アドレス表示が16進数8桁の固定値）
    // [ERROR] sprintfがおそらくセグフォルトを起こしてそう（sprintfだけコメントアウトして実行すると動きそうな気配）
        // -> asprintf にするとセグフォルトは起こらなくなるけど最後の表示がバグる ((null)とかになる)
    // [todo] まだ実行時間が遅い（画面サイズによっては受け入れ難いレベルに遅いのでやっぱりデータ構造を変えるしかない？）

    printf("\x1B[0;0H%s\r\n", header);

    int line_size = ((EDITOR->ws->ws_col-19) / 24)*8;
    int line_i = 0;
    int col_inline = 0;
    int curr_row, curr_col;
    bool already_located = false;

    /* 理論値は 11+1+line_size*(2+5+1+5+8)+4+3+2+4 = line_size*21+25
        [アドレス番号+:] 11
        [空白] 1
        [hex]  line_size*(2+5)    (5 = \x1b[47m)
        [空白] 4+3 (4 = \x1b[0m)
        [char] line_size*(1+5+8)  (\x1B[1m \x1B[0m 色を変えた文字)
        [end] 2+4 \r\n\x1b[0m
    */
    char *display_buf = malloc(sizeof(char)*(line_size*21+25));
    char *display_char_buf = malloc(sizeof(char)*(line_size*14));
    char *addr_buf = malloc(sizeof(char)*12);
    char *hex_buf  = malloc(sizeof(char)*3);
    char *char_buf = malloc(sizeof(char)*2);

    struct t_data *print_curr = EDITOR->head_data;
    print_curr = print_curr->next;

    if(EDITOR->head_data == EDITOR->cursor->point){
        // if EDITOR->cursor->point is header;
        curr_row = 0;
        curr_col = -1;
        already_located = true;
    }

    for(int i = 0; i < EDITOR->ws->ws_row-4; i++){
        memset(display_buf,      '\0', line_size*21+25);
        memset(display_char_buf, '\0', line_size*14);
        memset(addr_buf, '\0', 12);
        memset(hex_buf,  '\0', 3);
        memset(char_buf, '\0', 2);
        int filled_disp_buf = 0;
        int filled_disp_char_buf = 0;

        asprintf(&addr_buf, "0x%08x:", i*line_size);
        filled_disp_buf = _fill_buf(display_buf, addr_buf, filled_disp_buf, strlen(addr_buf));
        filled_disp_buf = _fill_buf(display_buf, " ", filled_disp_buf, 1);

        for(col_inline = 0; col_inline < line_size; col_inline++){
            if((print_curr == EDITOR->cursor->point) && !already_located){ /* For cursor */
                curr_row = i;
                curr_col = col_inline;
            }
            if(col_inline % 2 == 0){
                filled_disp_buf      = _fill_buf(display_buf,      "\x1b[47m", filled_disp_buf,      strlen("\x1b[47m"));
                filled_disp_char_buf = _fill_buf(display_char_buf, "\x1b[47m", filled_disp_char_buf, strlen("\x1b[47m"));
            }else{
                filled_disp_buf      = _fill_buf(display_buf,      "\x1b[0m",  filled_disp_buf,      strlen("\x1b[0m"));
                filled_disp_char_buf = _fill_buf(display_char_buf, "\x1b[0m",  filled_disp_char_buf, strlen("\x1b[0m"));
            }

            if(print_curr == NULL){
                filled_disp_buf      = _fill_buf(display_buf,      "  ", filled_disp_buf,      strlen("  "));
                filled_disp_char_buf = _fill_buf(display_char_buf, " ",  filled_disp_char_buf, strlen(" "));
            }else{
                asprintf(&hex_buf, "%02X", print_curr->data);
                filled_disp_buf = _fill_buf(display_buf, hex_buf, filled_disp_buf, strlen(hex_buf));

                if(0x20 <= print_curr->data && print_curr->data <= 0x7E){
                    asprintf(&char_buf, "%c", print_curr->data);
                    filled_disp_char_buf = _fill_buf(display_char_buf, char_buf,           filled_disp_char_buf, 1); // strlen(char_buf)
                }else{
                    filled_disp_char_buf = _fill_buf(display_char_buf, "\x1B[1m･\x1B[0m",  filled_disp_char_buf, strlen("\x1B[1m･\x1B[0m")); // strlen(char_buf)
                }
                print_curr = print_curr->next;
            }
        }

        filled_disp_buf = _fill_buf(display_buf, "   ", filled_disp_buf, strlen("   "));

        filled_disp_buf = _fill_buf(display_buf, display_char_buf, filled_disp_buf, filled_disp_char_buf);
        filled_disp_buf = _fill_buf(display_buf, "\r\n", filled_disp_buf, 2); // strlen("\r\n") = 2

        printf("%s", display_buf);
    }

    free(display_buf);
    free(display_char_buf);
    free(addr_buf);
    free(hex_buf);
    free(char_buf);

    printf("%s\r\n", footer[0]);
    printf("%s\r\n", footer[1]);
    printf("%s", footer[2]);

    /* display cursor */
    printf("a\x1B[%d;%dH", curr_row+1+1, (curr_col+1)*2+12+1);

    return;
}
