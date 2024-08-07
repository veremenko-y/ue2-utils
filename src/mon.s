include '../../sdk/ue2.inc'
include '../../sdk/ue2_io.inc'
include '../../sdk/macros.inc'

ASSEMBLER = 0

CR      = 13
BUFLEN  = $40
DOT     = '.'
COLON   = ':'
PROMPT  = '>'
SPACE   = ' '
RCHAR   = 'R'
ECHAR   = 'E'

init:
    lda     CR
    jsr     echo
    lda     PROMPT
    jsr     echo
    lda     $0            ; reset mode
    stl     mode
    stl     inputcnt

    lda     (I_STL shl 4) ; init input buffer
    orii    hi(inputbuf)
    stl     rdline_write
    lda     lo(inputbuf)
    stl     rdline_write+1

.loop:
    jsr     rdchar
    stl     inputchar
    jsr     echo

    inci    inputcnt, 1   ; increment counter
    orei    BUFLEN        ; if over buffer, reset
    bz      init

    ldl     inputchar
rdline_write:
    stl     $000          ; store in input buffer
    inc16i  rdline_write, 1
    ldl     inputchar
    orei    CR            ; if CR
    bz      nl
    jmpz    init.loop

nl:
    mov16   #inputbuf,inputbuf_ptr

exec_withclear:
    lda     0
    stl     parsedcnt
    stl     parsed        ; clear parsed
    stl     parsed+1
exec:
    deci    inputcnt, 1   ; if out of buffer, start over
    bl      .cont         ; if >= 0, continue
    jmpz    init
.cont:
    jsr     rdbuf
nextbuf:
    ldl     inputbuf_char

    orei    $30           ; if 0-9
    cmpi    $0A
    bl      notdigit
    stl     inputchar     ; temporary save

    jmpz    shiftin_parsed
notdigit:
    ldl     inputbuf_char

    scf     C
    sbci    $37           ; char - F+10
    cmpi    $10           ; if <=F
    bl      nothex        ; else execute
    cmpi    $09           ; if > 9
    bl      .ishex
    jmpz    nothex
.ishex:
    stl     inputchar
    jmpz    shiftin_parsed
nothex:
    ldl     inputbuf_char
    stl     inputchar


    ldl     inputchar
    orei    CR
    bz      run_mode

    ldl     inputchar
    cmpi    DOT
    bz      dot_mode
    cmpi    COLON
    bz      colon_mode
    cmpi    RCHAR
    bz      r_mode

    jmpz    run_mode
dot_mode:
    lda     1
    stl     mode
    jmpz    readsingle
colon_mode:
    lda     2
    stl     mode
    jmpz    readsingle
r_mode:
    ldl     addrfrom
    stl     r_mode_sm
    ldl     addrfrom+1
    stl     r_mode_sm+1
    scf     Z
r_mode_sm:
    bz      $000
run_mode:
    ldl     mode
    cmpi    $0
    bz      readsingle
    cmpi    $1
    bz      readmulti
    cmpi    $2
    bz      writemulti
    jmpz    init
readsingle:
    ldl     parsedcnt
    cmpi    0
    bz      .skip            ; skip if nothing to read
    ldl     parsed
    stl     addrfrom
    stl     addrto
    ldl     parsed+1
    stl     addrfrom+1
    stl     addrto+1
    jsr     readmemory
.skip:
    jmpz    exec_withclear
readmulti:
    lda     0
    stl     mode
    ldl     parsedcnt
    cmpi    0
    bz      .skip
    ldl     parsed
    stl     addrto
    ldl     parsed+1
    stl     addrto+1
    jsr     readmemory
.skip:
    jmpz    exec_withclear
writemulti:
    ldl     parsedcnt     ; if no data, skip
    cmpi    0
    bz      exec_withclear

    ldl     addrfrom      ; self modifying code
    orii    (I_STL shl 4) ; set write address and instruciton
    stl     write_sm
    ldl     addrfrom+1
    stl     write_sm+1

    ldl     parsed+1      ; load lo byte of the value
write_sm:                 ; self modifying write
    stl     $000

    ldl     addrfrom+1    ; add 1 to 16 bit address
    scf
    adci    1
    stl     addrfrom+1
    ldl     addrfrom
    adci    0
    ldl     addrfrom

    jmpz    exec_withclear

; =====================================
; Read character from input
; =====================================
proc    rdchar
.loop:
    lda     0             ; loop until char available
    ore     RXREADY
    bz      .loop
    ldl     RX            ; load char
    rsr
endp

; =====================================
; Echo char
; =====================================
proc    echo
    stl     sm+1
.loop:
    lda     0
    ore     TX
    bz      .loop
sm:
    lda     $00
    stl     TX
    rsr
endp


; =====================================
; Print memory between addfrom and addrto
; =====================================
proc    readmemory
    prologue
.start:
    jsr     readmemory_row

.loop:
    jsr     readmemory_next
    neg     addrto
    stl     temp
    ldl     addrfrom
    scf     C
    cmp     temp
    bz      .checklo
    bl      .end
.checklo:
    neg     addrto+1
    stl     temp
    ldl     addrfrom+1
    scf     C
    cmp     temp
    bz      .end          ; allo last one to be printed
    bl      .end 
.skip:
    ldl     addrfrom+1                          
    scf
    adci    1
    stl     addrfrom+1
    ldl     addrfrom
    adci    0
    ldl     addrfrom

    ldl     addrfrom+1
    andi    temp, $0f
    cmpi    0
    bz      .start
    jmpz    .loop
.end:
    epilogue
endp

proc    readmemory_row
    prologue
    lda     CR
    jsr     echo
    ldl     addrfrom
    jsr     printhex
    ldl     addrfrom+1
    jsr     printhex
    lda     COLON
    jsr     echo
    epilogue
endp

proc    readmemory_next
    prologue
    lda     SPACE
    jsr     echo
    ldl     addrfrom
    orii    (I_LDL shl 4)
    stl     .read_ind
    ldl     addrfrom+1
    stl     .read_ind+1
.read_ind:
    ldl     $000
    jsr     printhex
    epilogue
endp


proc    rdbuf
    ldl     inputbuf_ptr
    orii    (I_LDL shl 4)
    stl     read
    ldl     inputbuf_ptr+1
    stl     read+1
read:
    ldl     $000
    stl     inputbuf_char 
    inc16i  inputbuf_ptr, 1
    rsr
endp

; =====================================
; Shift 4 bits into `parsed`
; =====================================
shiftin_parsed:
    lda     0
    stl     temp
    lda     3             ; optimization, counting 3..0 with BL to loop on decrement
    stl     cnt
shiftin_loop:
    ldl     parsed+1
    scf
    adc     parsed+1
    stl     parsed+1
    bl      .carry
    jmpz    .nocarry
.carry:                   ; carry
    lda     1
    stl     temp
.nocarry:                 ; no carry
    ldl     parsed
    scf
    adc     parsed
    scf
    adc     temp
    stl     parsed
    lda     0
    stl     temp

    deci    cnt, 1
    bl      shiftin_loop

    ldl     parsed+1      ; add new nibble
    ori     inputchar
    stl     parsed+1

    inci    parsedcnt, 1

    ldl     parsed        ; remove hi nibble for 12 bit address
    andi    parsed, $0f            
    stl     parsed

    jmpz    exec

; ==========================================
; Print HEX in A
; ===========================================
proc    printhexnibble
    prologue
    scf
    adci    $30           ; nibble + 0
    cmpi    $3A           ; if a <= 9 - send
    bl      .hex
    jmpz    .dec           ; do second nibble
.hex:
    adci    $06           ; add 7 (6 + Carry from branch) to bump to A-F
.dec:
    jsr     echo
    epilogue
endp

; ==========================================
; Print 16 bit HEX
; ===========================================
proc    printhex
    prologue
    stl     temp          ; store original
    stl     inputchar     ; twice
    lda     0
    stl     nibble        ; for hi nibble

    lda     3             ; counting 3..0
    stl     cnt

.loop:
    ldl     temp
    scf
    adc     temp          ; shl 1
    stl     temp

    ldl     nibble
    adci    0
    scf
    adc     nibble
    stl     nibble

    deci    cnt, 1
    bl      .loop

    ldl     nibble
    jsr     printhexnibble

    ldl     inputchar     ; restore original
    andi    nibble, $0f
    jsr     printhexnibble
    epilogue
endp

temp:
    rb      1
inputbuf:
    rb      BUFLEN
inputbuf_char:
    rb      1
inputbuf_ptr:
    rb      2
parsed:
    rb      2
parsedcnt:
    rb      1
addrfrom:
    rb      2
addrto:
    rb      2
cnt:
    rb      1
nibble:
    rb      1
mode:
    rb      1
inputchar:
    rb      1
inputcnt:
    rb      1
