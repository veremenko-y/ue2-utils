sysent:
    bz sys_spawn
    bz sys_wait
    bz sys_sleep
    bz sys_exit
    bz sys_open
    bz sys_read
    bz sys_write
    bz sys_seek
    bz sys_close

sys_spawn:
    jsr spawn
    jsr sysret
    
sys_wait:
    jsr wait
    jsr sysret

sys_sleep:
    jsr sleep
    jsr sysret

sys_exit:
    jsr exit
    jsr sysret

sys_open:
    jsr open
    jsr sysret

sys_read:
    jsr read
    jsr sysret

sys_write:
    jsr write
    jsr sysret

sys_seek:
    jsr seek
    jsr sysret

sys_close:
    jsr close
    jsr sysret
