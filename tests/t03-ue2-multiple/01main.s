.text
.set Z, 1

loop:
    ldl     hellorld
    cmp     zero
done:
    bz      done         ; keep spinning in place
.globl echo
    jsr     echo
    lda     1
    adc     loop
    stl     loop
    
    scf     Z
    bz      loop

.data
hellorld:
    .byte      "\nHellorld"
zero:
    .byte      0
