.set C, 1
.set Z, 2
.set TX, 0xffc
;.globl C,Z,TX

echo:
    stl     TX
    rsr

;.globl echo
