; lda 1
; lda (foo)
; ;lda ; invalid!
; lda 1+2
; lda 255+1
; lda 254+1
; lda foo
; lda foo+2
; lda foo-2
; lda 9 / 3
; lda 3 * foo
lda 3 * bar

; foo:
;     .byte 1,2,3

.data
bar:
    .byte 3
