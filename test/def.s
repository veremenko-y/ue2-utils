.set C, 1
.set Z, 2
.set TX, 0xffc
// .globl C,Z,TX

echo:
    stl tmp
loop:
    ldl     TX
    cmp     #0
    bz      loop    
    ldl     tmp
    stl     TX
    rsr
.globl echo

.bss
tmp:
    .res 1
