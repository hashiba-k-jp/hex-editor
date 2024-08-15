enum msgStatus{
    s_dafault   = 0b00000000,
    saved       = 0b00000001,
    canceled    = 0b00000010,
    quitting    = 0b00000100,
};


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

void print_screen(EDITOR *EDITOR, char *header, char **footer){
    // [todo] 実行時間がやけに遅い
    // データ構造を根本的に変える必要がありそう？
    // 例えばある程度の大きさの配列にしてmemcpyすりゃforなり->nextなりの必要はなくなる。
    // しかしそうするとinsert/deleteの時にmemmovとか必要になりそう?
    printf("\x1B[0;0H");

    printf("%s\r\n", header);

    EDITOR->line_size = ((EDITOR->ws->ws_col-19) / 24)*8;
    int line_i = 0;
    int col_inline = 0;
    int curr_row, curr_col;

    unsigned char *buf = malloc(sizeof(char)*EDITOR->line_size);
    memset(buf, 0xFF, EDITOR->line_size);
    struct t_data *print_curr = EDITOR->head_data;


    for(int i = 0; i < EDITOR->ws->ws_row-4; i++){
        for(col_inline = 0; col_inline < EDITOR->line_size; col_inline++){
            if(print_curr == EDITOR->cursor->point){ /* For cursor */
                curr_row = i;
                curr_col = col_inline;
            }
            if(print_curr == NULL){
                break;
            }else{
                *(buf+col_inline) = print_curr->data;
                print_curr = print_curr->next;
            }
        }

        // [todo] too big file cannot be handeled with this code.
        // if(0xFFFFFFFF <= EDITOR->filesize){
        //     printf("0x%012x: ", i*EDITOR->line_size);
        // }else{
            printf("0x%08x: ", i*EDITOR->line_size);
        // }

        for(int j = 0; j < EDITOR->line_size; j++){
            if(j % 2 == 0){ printf("\x1b[47m"); }
            if(j < col_inline){
                printf("%02X", buf[j]);
            }else{
                printf("  ");
            }
            printf("\x1b[0m");
        }

        printf("   ");
        for(int j = 0; j < EDITOR->line_size; j++){
            if(j % 2 == 0){ printf("\x1b[47m"); }
            if(j < col_inline){
                if(0x20 <= buf[j] && buf[j] <= 0x7E){
                    printf("%c", buf[j]);
                }else{
                    printf("\x1B[1m･\x1B[0m");
                }
            }else{
                printf(" ");
            }
            printf("\x1b[0m");
        }
        printf("\r\n");
    }

    printf("%s\r\n", footer[0]);
    printf("%s\r\n", footer[1]);
    printf("%s", footer[2]);

    /* cursor */
    if(curr_col != -1){
        printf("a\x1B[%d;%dH", curr_row+1+1, (curr_col+1)*2+12+1);
    }

    return;
}
