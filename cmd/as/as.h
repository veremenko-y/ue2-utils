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
#define STOKBYTE 2
#define STOKWORD 3
#define STOKTEXT 4
#define STOKDATA 5
#define STOKBSS 6
#define STOKSET 7
#define STOKRES 8
#define STOKEXPORT 9
#define STOKIMPORT 10

/* Memory access types */
#define MNONE 0
#define MABS 1
#define MIMM 2

/* Token Types (in file after tokenizing) */
#define TOKINT 0 /* Integer token */
#define TOKSYM 1 /* Symbol token */
#define TOKSTR 2 /* String token */

#define RELOFFS 2 /* segout offset for reloc files */

/* as */
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