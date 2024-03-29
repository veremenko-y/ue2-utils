#ifdef __GNUC__
#include <obj.h>
#else
#include "obj.h"
#endif

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
#define LOBYTE 31

/* Pseudo instruction types */
#define ISPACE (1 + 32)
#define IBYTE (2 + 32)
#define IWORD (3 + 32)
/*#define ILONG (4 + 32)*/
#define ISET (8 + 32)
/*#define ICOMM (10 + 32)
#define ILSYM (12 + 32)
#define IFLOAT (14 + 32)
#define IDOUBLE (15 + 32)
#define IORG (16 + 32)
#define ISTAB (17 + 32)
#define IREG (18 + 32)
#define IINT (5 + 32)*/
#define IALIGN (13 + 32)
#define IDATA (6 + 32)
#define IGLOBAL (7 + 32)
#define ITEXT (9 + 32)
#define ILCOMM (11 + 32)
#define IREG (12 + 32)
/*#define IBREG (13 + 32)*/
#define ILOB (14 + 32)
#define IHIB (15 + 32)


#define ERR (-1)

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
