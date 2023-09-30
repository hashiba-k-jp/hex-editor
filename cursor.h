/**
* @file cursor.h
* @brief カーソル操作に関するもの。\n For moving the cursor.
* @author HASHIBA Keishi
*/

enum DIR{
    ARR_UP, ARR_DW, ARR_RI, ARR_LE,
};

void moveCursor(int dir){
    // printf("[called] moveCursor\r\n");

    if(E.editing != 0){
        return;
    }

    /*

     * idx 0  1  2  3
     *     01 23 45 67

     * cursor 0 : E.dx = 0 or 1
     * cursor 1 : E.dx = 2 or 3 or 4
     * cursor 2 : E.dx = 5 or 6 or 7
     * cursor 3 : E.dx = 8 or 9 or 10
     * cursor = (E.dx+1)/3 ...?
     * if(E.dx == 11){cursor will be 4; DO NOT MOVE THE CORSOR.}
     * else{MOVE.}
     */

    int idxCX = (E.rowoff + E.cy) * 3 * E.width + E.cx;
    int idx = (idxCX+1)/3;
    int tmpRow = E.rowoff + E.cy;

    switch (dir) {
        case ARR_UP:
            // printf("[called] moveCursor -> ARR_UP\r\n");
            if(E.cy == 0){
                if(E.rowoff != 0){
                    E.idx -= E.width;
                    E.rowoff--;
                }else{
                    E.idx = 0;
                    E.rowoff = 0;
                    E.cx = 0;
                    E.cy = 0;
                }
            }else{
                E.idx -= E.width;
                E.cy--;
            }
            break;
        case ARR_DW:
            if(tmpRow == ((E.numbytes - 1) / E.width)){
                E.idx = E.numbytes;
                if(E.numbytes % E.width != 0){
                    E.cx = 3 * (E.numbytes % E.width) - 1;
                }else{
                    E.cx = 3 * E.width - 1;
                }
            }else if(E.numbytes <= idx+E.width){
                E.idx = E.numbytes;
                E.cy++;
                E.cx = 3 * (E.numbytes % E.width) - 1;
            }else{
                if(tmpRow < (E.numbytes / E.width)){
                    E.idx += E.width;
                    if(E.cy == E.screenrows - 1){
                        E.rowoff++; // Scroll 1 line.
                        // cursor position will NOT be changed.
                    }else{
                        E.cy++;
                    }
                }
            }
            break;
        case ARR_RI:
            // EOF
            if(E.numbytes <= idx){
                // EOF
                // do NOTHING; do NOT allow to move cursor into the EOF or later bytes.
            }else{
                if(E.cx == (E.width*3)-1){
                    // end of the line
                    if(tmpRow == E.numbytes/E.width){
                        // end of the file; the last row is filled.
                    }else{
                        // otherwise; the next (last) row has some bytes, but not filled.
                        // E.idx will not be changed.
                        E.cx = 0;
                        E.cy++;
                    }
                }else{
                    E.idx += 1;
                    if(E.cx == 0){
                        E.cx += 2;
                    }else{
                        E.cx += 3;
                    }
                }
            }
            break;
        case ARR_LE:
            if(idx <= 0){
                // begin of the file; cursor can not move to left.
            }else{
                if(E.cx == 0){
                    // begin of the line.
                    if(tmpRow == 0){
                        // begin of the file. (Can this case happen?)
                    }else{
                        // E.idx will not be changed.
                        E.cx = (E.width*3)-1;
                        E.cy--;
                    }
                }else{
                    E.idx -= 1;
                    E.cx -= 3;
                    if(E.cx < 0){
                        E.cx = 0;
                    }
                }
            }
            break;
        default:
            printf("\x1b[1m[!!!] Unexpected problem : E01\x1b[0m\r\n");
    }
}
