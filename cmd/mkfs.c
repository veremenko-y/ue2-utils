#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fs.h>

struct direct dirbuf[DIRPERBL] = {
    { 1, "." },
    { 1, ".."}
};

union
{
    struct freeblk fb;
    char pad1[BSIZE];
} fbuf;
char buf[BSIZE];

struct dinode inode[INODEPERBL];

struct filesys filsys;
int ino;
FILE *f;

void bwrite(int blockno, char *buf)
{
    int n;
    fseek(f, blockno * BSIZE, SEEK_SET);
    n = fwrite(buf, sizeof(char), BSIZE, f);
    if (n != BSIZE)
    {
        printf("write error: %d\n", blockno);
        exit(1);
    }
}

bfree(int blockno)
{
    int i;

    filsys.tfree++;
    if (filsys.lfree_n >= FREEBCACHE)
    {
        fbuf.fb.lfree_n = filsys.lfree_n;
        for (i = 0; i < FREEBCACHE; i++)
            fbuf.fb.lfree[i] = filsys.lfree[i];
        bwrite(blockno, (char *)&fbuf);
        filsys.lfree_n = 0;
    }
    filsys.lfree[filsys.lfree_n++] = blockno;
}

int main()
{
    int size = (16 * 1024 * 1024) / BSIZE;
    int i, block;

    if (size > UINT16_MAX)
    {
        printf("fs too large\n");
        exit(1);
    }

    f = fopen("fs.img", "w");
    filsys.fsize = size;
    filsys.isize = size / INODEPERBL;
    printf("filesys=%d bytes\n", sizeof(filsys));
    printf("size=%d b\n", filsys.fsize);
    printf("inode=%d b(%d nodes)\n", filsys.isize, filsys.isize * INODEPERBL);
    for (i = 0; i < BSIZE; i++)
        buf[i] = 0;

    /* Write empty blocks into i-list */
    printf("zero all blocks\n");
    for (i = 0; i < filsys.fsize; i++)
    {
        bwrite(i, buf);
        filsys.tinode += INODEPERBL;
    }

    /* Init block free list */
    block = filsys.fsize - 1;
    for (i = filsys.fsize - 1; i > filsys.isize; i--)
    {
        bfree(i);
    }

    inode[1].i_mode = F_DIR | (0777 & MODE_MASK);
	inode[1].i_nlink = 3;
	inode[1].i_size = 2*sizeof(struct direct);
	inode[1].i_addr[0] = filsys.isize;

	/* Reserve reserved inode */
	inode[0].i_nlink = 1;
	inode[0].i_mode = ~0;
    bwrite(2, (char*)&inode);

	bwrite(filsys.isize, (char*)dirbuf);

    /* Write superblock */
    bwrite(1, (char*)&filsys);
    fclose(f);
    /* dump_filesys(&filsys); */
    return 0;
}