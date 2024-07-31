include '../../sdk/ue2.inc'
include '../../sdk/ue2_io.inc'
include '../../sdk/macros.inc'
include "def.inc"

org $800

; -------------------------------------
; Trap entry
; -------------------------------------
crt0:
    scf     Z
    bz      trap
; -------------------------------------
; Syscall entry
; -------------------------------------
sysvec: ; to be called as a syscall
    lda     0
    stl     SETHIBANK ; switching to kernel bank
    jmpz    sysenter
sysret: ; return here after syscal is compelte
    ldl     u_user+USER_hibank
    stl     SETHIBANK ; restore user bank
    scf     Z
    bz      u_user+USER_ret ; jump back to the caller
u_user:
    rb USER_sizeof
    align $1f
    assert $ = $820
.end:
; ### end of the code block ###
; to be copied into the process

sysenter:
    strh    u_user+USER_ret ; save return address
    strl    u_user+USER_ret+1
    ldl     u_user+USER_pid ; save current pid
    stl     curpid

    ldl     u_user+USER_sys_n
    stl     .sm_syscall+1
    scf     Z
.sm_syscall:
    bz      sysent

sysret:
    ldl     curpid
    scf
    adci    lo(proctbl)
    stl     .sm_procbank+1
.sm_procbank:
    ldl     proctbl            ; restore process hi bank
    stl     SETHIBANK        ; so we're able to pull return address
    jmpz    curproc+USER_resume

; -------------------------------------
; Kernel entry
; -------------------------------------
entry:
    ; init bank table
.bloop:
    lda 0
.sm_stbank:
    stl freebanks+NBANKS+1
    dec16i .sm_stbank,1
    bl .bloop
    lda 1
    stl freebanks ; mark 0 (kernel) bank taken
    ; init proc table
.loop:
    lda 0
.sm_proc:
    stl proctbl+NPROC-1
    deci .sm_proc+1,1
    bl .loop
    
    ; spawn init process
    mov16i str_init, spawn.arg_file
    jsr spawn
    
    ; run scheduler
;     lda 0
; .sm_proc:
;     stl proctbl+NPROC
;     lda 1
;     deci .sm_proc,1
;     bnl .sm_proc

str_init:
    STR 'MON'

