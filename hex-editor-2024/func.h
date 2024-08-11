
// implicit declarations
#define BLOCK_SIZE 0x100 // bytes

// terminalMode.h
void switch_cooked_mode(void);
void save_cooked_mode(void);
void switch_raw_mode(void);

// config.h
void print_usage(void);
void fin_program(void);
typedef struct editor EDITOR;

// file.h
typedef struct t_data T_DATA;
typedef struct t_cursor T_CURSOR;
int read_file(EDITOR*);

// screen.h
char* header_msg(EDITOR*);
char** footer_msg(EDITOR*);
void init_screen(EDITOR*);
void print_screen(EDITOR*, char*, char**);
