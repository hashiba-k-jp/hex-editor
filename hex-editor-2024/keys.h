enum KEYS{
    NUL    = 0x00,
    CTRL_G = 0x07, // BEL
    TAB    = 0x09,
    CTRL_Q = 0x11, // VT
    CTRL_S = 0x13,
    CTRL_X = 0x18,
    ESC    = 0x1B,
    DEL    = 0x7F,
    RET    = 0x0D, // RETURN (ENTER)

    KEY0   = 0xFF00,
    KEY1   = 0xFF01,
    KEY2   = 0xFF02,
    KEY3   = 0xFF03,
    KEY4   = 0xFF04,
    KEY5   = 0xFF05,
    KEY6   = 0xFF06,
    KEY7   = 0xFF07,
    KEY8   = 0xFF08,
    KEY9   = 0xFF09,
    KEYA   = 0xFF0A,
    KEYB   = 0xFF0B,
    KEYC   = 0xFF0C,
    KEYD   = 0xFF0D,
    KEYE   = 0xFF0E,
    KEYF   = 0xFF0F,

    UNEXPECTED,

    C_DEFAULT = 0xFF01,


    // DEL_KEY, HOME_KEY, END_KEY,
    // PAGE_UP, PAGE_DW,
};

enum ESC_SEQ{
    ARR_UP = 0x275B41,
    ARR_DW = 0x275B42,
    ARR_RI = 0x275B43,
    ARR_LE = 0x275B44,
    ARR_SHIFI_LE = 0x1B5B313B,  // ESC[1;
};
