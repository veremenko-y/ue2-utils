.set TX, 0xffff-3    
.set Z, 2
.set C, 1

loop:
    ldl     hellorld
    cmp     #0
    bz      .done
    stl tmp
.wait:
    ldl     TX
    cmp     #0
    bz      .wait
    ldl     tmp
    stl     TX
    lda     1
    scf     0Pclose
    adc     loop+2
    stl     loop+2
    
    scf     Z
    bz      loop
.done:
    bz      .done         # keep spinning in place
.data
hellorld:
    .byte      "Hellorld"
    .byte 0
tmp:
    .byte      0


# Hand assembled version for monitor with built-in
# assembler
# ================================================
# 400
# !LRP 410
# LDP
# CMP 419
# BRZ 407
# STL FFC
# INP
# SCF 2S
# BRZ 402
