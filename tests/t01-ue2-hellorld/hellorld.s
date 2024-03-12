.set C, 1
.set Z, 2
.set TX, 0xffc

loop:
    ldl     hellorld
    cmp     zero
done:
    bz      done         ; keep spinning in place
    stl     TX
    lda     1
    adc     loop+1
    stl     loop+1
    
    scf     Z
    bz      loop

.data
hellorld:
    .byte      "Hellorld"
zero:
    .byte      0
