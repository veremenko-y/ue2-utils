#include "as.h"
#include <stdio.h>
#include "cenas.h"

char type[] = {
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
TSEMI,      /* 3b ';' */
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
    {".lcomm", 0, ILCOMM, 0},
    {".align", 0, IALIGN, 0},
    /*{".comm", 0, ICOMM, 0},
    {".space", 0, ISPACE, 0},
    {".set", 0, ISET, 0},
    {".lsym", 0, ILSYM, 0},
    {".float", 0, IFLOAT, 0},
    {".double", 0, IDOUBLE, 0},
    {".org", 0, IORG, 0},
    {".stab", 0, ISTAB, 0}, */
    { "a", 0, IREG },
    { "b", 2, IREG },
    { "x", 4, IREG },
    { "y", 6, IREG },
    { "z", 8, IREG },
    { "s", 0xA, IREG },
    { "c", 0xC, IREG },
    { "p", 0xE, IREG },
    /*{ "alr", 0, IBREG },
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
    OP("hlt", 0x00, 0),
    OP("nop", 0x01, 0),
    OP("sf", 0x02, 0),
    OP("rf", 0x03, 0),
    OP("ei", 0x04, 0),
    OP("di", 0x05, 0),
    OP("sl", 0x06, 0),
    OP("rl", 0x07, 0),
    OP("cl", 0x08, 0),
    OP("rsr", 0x09, 0),
    OP("ri", 0x0A, 0),
    /*  Illegal 0x0B */
    OP("syn", 0x0C, 0),
    OP("pcx", 0x0D, 0),
    OP("dly", 0x0E, 0),
    OP("rsv", 0x0F, 0),
    OP("bl", 0x10, MREL),
    OP("bnl", 0x11, MREL),
    OP("bf", 0x12, MREL),
    OP("bnf", 0x13, MREL),
    OP("bz", 0x14, MREL),
    OP("bnz", 0x15, MREL),
    OP("bm", 0x16, MREL),
    OP("bp", 0x17, MREL),
    OP("bgz", 0x18, MREL),
    OP("ble", 0x19, MREL),
    OP("bs1", 0x1A, MREL),
    OP("bs2", 0x1B, MREL),
    OP("bs3", 0x1C, MREL),
    OP("bs4", 0x1D, MREL),
    OP("bi", 0x1E, MREL),
    OP("bck", 0x1F, MREL),
    OP("inrb", 0x20, MRC),
    OP("dcrb", 0x21, MRC),
    OP("clrb", 0x22, MRC),
    OP("ivrb", 0x23, MRC),
    OP("srrb", 0x24, MRC),
    OP("slrb", 0x25, MRC),
    OP("rrrb", 0x26, MRC),
    OP("rlrb", 0x27, MRC),
    OP("inab", 0x28, 0),
    OP("dcab", 0x29, 0),
    OP("clab", 0x2A, 0),
    OP("ivab", 0x2B, 0),
    OP("srab", 0x2C, 0),
    OP("slab", 0x2D, 0),
    /*  2E 	PageTable */
    /*  2F 	DMA */
    OP("inr", 0x30, MRC),
    OP("dcr", 0x31, MRC),
    OP("clr", 0x32, MRC),
    OP("ivr", 0x33, MRC),
    OP("srr", 0x34, MRC),
    OP("slr", 0x35, MRC),
    OP("rrr", 0x36, MRC),
    OP("rlr", 0x37, MRC),
    OP("ina", 0x38, 0),
    OP("dca", 0x39, 0),
    OP("cla", 0x3A, 0),
    OP("iva", 0x3B, 0),
    OP("sra", 0x3C, 0),
    OP("sla", 0x3D, 0),
    OP("inx", 0x3E, 0),
    OP("dcx", 0x3F, 0),
    OP("addb", 0x40, MRBRB),
    OP("subb", 0x41, MRBRB),
    OP("andb", 0x42, MRBRB),
    OP("orib", 0x43, MRBRB),
    OP("oreb", 0x44, MRBRB),
    OP("xfrb", 0x45, MRBRB),
    /*  OP("bignum", 0x46 ),*/
    OP("sabb", 0x49, 0),
    OP("nabb", 0x4A, 0),
    OP("xaxb", 0x4B, 0),
    OP("xayb", 0x4C, 0),
    OP("xabb", 0x4D, 0),
    OP("xazb", 0x4E, 0),
    OP("xasb", 0x4F, 0),
    OP("add", 0x50, MRAIR),
    OP("sub", 0x51, MRAIR),
    OP("and", 0x52, MRAIR),
    OP("ori", 0x53, MRAIR),
    OP("ore", 0x54, MRAIR),
    OP("xfr", 0x55, MRAIR),
    OP("eao", 0x56, 0),
    OP("dao", 0x57, 0),
    OP("aab", 0x58, 0),
    OP("sab", 0x59, 0),
    OP("nab", 0x5A, 0),
    OP("xax", 0x5B, 0),
    OP("xay", 0x5C, 0),
    OP("xab", 0x5D, 0),
    OP("xaz", 0x5E, 0),
    OP("xas", 0x5F, 0),
    OP("ldx", 0x60, MOD6),
    /*
    OP("ldx/", 0x61, MEMDIR),
    OP("ldx$", 0x62, MEMIND),
    OP("ldx", 0x63, MEMPC),
    OP("ldx*", 0x64, MEMPCRIND),
    OP("ldx+", 0x65, MEMINDX),
    OP("svc", 0x66, MEMIMMB),
    */
    /*  67 	MemBlock */
    /*  68 	STX+ A */
    /*  69 	STX+ B */
    /*  6A 	STX+ X */
    /*  6B 	STX+ Y */
    /*  6C 	STX+ Z */
    /*  6D 	STX+ S */
    /*  6E 	LST */
    /*  6F 	SST */
    /*  70 	Illegal */
    OP("jmp", 0x71, MOD5),
    /*
    OP("jmp$", 0x72, MEMIND),
    OP("jmp", 0x73, MEMPC),
    OP("jmp*", 0x74, MEMPCRIND),
    OP("jmp+", 0x75, MEMINDX),
    */
    /*  76 	EPE */
    /*  77 	MUL */
    /*  78 	DIV */
    OP("jsr", 0x79, MOD5),
    /*
    OP("jsr$", 0x7A, MEMIND),
    OP("jsr", 0x7B, MEMPC),
    OP("jsr*", 0x7C, MEMPCRIND),
    OP("jsr+", 0x7D, MEMINDX),
    */
    /*  7E 	STK */
    /*  7F 	POP */
    OP("ldab", 0x80, MOD14),
    /*
    OP("ldab/", 0x81, MEMDIR),
    OP("ldab$", 0x82, MEMIND),
    OP("ldab", 0x83, MEMPC),
    OP("ldab*", 0x84, MEMPCRIND),
    OP("ldab+", 0x85, MEMINDXALT),
    */
    /*  86 	DPE */
    /*  87 	Illegal */
    /*  88 	LDAB+ A */
    /*  89 	LDAB+ B */
    /*  8A 	LDAB+ X */
    /*  8B 	LDAB+ Y */
    /*  8C 	LDAB+ Z */
    /*  8D 	LDAB+ S */
    /*  8E 	LDAB+ C */
    /*  8F 	LDAB+ P */
    OP("lda", 0x90, MOD14),
    /*
    OP("lda/", 0x91, MEMDIR),
    OP("lda$", 0x92, MEMIND),
    OP("lda", 0x93, MEMPC),
    OP("lda*", 0x94, MEMPCRIND),
    OP("lda+", 0x95, MEMINDXALT),
    */
    /*  96 	SOP */
    /*  97 	Illegal */
    /*  98 	LDA+ A */
    /*  99 	LDA+ B */
    /*  9A 	LDA+ X */
    /*  9B 	LDA+ Y */
    /*  9C 	LDA+ Z */
    /*  9D 	LDA+ S */
    /*  9E 	LDA+ C */
    /*  9F 	LDA+ P */
    OP("stab", 0xA0, MOD14),
    /*
    OP("stab/", 0xA1, MEMDIR),
    OP("stab$", 0xA2, MEMIND),
    OP("stab", 0xA3, MEMPC),
    OP("stab*", 0xA4, MEMPCRIND),
    OP("stab+", 0xA5, MEMINDXALT),
    */
    /*  A6 	SEP */
    /*  A7 	Illegal */
    /*  A8 	STAB+ A */
    /*  A9 	STAB+ B */
    /*  AA 	STAB+ X */
    /*  AB 	STAB+ Y */
    /*  AC 	STAB+ Z */
    /*  AD 	STAB+ S */
    /*  AE 	STAB+ C */
    /*  AF 	STAB+ P */
    OP("sta", 0xB0, MOD14),
    /*
    OP("sta/", 0xB1, MEMDIR),
    OP("sta$", 0xB2, MEMIND),
    OP("sta", 0xB3, MEMPC),
    OP("sta*", 0xB4, MEMPCRIND),
    OP("sta+", 0xB5, MEMINDXALT),
    */
    /*  B6 	ECK */
    /*  B7 	Illegal */
    /*  B8 	STA+ A */
    /*  B9 	STA+ B */
    /*  BA 	STA+ X */
    /*  BB 	STA+ Y */
    /*  BC 	STA+ Z */
    /*  BD 	STA+ S */
    /*  BE 	STA+ C */
    /*  BF 	STA+ P */
    OP("ldbb", 0xC0, MOD14),
    /*
    OP("ldbb/", 0xC1, MEMDIR),
    OP("ldbb$", 0xC2, MEMIND),
    OP("ldbb", 0xC3, MEMPC),
    OP("ldbb*", 0xC4, MEMPCRIND),
    OP("ldbb+", 0xC5, MEMINDXALT),
    */
    /*  C6 	DCK */
    /*  C7 	Illegal */
    /*  C8 	LDBB+ A */
    /*  C9 	LDBB+ B */
    /*  CA 	LDBB+ X */
    /*  CB 	LDBB+ Y */
    /*  CC 	LDBB+ Z */
    /*  CD 	LDBB+ S */
    /*  CE 	LDBB+ C */
    /*  CF 	LDBB+ P */
    OP("ldb", 0xD0, MOD14),
    /*
    OP("ldb/", 0xD1, MEMDIR),
    OP("ldb$", 0xD2, MEMIND),
    OP("ldb", 0xD3, MEMPC),
    OP("ldb*", 0xD4, MEMPCRIND),
    OP("ldb+", 0xD5, MEMINDXALT),
    */
    /*  D6 	STR */
    /*  D7 	SAR */
    /*  D8 	LDB+ A */
    /*  D9 	LDB+ B */
    /*  DA 	LDB+ X */
    /*  DB 	LDB+ Y */
    /*  DC 	LDB+ Z */
    /*  DD 	LDB+ S */
    /*  DE 	LDB+ C */
    /*  DF 	LDB+ P */
    OP("stbb", 0xE0, MOD14),
    /*
    OP("stbb/", 0xE1, MEMDIR),
    OP("stbb$", 0xE2, MEMIND),
    OP("stbb", 0xE3, MEMPC),
    OP("stbb*", 0xE4, MEMPCRIND),
    OP("stbb+", 0xE5, MEMINDXALT),
    */
    /*  E6 	LAR */
    /*  E7 	Illegal */
    /*  E8  STBB+ */
    /*  E9  STBB+ */
    /*  EA  STBB+ */
    /*  EB  STBB+ */
    /*  EC  STBB+ */
    /*  ED  STBB+ */
    /*  EE  STBB+ */
    /*  EF  STBB+ */
    OP("stb", 0xF0, MOD14),
    /*
    OP("stb/", 0xF1, MEMDIR),
    OP("stb$", 0xF2, MEMIND),
    OP("stb", 0xF3, MEMPC),
    OP("stb*", 0xF4, MEMPCRIND),
    OP("stb+", 0xF5, MEMINDXALT),
    */
    /*  F6 	LIO/SIO */
    /*  F7 	MVL */
    /*  F8 	STB+ A */
    /*  F9 	STB+ B */
    /*  FA 	STB+ X */
    /*  FB 	STB+ Y */
    /*  FC 	STB+ Z */
    /*  FD 	STB+ S */
    /*  FE 	STB+ C */
    /*  FF 	STB+ P */
    0};

/* {Arguments, Size, Token [, Token ...], 0 [, Params ] } */
/* { ArgumentsBitfield, Size, Identifier # [ Token .. ], 0}*/
struct modexpr m_rc[] = {
    {0b10100000, 1, 0, IREG, TNL, 0}, /* ,c is optional*/
    {0b10100000, 1, 1, IREG, TCOMMA, IEXPR, TNL, 0}
};

struct modexpr m_arac[] = {
    {0b01010000, 3, 1, TDIV, TINT, TCOMMA, TINT, TNL, 0 },
    {0b01010100, 3, 2, TMINUS, IREG, TCOMMA, TINT, TCOMMA, TINT, TNL, 0 }
};

struct modexpr m_rbrb[] = {
    {0b10100000, 1, 0, IREG, TCOMMA, IREG, TNL, 0}
};

struct modexpr m_rair[] = {
    {0b10100000, 1, 1, IREG, TCOMMA, IREG, TNL, 0 },
    {0b01010100, 3, 2, TMINUS, IREG, TCOMMA, TINT, IREG, TNL, 0 }
};

struct modexpr m_m[] = {
    {0b01000000, 2, 0, TDIV, TINT, TNL, 0}
};

struct modexpr m_mod6[] = {
    {0b01000000, 2, 1, TEQUAL, IEXPR, TNL, 0},
    {0b01000000, 2, 2, TDIV, IEXPR, TNL, 0},
    {0b01000000, 2, 3, TIND, IEXPR, TNL, 0},
    {0b10000000, 1, 4, IEXPR, TNL, 0},
    {0b01000000, 1, 5, TMUL, IEXPR, TNL, 0},
    
    {0b01000000, 1, 6, TMINUS, IREG, TNL, 0},
    {0b01000000, 1, 7, TMINUS, IREG, TPLUS, TNL, 0},
    {0b01000000, 1, 8, TMINUS, IREG, TMINUS, TNL, 0},
    {0b00100000, 1, 9, TMINUS, TMUL, IREG, TNL, 0},
    {0b00100000, 1, 10, TMINUS, TMUL, IREG, TPLUS, TNL, 0},
    {0b00100000, 1, 11, TMINUS, TMUL, IREG, TMINUS, TNL, 0},

    {0b01010000, 2, 12, TMINUS, IREG, TCOMMA, IEXPR, TNL, 0},
    {0b01001000, 2, 13, TMINUS, IREG, TPLUS, TCOMMA, IEXPR, TNL, 0},
    {0b01001000, 2, 14, TMINUS, IREG, TMINUS, TCOMMA, IEXPR, TNL, 0},
    {0b00101000, 2, 15, TMINUS, TMUL, IREG, TCOMMA, IEXPR, TNL, 0},
    {0b00100100, 2, 16, TMINUS, TMUL, IREG, TPLUS, TCOMMA, IEXPR, TNL, 0},
    {0b00100100, 2, 17, TMINUS, TMUL, IREG, TMINUS, TCOMMA, IEXPR, TNL, 0},

    /* {IREG, 0, AREG}, */
    /* {IREG, TCOMMA, TINT, 0, AREGDISP}, */
    /* {IREG, TPLUS, 0, AREGPOI}, */
    /* {IREG, TPLUS, TCOMMA, TINT, 0, AREGDISPPOI}, */
    /* {IREG, TMINUS, 0, AREGPRD}, */
    /* {IREG, TMINUS, TCOMMA, TINT, 0, AREGPRDDISP}, */
    /* {TMUL, IREG, 0, AREGIND}, */
    /* {TMUL, IREG, TPLUS, 0, AREGPOIIND}, */
    /* {TMUL, IREG, TPLUS, TCOMMA, TINT, 0, AREGDISPPOIIND}, */
    /* {TMUL, IREG, TMINUS, 0, AREGPRDIND}, */
    /* {TMUL, IREG, TMINUS, TCOMMA, TINT, 0, AREGPRDDISPIND}, */
    /* {TMUL, IREG, TCOMMA, TINT, 0, AREGDISPIND}, */
};
