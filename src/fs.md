##Block 0

Bootloader

## Block 1

Example:
```
dirent {
    ino
    name[14]
} = size 16
BLOCKSIZE = 512;
DIRPERBLOCK = BLOCKSIZE / DIRENT = 32
FSSIZE = 31250 /*16 MB*/
ILISTSIZE = FSSIZE / DIRPERBLOCK = 976 /* 500Kb of i-list */
```

```
superblock
    s_fsize: size of volume in blocks
    s_isize: size in blocks of i-list = ILISTSIZE + 2
        +2 so s_isize points to the last i-list block, where 0 is bootloader and 1 is superblock
	s_tfree;   	/* total free blocks*/
	s_tinode;  	/* total free inodes */
```

Formatting:
```
    calculate fsize, isize
    fill blocks [2:isize] with 0
    for each block add DIRPERBLOCK to s_tinode

    write superblock to block 1
```

::: mermaid
graph TD;
   Superblock-->ilist["ilist(dirent)"];
:::
