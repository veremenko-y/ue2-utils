.set Z, 1
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
    .byte      "\nHellorld"
zero:
    .byte      0
