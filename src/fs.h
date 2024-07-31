// macro STR val
//     db lengthof val, val
// end macro
// 
// macro   STRUCTURE name, field, size
//     eval    'if ~ defined _', `name
//         eval    '_',`name,'=0'
//     end     if
//     eval    `name,'_',`field,'=_',`name
//     eval    '_',`name,'=_',`name,'+',`size
// end     macro
// 
// ; USER
// 
// STRUCTURE USER, resume, 4
// ; proc_resume_block:
// ;     ldl curproc+USER_lobank ; restore process lo bank
// ;     stl SETLOBANK           ; then return from syscall
// ; .end:
// STRUCTURE USER, pid, 1
// STRUCTURE USER, ret, 2
// STRUCTURE USER, size, 1
// STRUCTURE USER, lobank, 1
// STRUCTURE USER, hibank, 1
// STRUCTURE USER, sys_n, 1
// STRUCTURE USER, sys_arg0, 1
// STRUCTURE USER, cwd, 2
// STRUCTURE USER, files, 4
// STRUCTURE USER, sizeof, 0
// 
// NPROC   = 8               ; max number of processes
// NBANKS = 16
// EOFMEM  = $F00            ; end of physical memory
// 
// 
// ; FS
// 

.set ROOTBLK, 0x10
.set HDRSN, 16
.set NODESZ, 28
.set NNODE, 5
.set NODEBLKS, 8

.set FN_nlen, 1
.set FN_name, FN_nlen+7
.set FN_size, FN_name+2
.set FN_blocks, FN_size+16 // NODEBLKS*2
.set FN_next, FN_blocks+2
.set FN_sizeof, FN_next+0

