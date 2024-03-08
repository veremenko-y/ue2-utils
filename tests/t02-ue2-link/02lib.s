padding:
    .byte 1,2,3,4

echo:
    stl     sm
loop:
    lda     0
    ore     TX
    bz      loop
sm:
    lda     0
    stl     TX
    rsr
        
echo2:
    stl     TX
    rsr
.set TX, 0xffc

.globl echo
.globl echo2
