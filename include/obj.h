#ifndef OBJ_H
#define OBJ_H

#ifdef __GNUC__
#include <stdint.h>
#define PACKED __attribute__((packed))
#else
/* #include <pdp_compat.h> */
#endif

#define NAMESZ 16

#ifdef BITS12

typedef uint16_t addr_t;
typedef uint16_t data_t;
typedef uint32_t irword_t;
#define IRSIZE 3
#define IROFFSET 1
#define IRSHIFT 16
#define IRMASK (0xf << IRSHIFT)
#define RELSHIFT (IRSHIFT + 4)
#define ADDRSIZE (1 << 16)
#define ADDRMAX (ADDRSIZE - 1)
#define DATASIZE (1 << 8)
#define DATAMAX  (DATASIZE - 1)

#else 

typedef uint16_t addr_t;
typedef uint16_t data_t;
typedef uint16_t irword_t;
#define IRSIZE 2
#define IROFFSET 0
#define IRSHIFT 12
#define IRMASK (0xf << IRSHIFT)
#define RELSHIFT (IRSHIFT + 4)
#define ADDRSIZE (1 << 12)
#define ADDRMAX (ADDRSIZE - 1)
#define DATASIZE (1 << 8)
#define DATAMAX  (DATASIZE - 1)

#endif

/* Segments */
#define SEGTEXT 0
#define SEGDATA 1
#define SEGBSS 2
#define SEGCONST 3
#define SIZERELOFF 4

struct sym
{
    uint8_t name[NAMESZ + 1];
    uint8_t type;
    uint8_t segm;
    addr_t value;
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
#define RELCONST (1 << (RELSHIFT - 1))
#define RELSEGSHIFT ((RELSHIFT - 3))
#define RELTEXT (SEGTEXT << RELSEGSHIFT)
#define RELDATA (SEGDATA << RELSEGSHIFT)
#define RELBSS (SEGBSS << RELSEGSHIFT)
#define RELSEG (RELTEXT | RELDATA | RELBSS)
#define RELTYPE (RELCONST | RELSEG)

/* Object file */
#define MAGIC_OBJ ((uint16_t)0x4d47)
#define MAGIC_RELOC ((uint16_t)0x4d48)
#define MAGIC_EXE ((uint16_t)0x4d49)

#define IS_MAGIC_VALID(x) (x == MAGIC_OBJ || x == MAGIC_RELOC || x == MAGIC_EXE)
#define IS_RELOCATABLE(x) (x == MAGIC_OBJ || x == MAGIC_RELOC)

struct header
{
    uint16_t magic;
    uint16_t textsize;
    uint16_t datasize;
    uint16_t bsssize;
    uint8_t consize;
    uint16_t symsize;
    uint16_t trelsize;
    uint16_t drelsize;
    uint16_t load;
} PACKED;

/* Offsets */

#define SYMOFFSET(hdr)                  \
    ((hdr.textsize + hdr.datasize + hdr.trelsize + hdr.drelsize) + sizeof(hdr))
    
#define CODEOFFSET(hdr) (sizeof(hdr))
#define RELCODEOFFSET(hdr) (sizeof(hdr) + hdr.textsize + hdr.datasize)


#endif
