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

int save_file(EDITOR *EDITOR){
    // [todo] ファイルの保存
    // 指針としては
    // 1. uuidなどで衝突しないファイルを別名で作成
    // 2. 保存できた場合には
    // 3. 元のファイルを削除してファイル名の変更
    // こんな感じ？
    // ここでファイルのアクセス権限に注意!
    return -2;
}
