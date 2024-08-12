enum KEYS{
    NUL    = 0x0000,
    CTRL_G = 0x0007, // BEL
    TAB    = 0x0009,
    CTRL_Q = 0x0011, // VT
    CTRL_S = 0x0013,
    CTRL_X = 0x0018,
    ESC    = 0x001B,
    DEL    = 0x007F,
    RET    = 0x000D, // RETURN (ENTER)

    KEY0   = 0x0100,
    KEY1   = 0x0101,
    KEY2   = 0x0102,
    KEY3   = 0x0103,
    KEY4   = 0x0104,
    KEY5   = 0x0105,
    KEY6   = 0x0106,
    KEY7   = 0x0107,
    KEY8   = 0x0108,
    KEY9   = 0x0109,
    KEYA   = 0x010A,
    KEYB   = 0x010B,
    KEYC   = 0x010C,
    KEYD   = 0x010D,
    KEYE   = 0x010E,
    KEYF   = 0x010F,

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
