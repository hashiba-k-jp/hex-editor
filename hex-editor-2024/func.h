
// implicit declarations
// #define BLOCK_SIZE 0x100 // bytes


typedef struct editor EDITOR;       // config.h
typedef struct t_data T_DATA;       // file.h
typedef struct t_cursor T_CURSOR;   // file.h


// hex.c
int keyProcess(int, char*, EDITOR*, char**);
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
char* header_msg(EDITOR*);
char** footer_msg(EDITOR*);
void init_screen(EDITOR*);
void print_screen(EDITOR*, char*, char**);
