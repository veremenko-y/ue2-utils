.set TX, 0xffff-3
.set Z, 2
.set C, 1

CR = 13
BUFLEN = 40
MAX_VAR_CHAR = 'C'

init:
    mov16i dstack, dptr
    lda     0
    stl     cptr
    stl     linecnt
    stl     linecnt+1
    mov16  #heap, heapend
    jmpz    parse

;---
proc exec_free
    prologue
    mov16   heapend, areg
    neg   areg
    stl areg
    neg   areg+1
    stl areg+1

    scf
    lda lo($ff0)
    adc     areg+1
    stl     areg+1
    lda hi($ff0)
    adc     areg
    stl     areg
    mov16  #free_str, xreg
    jsr    printstr
    lda ' '
    jsr echo
    jsr    printnum
    epilogue
endp

;---
proc dpush
    ldl dptr+1
    stl .hi+1
    ldl dptr
    orii (I_STL shl 4)
    stl .hi

    scf
    ldl .hi+1
    adci 1
    stl .lo+1
    ldl .hi
    adci 0
    stl .lo
    ldl areg
.hi:
    stl $000
    ldl breg
.lo:
    stl $000
    rsr
endp

;---
proc dpop
    ldl dptr+1
    stl .hi+1
    ldl dptr
    orii (I_LDL shl 4)
    stl .hi

    scf
    ldl .hi+1
    adci 1
    stl .lo+1
    ldl .hi
    adci 0
    stl .lo
.hi:
    ldl $000
    stl areg
.lo:
    ldl $000
    stl breg
    rsr
endp

proc readx
    ldl xreg+1
    stl .hi+1
    ldl xreg
    orii (I_LDL shl 4)
    stl .hi
.hi:
    ldl $000
    rsr
endp

proc writex
    stl .ldl+1
    ldl xreg+1
    stl .stl+1
    ldl xreg
    orii (I_STL shl 4)
    stl .stl
.ldl:
    lda $00
.stl:
    stl $000
    rsr
endp

proc incx
    inc16i xreg,1
    rsr
endp

proc decx
    dec16i xreg,1
    rsr
endp

; ;---
; ; Load LRP from xreg:yreg
; ;---
; proc lrp_sm
;     ldl     xreg+1
;     stl     lrp_sm_ptr+1

;     ldl     xreg
;     orii    (I_LRP<<4)
;     stl     lrp_sm_ptr
; lrp_sm_ptr:
;     lrp     $000
; endp

; ;---
; ; Load LRP from xreg:yreg, increment xreg pair
; ;---
; proc lrp_sm_incr
;     jsr    lrp_sm
;     ldl     xreg+1
;     stl     lrp_sm_incr_ptr+1
;     scf
;     adci    1
;     stl     xreg+1

;     ldl     xreg
;     orii    (I_LRP<<4)
;     stl     lrp_sm_incr_ptr
;     ldl     xreg
;     adci    0
;     stl     xreg
; lrp_sm_incr_ptr:
;     lrp     $000
; endp


;---
; Print 16 bit integer.
; Input: areg:breg
;---
proc printnum
    prologue
    lda 0
    stl tok_cnt
    mov16i  10, div_in2
    mov16i  tok_last, xreg
    mov16   areg, div_in1
.printnum_loop:
    jsr    div16
    ldl     div_rem+1                    ; rem to char
    scf
    adci '0'
    jsr writex ; store char
    jsr decx
    inci    tok_cnt, 1
    cmpeq16i div_in1, 0                  ; if result is 0, we're done
    bz      .printnum_done
    jmpz    .printnum_loop

.printnum_done:
    ldl     tok_cnt
    jsr writex

    jsr    printstr
    epilogue
endp

;---
; Print string from xreg:yreg. Format is [length] [string bytes]
;---
proc printstr
    prologue
    jsr readx
    stl     tmpreg                       ; to count down
    stl     tmpreg+1                     ; to preserve for return
    jsr incx

.printstr_loop:
    ldl     tmpreg
    cmpi    $00
    bz      .printstr_done
    jsr readx
    jsr echo
    jsr incx
    deci    tmpreg, 1
    jmpz    .printstr_loop
.printstr_done:
    ldl     tmpreg+1
    epilogue
endp

;---
proc rdline
    prologue
    mov16i inputbuf,xreg
.rdline_loop:
    jsr rdchar
    jsr writex
    jsr echo
    cmpi    CR
    bz      .rdline_done
        cmpi    '@' ; reset line char
    bnz .skip
        lda 0
        stl inputbufcnt
        jmpz .rdline_done
.skip:
    jsr incx
    inci    inputbufcnt,1
    cmpi    BUFLEN
    bz      error
    jmpz    .rdline_loop
.rdline_done:
    epilogue
endp

;---
error:
    lda     CR
    jsr echo
    lda     'E'
    jsr echo
    jmpz    parse


;---
proc parse_var
    prologue
    lda TOK_NONE
    stl tok_type
    jsr peek_ns_char
    cmpi 'A'
    bnl     .skip
    cmpi    (MAX_VAR_CHAR+1)
    bl      .skip
        jsr get_char ; consume
        sbci 'A'
        stl tok
        movi TOK_VAR, tok_type
        lda 1
        jmpz .done
.skip:
    lda 0
.done:
    epilogue
endp

;---
; Get variable value
;---
proc get_var
    mov16 #vars, xreg
    ldl tok
    scf
    adc tok ; << 1
    add16_8 xreg
    jsr lrp_sm
    ldp
    inp
    stl tok
    ldp
    stl tok+1
    rsr
endp

;---
; Get variable value
;---
proc set_var
    mov16 tok, tok+2
    jsr tokpop
    mov16 #vars, xreg
    ldl tok
    scf
    adc tok ; << 1
    add16_8 xreg
    ldl tok+2
    jsr writex
    jsr incx
    ldl tok+3
    jsr writex
endp

;---
; Helper routine, get next byte of string to compare
;---
proc cmp_get_byte
    prologue
    mov16   cmp_id_ptr, xreg
    jsr readx
    stl breg
    jsr incx
    mov16   xreg, cmp_id_ptr
    ldl breg
    epilogue
endp

;---
; Compare current string in tok buffer with string supplied in cmp_id_ptr.
; RRA = 1 if true
;---
cmp_id_cnt:
    rb 1
cmp_id_ptr:
    rb 2
cmp_ptr_save:
    rb 2
cmp_cnt_save:
    rb 1
proc compare_id
    prologue
    mov16   parse_buf_ptr, cmp_ptr_save ; save pointer, to rollback if unsuccesful
    mov inputbufcnt, cmp_cnt_save

    jsr    cmp_get_byte
    stl     cmp_id_cnt

.loop:
    jsr    cmp_get_byte
    stl     areg
    jsr    get_char
    ore     areg
    bnz     .false
    deci    cmp_id_cnt, 1
    cmpi    0
    bnz     .loop              ; if count != 0, continue comparing

.true:
    lda     1
    jmpz    .done
.false:
    mov16   cmp_ptr_save, parse_buf_ptr ; restore buffer pointer
    mov cmp_cnt_save, inputbufcnt
    lda     0                            ; false
.done:
    epilogue
endp


;---
proc tokpush
    mov16 tok, areg
    jsr dpush
    mov tok_type, areg
    jsr dpush
endp

;---
proc tokpop
    jsr dpop
    mov areg, tok_type
    jsr dpop
    mov16 areg, tok
endp


macro add16_8 addr
    scf
    adc addr+1
    stl addr+1
    lda 0
    adc addr
    stl addr
end macro

;---
proc term_number
    prologue
    jsr peek_char
    orei    $30                          ; if '0'-'9'
    cmpi    $0A
    bl      .false

    stl     tok+1                          ; reset t, write first digit to it
    jsr get_char ; consume character
    lda     0
    stl     tok
.loop:                             ; loop over to read numbers
    ; parse number
    ldl     inputbufcnt
    cmpi    0
    bz      .finalize
    jsr    peek_char
    ldl     char
    orei    $30
    cmpi    $0A
    bl      .finalize
    stl     breg
    mov16   tok, mul_in1
    mov16  #10, mul_in2
    jsr    mul16
    mov16   mul_out+2, tok                 ; copy low 16 bit to result
    ldl     breg
    add16_8 tok                            ; add RRA to T
    jsr get_char
    jmpz    .loop
.finalize:
    lda 1
    jmpz .done
.false:
    lda 0
.done:
    epilogue
endp

TOK_NONE = 0
TOK_INT = 1
TOK_STRING = 2
TOK_VAR = 3

;---
proc term_string
    prologue
    lda 0
    stl tok_cnt
    jsr peek_char
    cmpi '"'
    bnz term_string_false
    jsr get_char ; consume
    ; saving ptr to start of the string
    ; strbuf bytes are just using temporarily
    mov16 parse_buf_ptr, tok
term_string_loop:
    jsr peek_char
    cmpi 0
    bz term_string_false
    cmpi '"'
    bz term_string_done
    inci tok_cnt, 1
    jsr get_char
    jsr term_string_loop ; consume

term_string_done:
    jsr get_char ; consume
    mov tok_cnt, tmpreg ; cnt
    mov16 tok, areg ; src (temp value we stored before)
    mov16 #tok, xreg ; dest (location)
    jsr memcpy
    lda 1
    bz term_string_ret

term_string_false:
    lda 0
    epilogue
endp


; ;---
; factor:

;     jsr parse_var
;     cmpi 1
;     bnz .skipvar
;         jsr get_var
;         jsr factor_retnum ;saving few bytes
; .skipvar:
;     jsr term_number
;     cmpi 0
;     bz .skipint
;         ;;mov16 tok, areg
; factor_retnum:
;         movi TOK_INT, tok_type
;         jmpz factor_done
; skipint:
;     jmpz factor_done
; factor_done:
;     sret

proc _term_consume_and_get
prologue
    jsr get_char ; consume
    jsr tokpush
    scall factor
    mov16 tok, tok+2
    jsr tokpop
epilogue
endp
; ;---
; term:
;     scall factor
;     jsr peek_ns_char
;     cmpi 0
;     bz term_done
;     ldl char
;     cmpi '*'
;     bnz :+
;         jsr _term_consume_and_get
;         mov16 tok, mul_in1
;         mov16 tok+2, mul_in2
;         jsr mul16
;         mov16 mul_out+2, tok
;         jmpz term_done
;     :
;     cmpi '/'
;     bnz :+
;         jsr _term_consume_and_get
;         mov16 tok, div_in1
;         mov16 tok+2, div_in2
;         jsr div16
;         mov16 div_in1, tok
;         jmpz term_done
;     :
;     cmpi '%'
;     bnz :+
;         jsr _term_consume_and_get

;         mov16 tok, div_in1
;         mov16 tok+2, div_in2
;         jsr div16
;         mov16 div_rem, tok
;         jmpz term_done
;     :
; term_done:
;     sret

;---
proc _expr_consume_and_get
    jsr get_char
    jsr tokpush
    ; scall term
    mov16 tok, tok+2
    jsr tokpop
endp
;---
; expr:
;     ; scall   term
;     jsr peek_ns_char
;     cmpi 0
;     bz expr_done
;     ldl char
;     cmpi '+'
;     bnz :+
;         jsr _expr_consume_and_get
;         scf
;         ldl tok+3
;         adc tok+1
;         stl tok+1
;         ldl tok+2
;         adc tok
;         stl tok
;         jmpz expr_done
;     :
;     cmpi '-'
;     bnz :+
;         jsr _expr_consume_and_get

;         ; negate right
;         ldl tok+2
;         nand tok+2
;         stl tok+2
;         ldl tok+3
;         nand tok+3
;         stl tok+3

;         scf C
;         ldl tok+1
;         adc tok+3
;         stl tok+3
;         ldl tok
;         adc tok+2
;         stl tok+2
;         jmpz expr_done
;     :
; expr_done:
;     sret

macro istring str
    db lengthof str
    db str
end macro
macro iword addr
    db hi(addr), lo(addr)
end macro
macro ijumpt str, addr
    iword str
    iword addr
end macro
; before jump table 2571 bytes
; after jump table 2572 bytes
stmt_table:
    ijumpt free_str, stmt_free
    ijumpt print_str, stmt_print
    ijumpt let_str, stmt_let
    ijumpt if_str, stmt_if
    ijumpt clear_str, init
    ijumpt goto_str, stmt_goto
    ijumpt list_str, stmt_list
    ijumpt run_str, stmt_run
    iword 0

free_str:
    istring "FREE"
print_str:
    istring "PRINT"
let_str:
    istring "LET"
if_str:
    istring "IF"
clear_str:
    istring "CLEAR"
goto_str:
    istring "GOTO"
list_str:
    istring "LIST"
run_str:
    istring "RUN"

;---
stmt_free:
    jsr exec_free
    jsr statement.body

;---
stmt_print:
print_loop:
    ; scall expr
    ldl tok_type
    cmpi TOK_NONE
    bnz .notstmt
        lda CR
        jsr echo
        jsr statement
.notstmt:
    cmpi TOK_INT
    bnz .notint
        mov16 tok, areg
        jsr printnum
        jsr print_loop
.notint:
    cmpi TOK_STRING
    bnz .notstr
        mov16 #tok_cnt, xreg
        jsr printstr
        jsr print_loop
.notstr:
    jsr error

;---
stmt_goto:
    prologue
    ;scall expr
    ldl tok_type
    cmpi TOK_INT
    bnz error ; must be INT token
    mov16 tok, linenum
    lda 2 ; goto mode
    stl it_mode
.done:
    epilogue

; ---
stmt_list:
    lda 0
    bz run_iterate

;---
stmt_let:
    prologue
    jsr parse_var
    cmpi 1
    bnz error

    jsr peek_ns_char
    cmpi '='
    bnz error
    jsr get_char ; consume
    jsr tokpush
    ;scall expr
    jsr set_var
    jsr statement
    epilogue
;---
stmt_if:
    lda 0
    stl cmp2
    ;scall expr
    jsr peek_ns_char
    cmpi '<'
    bz stmt_if_less
    cmpi '>'
    bz stmt_if_more
    cmpi '='
    bz stmp_if_eq
stmt_if_less:
    stl cmp1
    jsr consume_and_peek_ns
    cmpi '='
    bnz .noteq
    stl cmp2
    jsr get_char ; consume
.noteq:
    jmpz stmt_if_compare
stmt_if_more:
    stl cmp1
    jsr consume_and_peek_ns
    cmpi '='
    bnz .noteq
    stl cmp2
    jsr get_char ; consume
.noteq:
    jmpz stmt_if_compare
stmp_if_eq:
    stl cmp1
    jsr consume_and_peek_ns
stmt_if_compare:
    jsr tokpush
;    scall expr ; get second half of comparation


; local vars
cmp1:=tok+4
cmp2:=tok+5
;---
stmt_run:
    lda 1
run_iterate:
    stl it_mode
    jsr iterate_line
    jsr parse

;---
statement_ptr:
    rb 2
statement_jump_ptr:
    rb 2
proc statement
    prologue
.body:
    jsr peek_ns_char
    cmpi 0
    bz .done
    mov16 #stmt_table, statement_ptr
.loop:
    mov16 statement_ptr, xreg
    jsr readx
    stl cmp_id_ptr
    jsr incx
    jsr readx
    stl cmp_id_ptr+1
    jsr incx
    cmpeq16i cmp_id_ptr, 0
    bz error
    jsr readx
    stl statement_jump_ptr
    jsr incx
    jsr readx
    stl statement_jump_ptr+1
    jsr incx
    jsr compare_id
    cmpi 1
    bz statement_jump_ptr
    inc16i statement_ptr, 4
    jmpz .loop
.done:
    epilogue
endp

;---
proc get_char
    prologue
    mov16   parse_buf_ptr, xreg
    jsr readx
    stl char
    jsr incx
    mov16   xreg, parse_buf_ptr
    deci    inputbufcnt, 1
    ldl char
    epilogue
endp

;---
; Consume current character from the buffer
; then peek next non space character
;---
proc consume_and_peek_ns
prologue
    jsr get_char
    jsr peek_ns_char
epilogue
endp

;---
; Peek next character
;---
proc peek_char
    prologue
    ldl inputbufcnt
    cmpi 0
    bz .done ; return 0 if nothing found
    mov16   parse_buf_ptr, xreg
    jsr readx
    stl char
.done:
    epilogue
endp

;---
; Peek next character, skip spaces
;---
proc peek_ns_char
prologue
.loop:
    jsr peek_char
    cmpi ' '
    bnz .done
    jsr get_char
    jmpz .loop
.done:
epilogue
endp


;---
it_ptr:
    rb 2
it_len:
    rb 1
it_mode:
    rb 1
proc iterate_line
    prologue
    mov16 #0, linenum
.next:
    jsr find_line
    stl t ; save result
    ldl it_mode
    cmpi 2
    bnz .notgoto; if in goto mode
        lda 1
        ore t ; if exact line not found, error
        bnz error
        lda 1
        stl it_mode ; reset mode ot run
        jmpz .donegoto
.notgoto:
    ldl t ; restore find result
    cmpi 2
    bnz .done
.donegoto:

    ldl RXREADY ; break execution
    cmpi 0
    bnz .done

    mov16 xreg, it_ptr ; save
    inc16i it_ptr, 2 ; add 2 skip number and length

    jsr    readx                              ; load line number
    stl     linenum
    stl     areg
    jsr incx
    jsr readx
    stl     linenum+1
    stl     areg+1
    jsr incx
    inc16i linenum, 1
    jsr readx
    stl     it_len
    jsr incx

    ldl it_mode
    cmpi 0
    bz .modelist
    jmpz .moderun
.modelist:
    lda     CR
    jsr echo
    jsr    printnum
    lda ' '
    jsr echo
    mov16 it_ptr, xreg ; save
    jsr printstr
    bz .inext

.moderun:
    inc16i it_ptr, 1 ; skip length
    mov16 it_ptr, parse_buf_ptr
    mov it_len, inputbufcnt
    jsr statement
.inext:
    scf
    ldl it_len
    adc it_ptr+1
    stl it_ptr+1
    lda 0
    adc it_ptr
    stl it_ptr

    jsr .next

.done:
    epilogue
endp

;---
; Searches for linenum, if not found, address of the first byte
; where line with N greater than linenum begins
;---
find_line_neg:
        rb 2
proc find_line
    prologue
    mov16  #heap, xreg
    mov16   linecnt, tmpreg
    ldl linenum
    stl find_line_neg
    nand find_line_neg
    stl find_line_neg
    ldl linenum+1
    stl find_line_neg+1
    nand find_line_neg+1
    stl find_line_neg+1
.loop:
    cmpeq16i tmpreg, 0
    bz      .false
    dec16i  tmpreg, 1

    jsr    readx                          ; load line number
    stl     t
    jsr incx
    jsr readx
    stl     t+1
    jsr incx
    ; todo load and compare at the same time
    scf     C
    ldl     t
    adc     find_line_neg
    bnl     .lower
    bnz     .move
    scf     C
    ldl     t+1
    adc     find_line_neg+1
    bnl     .lower
    bz      .true
    jmpz    .move               ; line# is larger
.lower:
    jsr readx                                  ; load length of line
    stl     t
    jsr incx

    scf
    adci    3                            ; add 3 to it (header size)
    scf
    adc     xreg+1                       ; move xreg to next line
    stl     xreg+1
    ldl     xreg
    adci    0
    stl     xreg

    jmpz    .loop

.false:
    lda     0
    bz      .done
.move:
    lda     2
    bz      .done
.true:
    lda     1
.done:
    epilogue
endp
;---
; Stores currently parsed line `tok` using line `linenum`
;---
; linesize:
;     rb 2
store_line:
    jsr    find_line
    cmpi    0
    bz      append_line
    ; ! right now we can't move heap memory
    jmpz    error
append_line:
    scf
    ldl     heapend+1
    adc     inputbufcnt
    stl     heapend+1
    ldl     heapend
    adci    0
    stl     heapend
    inc16i  linecnt, 1
    ldl     linenum
    jsr     writex
    jsr     incx
    ldl     linenum+1
    jsr     writex
    jsr     incx
    ldl     inputbufcnt
    jsr     writex
    jsr     incx
    inc16i  xreg, 3

    mov16   parse_buf_ptr, areg
    mov     inputbufcnt, tmpreg
    jsr memcpy
    jsr parse

;---
; Copy tmpreg (0,255) bytes from areg:breg into xreg:yreg
;---
proc memcpy
    ldl     breg
    stl     memcpy_load+1
    ldl     areg
    orii    (I_LDL shl 4)
    stl     memcpy_load

    ldl     yreg
    stl     memcpy_store+1
    ldl     xreg
    orii    (I_STL shl 4)
    stl     memcpy_store
memcpy_loop:
    lda     0
    ore     tmpreg
    bz      memcpy_done
    deci    tmpreg, 1
memcpy_load:
    ldl     $000
memcpy_store:
    stl     $000
    inc16i  memcpy_load, 1
    inc16i  memcpy_store, 1
    jmpz    memcpy_loop
memcpy_done:
endp

;---
parse_buf_ptr:
    rb 2
linenum:
    rb 2

parse:
    lda     0
    stl     linenum
    stl     linenum+1
    stl     inputbufcnt
    mov16  #inputbuf, parse_buf_ptr      ; reset pointers

    lda     CR
    jsr echo
    lda '>'
    jsr echo
    jsr    rdline
    ldl     inputbufcnt
    cmpi    0
    bz      parse

parse_loop:
    ldl     inputbufcnt                  ; check input buffer for 0
    cmpi    0
    bz      parse
    jsr     term_number
    cmpi 0
    bz not_number
    mov16 tok, linenum
not_number:
    cmpeq16i linenum, 0
    bz      exec_run                     ; if no line number, execute
    jmpz    store_line                   ; else, store line
exec_run:
    jsr statement
    jsr parse
exec_none:
    jmpz    parse


; =====================================
; Read character from input
; =====================================
proc    rdchar
.loop:
    lda     0                 ; loop until char available
    ore     RXREADY
    bz      .loop
    ldl     RX                ; load char
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



mul_in1:
div_in1:
    rb 2
mul_in2:
div_in2:
    rb 2
mul_out:
div_rem:
    rb 4
mul_cnt:
div_cnt:
    rb 1
mul_tmp:
div_tmp:
    rb 1
div16:
    lda     0
    stl     div_rem
    stl     div_rem+1
    movi    16, div_cnt
divL1:
    ldl     div_in1+1
    scf
    adc     div_in1+1
    stl     div_in1+1

    ldl div_in1
    adc div_in1
    stl div_in1

    ldl div_rem+1
    adc div_rem+1
    stl div_rem+1

    ldl div_rem
    adc div_rem
    stl div_rem

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

mul16:
    lda     0                            ; clear upper bits
    stl     mul_out
    stl     mul_out+1
    lda     16
    stl     mul_cnt
L1:
    ; divide multiplier by 2
    ldl mul_in1
    scf
    ror
    stl mul_in1
    ldl mul_in1+1
    ror
    stl mul_in1+1

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
    ldl mul_out
    ror
    stl mul_out

    ldl mul_out+1
    ror
    stl mul_out+1

    ldl mul_out+2
    ror
    stl mul_out+2

    ldl mul_out+3
    ror
    stl mul_out+3

    deci    mul_cnt, 1
    bnz     L1
    rsr


t:
    rb 2
inputbuf:
    rb BUFLEN
inputbufcnt:
    rb 1

tok_cnt:
    rb 1
tok:
    rb BUFLEN
tok_last = $-1
tok_type:
    rb 1
cptr:
    rb 1
cstack:
    rb 16
dptr:
    rb 2
dstack:
    rb 16
char:
    rb 1
areg:
    rb 1
breg:
    rb 1
xreg:
    rb 1
yreg:
    rb 1
treg:
    rb 2
tmpreg:
    rb 2
linecnt:
    rb 2
vars:
    rb (MAX_VAR_CHAR-'A'+1)*2
heapend:
    rb 2
heap = ConstsEnd
