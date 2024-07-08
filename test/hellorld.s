.globl TX
.globl echo
.res 2
loop:
   ldl     hellorld
   cmp     #0
done:
    bz      done         ; keep spinning in place
    jsr echo
    lda     1
    adc     loop+1
    stl     loop+1
    
    scf     Z
    bz     loop
    ; ldl #done
.data
hellorld:
    .byte      "Hellorld"
