#include "as.h"
#include "ue2.h"
#include <stdio.h>

int type[] = {
EOF,
TSPACE,     /* 0 'NUL' */
0,      /* 1 'SOH' */
0,      /* 2 'STX' */
0,      /* 3 'ETX' */
0,      /* 4 'EOT' */
0,      /* 5 'ENQ' */
0,      /* 6 'ACK' */
0,      /* 7 'BEL' */
0,      /* 8 'BS' */
TSPACE,     /* 9 'HT' */
TNL,        /* a 'LF' */
0,      /* b 'VT' */
0,      /* c 'FF' */
TSPACE,     /* d 'CR' */
0,      /* e 'SO' */
0,      /* f 'SI' */
0,      /* 10 'DLE' */
0,      /* 11 'DC1' */
0,      /* 12 'DC2' */
0,      /* 13 'DC3' */
0,      /* 14 'DC4' */
0,      /* 15 'NAK' */
0,      /* 16 'SYN' */
0,      /* 17 'ETB' */
0,      /* 18 'CAN' */
0,      /* 19 'EM' */
0,      /* 1a 'SUB' */
0,      /* 1b 'ESC' */
0,      /* 1c 'FS' */
0,      /* 1d 'GS' */
0,      /* 1e 'RS' */
0,      /* 1f 'US' */
TSPACE,     /* 20 'space' */
0,      /* 21 '!' */
TDQ,        /* 22 '"' */
TCOMM,      /* 23 '#' */
TIND,       /* 24 '$' */
0,      /* 25 '%' */
TAND,       /* 26 '&' */
TSQ,        /* 27 ''' */
TLP,        /* 28 '(' */
TRP,        /* 29 ')' */
TMUL,       /* 2a '*' */
TPLUS,      /* 2b '+' */
TCOMMA,     /* 2c ',' */
TMINUS,     /* 2d '-' */
TALPH,     /* 2e '.' */
TDIV,       /* 2f '/' */
TDIG,       /* 30 '0' */
TDIG,       /* 31 '1' */
TDIG,       /* 32 '2' */
TDIG,       /* 33 '3' */
TDIG,       /* 34 '4' */
TDIG,       /* 35 '5' */
TDIG,       /* 36 '6' */
TDIG,       /* 37 '7' */
TDIG,       /* 38 '8' */
TDIG,       /* 39 '9' */
TCOLON,     /* 3a ':' */
TCOMM,      /* 3b ';' */
TLSH,       /* 3c '<' */
TEQUAL,     /* 3d '=' */
TRSH,       /* 3e '>' */
0,      /* 3f '?' */
0,      /* 40 '@' */
TALPH,      /* 41 'A' */
TALPH,      /* 42 'B' */
TALPH,      /* 43 'C' */
TALPH,      /* 44 'D' */
TALPH,      /* 45 'E' */
TALPH,      /* 46 'F' */
TALPH,      /* 47 'G' */
TALPH,      /* 48 'H' */
TALPH,      /* 49 'I' */
TALPH,      /* 4a 'J' */
TALPH,      /* 4b 'K' */
TALPH,      /* 4c 'L' */
TALPH,      /* 4d 'M' */
TALPH,      /* 4e 'N' */
TALPH,      /* 4f 'O' */
TALPH,      /* 50 'P' */
TALPH,      /* 51 'Q' */
TALPH,      /* 52 'R' */
TALPH,      /* 53 'S' */
TALPH,      /* 54 'T' */
TALPH,      /* 55 'U' */
TALPH,      /* 56 'V' */
TALPH,      /* 57 'W' */
TALPH,      /* 58 'X' */
TALPH,      /* 59 'Y' */
TALPH,      /* 5a 'Z' */
TLB,        /* 5b '[' */
0,      /* 5c '\' */
TRB,        /* 5d ']' */
TXOR,       /* 5e '^' */
TALPH,      /* 5f '_' */
0,      /* 60 '`' */
TALPH,      /* 61 'a' */
TALPH,      /* 62 'b' */
TALPH,      /* 63 'c' */
TALPH,      /* 64 'd' */
TALPH,      /* 65 'e' */
TALPH,      /* 66 'f' */
TALPH,      /* 67 'g' */
TALPH,      /* 68 'h' */
TALPH,      /* 69 'i' */
TALPH,      /* 6a 'j' */
TALPH,      /* 6b 'k' */
TALPH,      /* 6c 'l' */
TALPH,      /* 6d 'm' */
TALPH,      /* 6e 'n' */
TALPH,      /* 6f 'o' */
TALPH,      /* 70 'p' */
TALPH,      /* 71 'q' */
TALPH,      /* 72 'r' */
TALPH,      /* 73 's' */
TALPH,      /* 74 't' */
TALPH,      /* 75 'u' */
TALPH,      /* 76 'v' */
TALPH,      /* 77 'w' */
TALPH,      /* 78 'x' */
TALPH,      /* 79 'y' */
TALPH,      /* 7a 'z' */
0,      /* 7b '{' */
TIOR,       /* 7c '|' */
0,      /* 7d '}' */
TTILDE,     /* 7e '~' */
0,      /* 7f 'DEL' */
};

#define OP(name, opcode, access)      \
    {                                               \
        name, opcode, TINTSTR, access \
    }

struct instab instab[] = {
    {".byte", 0, IBYTE, 0},
    {".word", 0, IWORD, 0},
    /*{".long", 0, ILONG, 0},
    {".int", 0, IINT, 0},
    */
    {".data", 0, IDATA, 0},
    {".globl", 0, IGLOBAL, 0},
    {".text", 0, ITEXT, 0},
    {".set", 0, ISET, 0},
/*    {".lcomm", 0, ILCOMM, 0},*/
    {".align", 0, IALIGN, 0},
    /*{".comm", 0, ICOMM, 0},
    {".space", 0, ISPACE, 0},
    {".lsym", 0, ILSYM, 0},
    {".float", 0, IFLOAT, 0},
    {".double", 0, IDOUBLE, 0},
    {".org", 0, IORG, 0},
    {".stab", 0, ISTAB, 0}, */
    /*{ "a", 0, IREG },
    { "b", 2, IREG },
    { "x", 4, IREG },
    { "y", 6, IREG },
    { "z", 8, IREG },
    { "s", 0xA, IREG },
    { "c", 0xC, IREG },
    { "p", 0xE, IREG },
    { "alr", 0, IBREG },
    { "blr", 1, IBREG },
    { "ylr", 3, IBREG },
    { "zlr", 4, IBREG },
    { "slr", 5, IBREG },
    { "aur", 0, IREG },
    { "bur", 1, IREG },
    { "yur", 3, IREG },
    { "zur", 4, IREG },
    { "sur", 5, IREG },
    */
    OP("bz", 0, MABS),
    OP("bl", 1, MABS),
    OP("lda", 2, MIMM),
    OP("ldl", 3, MABS),
    OP("stl", 4, MABS),
    OP("jsr", 5, MABS),
    OP("strh", 6, MABS),
    OP("strl", 7, MABS),
    OP("rsr", 8, MNONE),
    OP("scf", 9, MIMM),
    OP("adc", 10, MABS),
    OP("cmp", 11, MABS),
    OP("ror", 12, MABS),
    OP("nand", 13, MABS),
    OP("ori", 14, MABS),
    OP("ore", 15, MABS),
    0};

/* {Arguments, Size, Token [, Token ...], 0 [, Params ] } */
/* { ArgumentsBitfield, Size, Identifier # [ Token .. ], 0}*/
/*struct modexpr m_rc[] = {
    {0b10000000, 1, 1, IEXPR, TNL, 0}
};*/

