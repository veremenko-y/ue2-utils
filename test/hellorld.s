// #include "test.h"
.globl TX
.globl echo
loop:
    lda #echo+1
   ldl     hellorld
   cmp     #0
.done:
    bz      .done         // keep spinning in place
    jsr echo
    lda     1
    adc     loop+1
    stl     loop+1
    
    scf     Z
    bz     loop
global:
.res 2
.done:
    bz .done:
.data
hellorld:
    .byte      "Hellorld"
