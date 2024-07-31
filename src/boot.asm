#include "cpu.h"

.set LOADADDR, 0x800
.set TRAMPOLINE, 0x900
.set STAGE2BANK, 1
.set AT, 0x40

.globl TX, SETBLK

count:
    lda STAGE2BANK
    stl SETHIBANK // set bank 1 at 0x800
    scf 0
    lda AT
    stl TX
    ldl SETBLK
    lda 0
    stl SETBLK
    stl SETBLK
.wait:
    lda 0 // wait for read to complete
    cmp STATBLK
    bz .wait
    
    lda 0
    stl count
.loop:
    ldl BLKBUF
.store:
    stl LOADADDR
    // inc16i .store, 1
    // deci count, 1
    bz .tostage2
    // jmpz  .loop
.tostage2:
    lda I_STL + (>SETLOBANK)
    stl TRAMPOLINE
    lda (<SETLOBANK)
    stl TRAMPOLINE+1
    lda 0
    stl TRAMPOLINE+2
    stl TRAMPOLINE+3
    lda STAGE2BANK
    bz TRAMPOLINE


