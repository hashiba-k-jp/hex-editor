#include "func.h"

struct t_data{ // [todo] 4-bit or 8-bit ?
    /* 0x00-0xFF */
    unsigned char data;

    // unsigned long long data; /* 0x00 - 0xFFFFFFFFFFFFFFFF */
    struct t_data *next;
    struct t_data *prev;
};

struct t_cursor{
    struct t_data *point;
    bool editing;
    bool head; /* true if cursor == EDITOR.head_data */
};

/* This function may be called only once. */
int read_file(EDITOR *EDITOR){
    EDITOR->cursor = malloc(sizeof(T_CURSOR));
    EDITOR->cursor->editing = false;

    int i = 0;
    char buf[1];
    int len;

    if(EDITOR->filesize <= -1){
        err(-1, "Invalid filesize");
    }

    EDITOR->head_data = malloc(sizeof(T_DATA));
    EDITOR->cursor->point = EDITOR->head_data;

    while(i < EDITOR->filesize){
        T_DATA *tmp = malloc(sizeof(T_DATA));
        if((len = read(EDITOR->fd, buf, 1)) != 1){
            err(len, "Failed to read char.");
        }else{
            tmp->data = buf[0];
        }
        EDITOR->cursor->point->next = tmp;
        tmp->prev = EDITOR->cursor->point;
        EDITOR->cursor->point = tmp;

        // printf("[debug] %6d / %6d [%02X]\r\n", i, EDITOR->filesize, tmp->data);
        i++;
    }

    // T_DATA *tail = malloc(sizeof(T_DATA));
    // EDITOR->cursor->point->next = tail;
    // tail->prev = EDITOR->cursor->point->next;

    EDITOR->head_data = EDITOR->head_data->next;
    EDITOR->cursor->point = EDITOR->head_data;

    EDITOR->cursor->head = true;
    return 0;
    /* if failed to read file, but err did not occur, this function should return -1. */
}
