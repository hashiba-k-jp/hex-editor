#define BLOCK_SIZE 0x100
// used in file.h

typedef struct editor{
    char *filename;
    int fd;             // file descriptor for the input file.
    bool isColored;
    struct winsize *ws; // defined in <sys/ioctl.h>
    int curr_row;       // CURRent row
    int curr_col;       // CURRent row
    int filesize;
    bool isEdited;
    char **footer;
    char **header;

    unsigned char *file_data;
    int num_byte;
    struct t_data *head_data;   // to be UN-USED
    struct t_data *tail_data;   // to be UN-USED
    struct t_cursor *cursor;    // to be UN-USED

    int curr_footer_status;
    int prev_footer_status;

    unsigned int msgStatus;

}EDITOR;

typedef struct t_data{ // [todo] 4-bit or 8-bit ?
    /* 0x00-0xFF */
    unsigned char data;

    // unsigned long long data; /* 0x00 - 0xFFFFFFFFFFFFFFFF */
    struct t_data *next;
    struct t_data *prev;
}T_DATA;

typedef struct t_cursor{
    struct t_data *point;
    bool editing;
    // bool head; /* true if cursor == EDITOR.head_data */
    bool hex_input;
}T_CURSOR;
enum input_window{ HEX_INPUT, ASCII_INPUT, };

// hex.c
int keyProcess(int, char*, EDITOR*);
int keyInput();
void _move_cursor(EDITOR*, int, int);

// terminalMode.h
void switch_cooked_mode(void);
void save_cooked_mode(void);
void switch_raw_mode(void);

// config.h
void print_usage(void);
void fin_program(void);

// file.h
int read_file(EDITOR*);
int save_file(EDITOR*);

// screen.h
void get_winsize(EDITOR*);
void header_msg(EDITOR*);
void footer_msg(EDITOR*);
void init_screen(EDITOR*);
// void print_screen(EDITOR*);
void print_screen2(EDITOR*);
