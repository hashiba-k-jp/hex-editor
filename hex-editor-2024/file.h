/* This function may be called only once. */
int read_file(EDITOR *EDITOR){

    if(EDITOR->fd == -1){
        // [todo] ファイルが開かれていない場合の処理をここで

    }else{
        int i = 0;
        char *buf = (char *)malloc(sizeof(char));
        int len;
        T_CURSOR *reader = malloc(sizeof(T_CURSOR));
        reader->point = EDITOR->head_data;

        if(EDITOR->filesize <= -1){
            err(-1, "Invalid filesize");
        }

        while(i < EDITOR->filesize){
            T_DATA *tmp = malloc(sizeof(T_DATA));
            if((len = read(EDITOR->fd, buf, 1)) != 1){
                err(len, "Failed to read char.");
            }else{
                tmp->data = *buf;
            }
            reader->point->next = tmp;
            tmp->prev = reader->point;
            reader->point = tmp;
            // printf("[debug] %6d / %6d [%02X]\r\n", i, EDITOR->filesize, tmp->data);
            i++;
        }
        reader->point->next = EDITOR->tail_data;
        EDITOR->tail_data->prev = reader->point;
    }
    return 0;
}

int save_file(EDITOR *EDITOR){
    // [todo] ファイルの保存
    // 指針としては
    // 1. uuidなどで衝突しないファイルを別名で作成
    // 2. 保存できた場合には
    // 3. 元のファイルを削除してファイル名の変更
    // こんな感じ？
    // ここでファイルのアクセス権限に注意!

    if(access(EDITOR->filename, W_OK) != 0){
        err(errno, "Cannot overwrite to the file %s", EDITOR->filename);
    }else{
        EDITOR->fd = open(EDITOR->filename, O_WRONLY);
    }

    T_DATA *buffer = EDITOR->head_data;
    int len = 0;
    // printf("%d", EDITOR->fd);
    // exit(0);
    while((buffer = buffer->next) != EDITOR->tail_data){
        if((len = write(EDITOR->fd, buffer, 1)) != 1){
            err(errno, "Failed to write to the file.");
        }
    }

    return -2;
}
