proc open
    prologue
    ; set string compare argument
    mov16 .arg_file, strcmp.arg_str1
    ; set fs buffer for reading headers
    movi hi(FSBUF), read_blk.arg_dest
    ; set start block
    mov16i ROOTBLK, .blk

.loopblocks:
    ; read header
    mov16 .blk, set_block.arg_blk
    jsr set_block
    jsr read_blk
    ; init node counter
    movi NNODE,.nodes
    ; init start address
    mov16i FSBUF,.addr

.loopnodes:
    mov16 .addr, strcmp.arg_str2
    jsr strcmp ; compare names
    bz .foundfile
    ; move to next node
    inc16i .addr, FNODE_sizeof
    deci .nodes,1
    bnl .loopnodes

    ; for now only checking first block
    ; inc16i .blk,1


.foundfile:
    ;stl HALT


    epilogue
.arg_file: rb 2
.blk: rb 2
.nodes: rb 1
.addr: rb 2
endp

proc read
    prologue
    rsr
    epilogue
endp

proc write
    prologue
    rsr
    epilogue
endp

proc seek
    prologue
    rsr
    epilogue
endp

proc close
    prologue
    rsr
    epilogue
endp
