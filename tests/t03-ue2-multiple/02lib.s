.text
padding:
    .byte 1,2,3,4

echo:
    stl     sm
loop2:
    lda     0
    ore     TX
    bz      loop2
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
