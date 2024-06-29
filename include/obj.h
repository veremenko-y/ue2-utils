#ifndef OBJ_H
#define OBJ_H

#ifdef __GNUC__
#include <stdint.h>
#define PACKED __attribute__((packed))
#else
/* #include <pdp_compat.h> */
#endif

#define NAMESZ 8

typedef uint16_t word_t;

struct sym
{
    uint8_t name[NAMESZ + 1];
    uint8_t type;
    uint8_t segm;
    word_t value;
} PACKED;

/* Symbol types (for reloaction) */
#define SYMUNDEF 0 /* undefined */
#define SYMABS (1<<0) /* absolute value */
#define SYMREL (1<<1) /* relative value */

#define SYMTYPE (SYMABS | SYMREL)

#define SYMCONST (1 << 6) /* constant value (to be emmited into data segment) */
#define SYMEXPORT (1 << 7)

/* Relocation types */
/* #define RELFULL 0
#define RELLO 1
#define RELHI 2
#define RELCONST 3 */

/* Segments */
#define SEGTEXT 0
#define SEGDATA 1
#define SEGBSS 2
#define SEGCONST 3

#define MAGIC 'U'
struct header
{
    uint8_t magic;
    uint8_t hasrel;
    uint16_t textsize;
    uint16_t datasize;
    uint16_t bsssize;
    uint16_t symsize;
} PACKED;

#endif