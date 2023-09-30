/**
* @file editdata.h
* @brief データの編集に関するもの。挿入･削除操作後に画面上でカーソル操作を行うため、 cursor.h を要する。\n For editing the data. cursor.h is required because the cursor will be moved after inserting and/or deleting data.
* @author HASHIBA Keishi
*/

/**
* この関数はデータに文字を追加する。\n
* This function is used to insert characters into the data.
*
* @param at 追加する位置を示す。\n The location of inserting.
* @param ca 追加する文字バイト。\n The byte data of inserting character.
*/
void editorInsertByte(int at, unsigned int ca){
    // printf("[called] editorInsertByte\n");

    E.bytes = realloc(E.bytes, sizeof(BYTE) * (E.numbytes + 1));

    E.bytes[at].c = ca;
    E.numbytes++;
}


/**
* この関数はメインデータにデータを挿入する。挿入位置は EDITORCONFIG.idx から計算される。\n
* This function will insert data. The position is calculated by EDITORCONFIG.idx.
* @param c 挿入データ。\n Inserting data.
*/
void insertChar(int c){
    /*

     * idx 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     *     01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ef
     *            ~~
     *     if cursor is located right after '5', idx should be 3.

     * idx 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     *     01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ef

     * cursor 0 : E.dx = 0
     * cursor 1 : E.dx = 2 or 3
     * cursor 2 : E.dx = 5 or 6
     * cursor = (E.dx+1)/3 ...?
     * E.dx max = 47
     */

    int idxCX = (E.rowoff + E.cy) * 3 * E.width + E.cx;
    int idx = (idxCX+1)/3;

    int hexHalf;

    if(0x30 <= c && c <= 0x39){
        hexHalf = c - 0x30;
    }else if(0x61 <= c && c <= 0x66){
        hexHalf = c - 0x57;
    }else if(0x41 <= c && c <= 0x46){
        hexHalf = c - 0x37;
    }

    if(E.idx > E.numbytes){
        // This case should not be happned.
        // ... and I think this case can not happen.
        E.idx = E.numbytes;
    }

    if(E.editing){
        // editing
        int hexFull = E.bytes[E.idx-1].c + hexHalf;
        E.bytes[E.idx-1].c = hexFull;
        E.editing = 0;
    }else{
        // not editing.
        memcpy(E.bytes+E.idx+1, E.bytes+E.idx, sizeof(BYTE)*(E.numbytes-idx));
        editorInsertByte(E.idx, hexHalf*0x10);
        if(E.cx == (3*E.width) - 1){
            moveCursor(ARR_RI);
        }
        moveCursor(ARR_RI);
        E.editing = 1;
    }
}

/**
* この関数はメインデータからデータを削除する。削除されるデータの位置は EDITORCONFIG.idx から計算される。\n
* This function will delete data. The delete data will be calculated by EDITORCONFIG.idx.
*/
void deleteChar(void){
    if(E.editing){
        // editing.
        memcpy(E.bytes+E.idx-1, E.bytes+E.idx, sizeof(BYTE)*(E.numbytes-E.idx));
        E.numbytes--;
        E.editing = 0;
        moveCursor(ARR_LE);
        if(E.idx % E.width == 0){
            moveCursor(ARR_LE);
        }
    }else{
        // not editing.
        if(E.idx == 0){return;}
        // moveCursor(ARR_LE);
        E.bytes[E.idx-1].c = (E.bytes[E.idx-1].c / 0x10) * 0x10;
        E.editing = 1;
    }
}
