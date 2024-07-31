; Create new process
proc spawn
    prologue
    ; mov16 .arg_file, open.arg_file
    ; jsr open
    
    epilogue
.arg_file: rb 2
endp

; Replace current process
proc exec
    prologue
    ; mov16 .arg_file, open.arg_file
    ; jsr open
    
    epilogue
.arg_file: rb 2
endp

proc wait
    prologue
    epilogue
endp

proc sleep
    prologue
    epilogue
endp

proc exit
    prologue
    epilogue
endp
