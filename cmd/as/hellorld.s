; set not supported yet
.set C, 1
.set Z, 2
.set TX, 0xffc

loop:
   ldl     hellorld
   cmp     #3
done:
    bz      done         ; keep spinning in place
    stl     TX
    lda     1
    ; expressions  not supported yet
    adc     loop ;+1
    stl     loop ;+1
    adc     #loop
    
    scf     Z
    jsr     reset

.data
    .res Z
hellorld:
    .byte      "Hellorld"

.bss
    .res 19
    label:
    .res 20

