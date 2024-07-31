#include "cpu.h"
#include "fs.h"
// 
// .set LOADADDR, 0x800 ; address to load kernel into
// .set ENTRYADDR, 0x83C
// 
// .set HDRPAGE, 0x01 ; temporary storage for header block
// .set SIZE, (HDRPAGE shl 8) + FNODE_size
// .set BLK, (HDRPAGE shl 8) + FNODE_blocks
// 
// swboot:
//     scf ; 4 byte nops
//     scf
//     lda 0
//     stl SETHIBANK
//     mov16i ROOTBLK, set_block.arg_blk
//     jsr set_block
//     movi HDRPAGE, read_blk.arg_dest
//     jsr read_blk
// .loadhi:
//     ldl BLK
//     stl set_block.arg_blk
// .loadlo:
//     ldl BLK+1
//     stl set_block.arg_blk+1
//     jsr set_block
// .dest:
//     lda hi(LOADADDR)
//     stl read_blk.arg_dest
//     jsr read_blk
//     deci SIZE, 1 ; decrement high (block) size only
//     bnl .done ; < 0
//     inc16i .loadhi, 2
//     inc16i .loadlo, 2
//     inci .dest+1, 1
//     jmpz .loadhi
// .done:
//     jmpz ENTRYADDR
// 
// include 'sys/blk.asm'
// 
// ; proc pagecpy
// ;     ldl .src
// ;     orii (I_LDL shl 4)
// ;     stl .read
// ;     ldl .dst
// ;     orii (I_STL shl 4)
// ;     stl .write
// ; .loop:
// ; .read:
// ;     ldl $000
// ; .write:
// ;     stl $000
// ;     inci .read+1,1
// ;     stl .write+1
// ;     bnz .loop
// ;     rsr
// ; .src: rb 1
// ; .dst rb 1
// ; endp
