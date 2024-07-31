
EmitConsts

kend:
WORD KENRELEND

virtual 
    curpid: rb 1
    align   $1f
    proctbl: rb NPROC
    freebanks: rb NBANKS

    align   $ff
    FSBUF: rb 256
    KENRELEND = $
end virtual

assert KENRELEND < $F00
