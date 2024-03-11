#ifndef OBJ_H
#define OBJ_H

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#include <stdint.h>
#else
#define PACKED
#include "pdp11_stdint.h"
#endif

#define NCPS 32 /* length of label/symbol */
#define ADDRTYPE uint16_t
#define MAXADDR ((1 << 12) - 1) /* yaros todo: move to ue2.h */

#define A_MAGIC1 0407 /* normal */
#define A_MAGIC2 0410 /* read-only text */
#define A_MAGIC3 0411 /* separated I&D (not on VAX) */
#define A_MAGIC4 0405 /* overlay */
#define A_MAGIC5 0413 /* demand page read-only text */

struct hdr
{
	uint16_t magic;
	uint16_t tsize;
	uint16_t dsize;
	uint16_t bsize;
	uint16_t ssize;
	uint16_t entry;
	uint16_t trsize;
	uint16_t drsize;
} PACKED;

struct symtab
{
	uint8_t name[NCPS];
	uint8_t type;
	uint8_t tag;
	uint16_t index;
	uint16_t value;
} PACKED;

#define NHASH 4000 /* numbe rof hash table */
#define HASHCLOGGED (NHASH * 3) / 4

/*
 * Symbol types
 */
#define XUNDEF 0x0
#define XABS 0x2
#define XTEXT 0x4
#define XDATA 0x6
#define XBSS 0x8
#define XDATAO 0xA
#define XBSSO 0xC
#define XTEXTO 0xE
#define XABSO 0x10
#define XUNDEFO 0x12 /* I think it means external absolute */

#define XTXRN 0xA /* external symbol */
#define XXTRN 0x1
#define XTYPE 0x1E

#define XFORW 0x20 /* Was forward-referenced when undefined */

/* Text segment offset*/
#define T_OFFSET(hdr) (sizeof(hdr))
/* Data segment offset */
#define D_OFFSET(hdr) (sizeof(hdr) + hdr.tsize)
/* Text relocation offset */
#define TREL_OFFSET(hdr) (sizeof(hdr) + hdr.tsize + hdr.dsize)
/* Data relocation offset */
#define DREL_OFFSET(hdr) (sizeof(hdr) + hdr.tsize + hdr.dsize + hdr.trsize)
/* Symbol segment offset */
#define SYM_OFFSET(hdr) (sizeof(hdr) + hdr.tsize + hdr.dsize + hdr.trsize + hdr.drsize)

#endif