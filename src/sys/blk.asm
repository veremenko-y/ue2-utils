proc set_block
.wait:
    ldl STATBLK ; wait for read to complete
    cmpi 0
    bz .wait

    ldl SETBLK ; reset counterB
    ldl .arg_blk+1
    stl SETBLK
    ldl .arg_blk
    stl SETBLK
    
.wait2:
    ldl STATBLK ; wait for read to complete
    cmpi 0
    bz .wait2
    rsr
.arg_blk:
    rb 2
endp

;---
proc read_blk
    prologue
    lda '.'
    jsr echo
.sm_dest:
    lda $00
    orii (I_STL shl 4)
    stl .store
.loop:
    ldl BLKBUF
.store:
    stl 0
    inci .store+1, 1
    bnz .loop
    epilogue
.arg_dest = .sm_dest+1
endp


proc    echo
    stl     .sm+1
.loop:
    lda     0
    ore     TX
    bz      .loop
.sm:
    lda     $00
    stl     TX
    rsr
endp
