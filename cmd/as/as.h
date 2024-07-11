#ifndef AS_H
#define AS_H

#include "dbg.h"

#ifdef __GNUC__
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#else
/* #include <pdp_compat.h> */
#endif

#include <obj.h>

/* Symbol token types (stored in struct sym) */
#define STOKID 0  /* label */
#define STOKINS 1 /* instruction */
#define STOKALIGN 2
#define STOKBYTE 3
#define STOKWORD 4
#define STOKTEXT 5
#define STOKDATA 6
#define STOKBSS 7
#define STOKSET 8
#define STOKRES 9
#define STOKGLOBL 10

#define SYMID(x) (x - symstart)

/* Memory access types */
#define MNONE 0
#define MABS 1
#define MIMM 2

/* Token Types (in file after tokenizing) */
#define TOKINT 0 /* Integer token */
#define TOKSYM 1 /* Symbol token */
#define TOKSTR 2 /* String token */

#define RELOFFS 2 /* segout offset for reloc files */

#define EXPINT 0 /* Integer */
#define EXPSYM 1 /* Symbol */
#define EXPEXP 2 /* Expression */
#define EXPNON 4 /* Expression */

#define EXPRSIZE 12 /* Max expressions */

struct expr
{
    uint8_t type;
    union {
        struct expr *expr;
        struct sym* sym;
        word_t val;
    } l;
    uint8_t op;
    struct expr *r;
} PACKED;


/* as */
extern FILE *fin;
extern FILE *fout;
extern FILE *segout[4];
extern uint8_t passno;
extern uint16_t lineno;
extern char *outname;

/* as0 */
extern struct sym *syms;
extern struct sym *cursym;
extern uint16_t cursymn;
extern uint16_t symscnt;
extern uint16_t symstart;
extern char strbuf[NAMESZ + 1];

/* as1 */
extern uint16_t segsize[4];

extern error(format, args);

#endif