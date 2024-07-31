proc memcpy
    ldl .src
    orii (I_LDL shl 4)
    stl .read
    ldl .src+1
    stl .read+1
    
    ldl .dst
    orii (I_STL shl 4)
    stl .write
    ldl .dst+1
    stl .write+1
.loop:
.read:
    ldl $000
.write:
    stl $000
    inc16i .read+1,1
    inc16i .write+1,1
    deci .n,1
    bnz .loop
    rsr
.src: rb 2
.dst rb 2
.n: rb 1
assert (memcpy and $ff00) = ($ and $ff00)
; .inc16:
;     bz .w
;     movi .read+1, 
; .w:
;     ldl
endp





; proc print
;     prologue
;     epilogue
; .arg_str: rb 2
; endp

proc putch
.loop:
    lda     0
    ore     TX
    bz      .loop
.sm:
    lda     $00
    stl     TX
    rsr
.arg_c = .sm+1
endp

proc strcmp
    smaddr .arg_str1, I_LDL, .load1
    smaddr .arg_str2, I_ORE, .load2
.load1:
    lda $000
    scf
    adci 0
    stl .count
    bz .false
.load2:
    ore $000
    bnz .false
    inc16i .load1,1
    inc16i .load2,1
    deci .count, 1
    bl .load1
    ;jmpz .load1
.false:
    scf ; reset Z flag
    rsr
.true:
    scf Z
    rsr
.arg_str1: rb 2
.arg_str2: rb 2
.count: rb 1
endp
