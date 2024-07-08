.globl TX
.globl echo

loop:
   ldl     hellorld
   cmp     #0
done:
    bz      done         ; keep spinning in place
    jsr echo
    lda     1
    ; expressions  not supported yet
    adc     loop+1
    stl     (loop+1)
    
    scf     Z
    bz     loop
    ; ldl #done
.data
hellorld:
    .byte      "Hellorld"

.bss
    .res 19
    label:
    .res 20

