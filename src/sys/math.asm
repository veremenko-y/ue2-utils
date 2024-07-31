mul_in1:
div_in1:
    rb      2
mul_in2:
div_in2:
    rb      2
mul_out:
div_rem:
    rb      4
mul_cnt:
div_cnt:
    rb      1
mul_tmp:
div_tmp:
    rb      1

proc    div16
    lda     0                            
    stl     div_rem
    stl     div_rem+1
    movi    16, div_cnt
divL1:
    ldl     div_in1+1     
    scf
    adc     div_in1+1               
    stl     div_in1+1

    ldl     div_in1
    adc     div_in1
    stl     div_in1

    ldl     div_rem+1
    adc     div_rem+1
    stl     div_rem+1

    ldl     div_rem
    adc     div_rem
    stl     div_rem

    sub     div_rem+1, div_in2+1
    stl     div_tmp
    sbc     div_rem, div_in2
    bnl     divL2                        
    stl     div_rem                      
    mov     div_tmp, div_rem+1
    inci    div_in1+1, 1
divL2:
    deci    div_cnt, 1
    bnz     divL1
    rsr
endp

proc    mul16
    lda     0 ; clear upper bits
    stl     mul_out
    stl     mul_out+1
    lda     16
    stl     mul_cnt
L1:
 ; divide multiplier by 2
    ldl     mul_in1
    scf
    ror
    stl     mul_in1
    ldl     mul_in1+1
    ror
    stl     mul_in1+1

    bnl     L2
    scf     
    ldl     mul_out+1
    adc     mul_in2+1
    stl     mul_out+1

    ldl     mul_out
    adc     mul_in2
    stl     mul_out
L2:
 ; shift out result
    ldl     mul_out
    ror
    stl     mul_out

    ldl     mul_out+1
    ror
    stl     mul_out+1

    ldl     mul_out+2
    ror
    stl     mul_out+2

    ldl     mul_out+3
    ror
    stl     mul_out+3

    deci    mul_cnt, 1
    bnz     L1
    rsr
endp
