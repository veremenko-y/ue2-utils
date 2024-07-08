.set C, 1
.set Z, 2
.set TX, 0xffc
;.globl C,Z,TX

echo:
    stl tmp
    ldl     TX
    cmp     #0
    bz      echo    
    ldl     tmp
    stl     TX
    rsr
.bss
tmp:
    .res 1
;.globl echo
