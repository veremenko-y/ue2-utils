#include <obj.h>

#define NLOC 1	   /* number of location ctrs */

struct instab
{
	uint8_t name[NCPS];
	uint8_t opcode; /* must have same offset as symtab.type */
	uint8_t tag;	 /* yacc-coded nargs (INST0, ...) or token # (IALIGN, ...) */
	uint8_t access;
} PACKED;

struct exp
{
	uint8_t xtype;
	uint8_t xloc;
	uint16_t xvalue;
	uint16_t yvalue; /* 2nd half of double floating */
	struct symtab *xname;
} PACKED;

struct modexpr {
	uint8_t args;
	uint8_t size;
	uint8_t opt1;
	uint8_t tokens[12];
} PACKED;

/* Token types */
#define TEOF 0
#define TSPACE 1
#define TNL 2
#define TDQ 3
#define TCOMM 4
#define TIND 5
#define TAND 6
#define TSQ 7
#define TLP 8
#define TRP 9
#define TMUL 10
#define TPLUS 11
#define TCOMMA 12
#define TMINUS 13
#define TALPH 14
#define TDIV 15
#define TDIG 16
#define TCOLON 17
#define TSEMI 18
#define TLSH 19
#define TEQUAL 20
#define TRSH 21
#define TLB 22
#define TRB 23
#define TXOR 24
#define TIOR 25
#define TTILDE 26

#define TINT 28
#define TSTR 29
#define TNAME 30
#define TINTSTR 31

#define IEXPR 32
/* Pseudo instruction types */
#define ISPACE ((char)(1 + 32))
#define IBYTE ((char)(2 + 32))
#define IWORD ((char)(3 + 32))
/*#define ILONG ((char)(4 + 32))*/
#define ISET ((char)(8 + 32))
/*#define ICOMM ((char)(10 + 32))
#define ILSYM ((char)(12 + 32))
#define IFLOAT ((char)(14 + 32))
#define IDOUBLE ((char)(15 + 32))
#define IORG ((char)(16 + 32))
#define ISTAB ((char)(17 + 32))
#define IREG ((char)(18 + 32))
#define IINT ((char)(5 + 32))*/
#define IALIGN ((char)(13 + 32))
#define IDATA ((char)(6 + 32))
#define IGLOBAL ((char)(7 + 32))
#define ITEXT ((char)(9 + 32))
#define ILCOMM ((char)(11 + 32))
#define IREG ((char)(12 + 32))
/*#define IBREG ((char)(13 + 32))*/


/*#define ERR (-1)*/
/*#define NBPW 32 *//* Bits per word */

/* reference types for loader */
#define PCREL 1
#define LEN1 2
#define LEN2 4
#define LEN4 6
#define LEN8 8

extern int type[129];
extern struct instab instab[];
