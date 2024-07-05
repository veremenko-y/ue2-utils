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
#define SYMUNDEF 0      /* undefined */
#define SYMABS (1 << 0) /* absolute value */
#define SYMREL (1 << 1) /* relative value */
#define SYMCONST (1 << 2) /* constant value (to be emmited into data segment) */

#define SYMTYPE (SYMABS | SYMREL | SYMCONST)

#define SYMEXPORT (1 << 7)
#define SYMCOEXPORT (1 << 6)

/* Relocation types */
#define RELCONST (1 << (7+8))
#define RELSEGSHIFT (5+8)
#define RELTEXT (1 << RELSEGSHIFT)
#define RELDATA (2 << RELSEGSHIFT)
#define RELBSS (3 << RELSEGSHIFT)
#define RELSEG (RELTEXT | RELDATA | RELBSS)


/* Segments */
#define SEGTEXT 0
#define SEGDATA 1
#define SEGBSS 2
#define SEGCONST 3

#define MAGIC_OBJ ((uint16_t)0x4d47)
#define MAGIC_RELOC ((uint16_t)0x4d48)
#define MAGIC_EXE ((uint16_t)0x4d49)

#define IS_MAGIC_VALID(x) (x == MAGIC_OBJ || x == MAGIC_RELOC || x == MAGIC_EXE)
#define IS_RELOCATABLE(x) (x == MAGIC_OBJ || x == MAGIC_RELOC)

struct header
{
    uint16_t magic;
    uint8_t hasrel;
    uint16_t textsize;
    uint16_t datasize;
    uint16_t bsssize;
    uint8_t consize;
    uint16_t symsize;
    uint16_t load;
} PACKED;

/* Offsets */

#define SYMOFFSET(hdr)                  \
    (                                   \
        (hdr.textsize + hdr.datasize) * \
            (hdr.hasrel ? 2 : 1) +      \
        sizeof(hdr))
    
#define CODEOFFSET(hdr) (sizeof(hdr))
#define RELCODEOFFSET(hdr) (sizeof(hdr) + hdr.textsize + hdr.datasize)



#endif