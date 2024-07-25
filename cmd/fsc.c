#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <readline/readline.h>
#include <fs.h>

#define NINODES 8 /* in memory inodes */

struct filesys filsys;
struct freeblk fbuf;
uint16_t umask = 0777;
uint8_t buf[BSIZE];
struct cinode *rootdir;
struct cinode *cwd;

struct cinode inode[NINODES]; /* in memory inode buffer*/
FILE *f;

#define dprintf printf
/* #define dprintf(...) */

void bwrite(blkno_t blockno, char *buf)
{
    dprintf("bwrite: %d\n", blockno);
    fseek(f, blockno * BSIZE, SEEK_SET);
    int n = fwrite(buf, sizeof(char), BSIZE, f);
    if (n != BSIZE)
    {
        printf("write error: %d\n", blockno);
        exit(1);
    }
    fflush(f);
}

void bread(blkno_t blockno, char *buf)
{
    dprintf("bread: %d\n", blockno);
    fseek(f, blockno * BSIZE, SEEK_SET);
    int n = fread(buf, sizeof(char), BSIZE, f);
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
        fbuf.lfree_n = filsys.lfree_n;
        for (i = 0; i < FREEBCACHE; i++)
            fbuf.lfree[i] = filsys.lfree[i];
        bwrite(blockno, (char *)&fbuf);
        filsys.lfree_n = 0;
    }
    filsys.lfree[filsys.lfree_n++] = blockno;
}

blkno_t balloc()
{
    blkno_t block;
    filsys.tfree--;
    block = filsys.lfree[filsys.lfree_n--];
    if (block == 0)
    {
        printf("out of free space\n");
        exit(1);
    }
    if (filsys.lfree_n <= 0)
    {
        bread(block, &fbuf);
        memcpy(&filsys.lfree_n, &fbuf, sizeof(fbuf));
        if (filsys.lfree_n == 0 || filsys.lfree_n > FREEBCACHE)
        {
            printf("corrupted\n");
            exit(1);
        }
    }
    /*   bread(block, buf);
      memset(buf, 0, BSIZE);
      bwrite(block, buf); */
    return block;
}

/* inumber to disk address */
#define itob(x) (blkno_t)(((x + 16) >> 3))
/* inumber to disk offset */
#define itoo(x) (int)((x + 16) & 07)

ifree(ino_t ino)
{
    if (filsys.lifree_n >= FREEICACHE)
    {
        return;
    }
    filsys.lifree[filsys.lifree_n++] = ino;
}

struct cinode *iget(ino_t ino)
{
    struct cinode *finode = NULL;
    int i;
    for (i = 0; i < NINODES; i++)
    {
        if (inode[i].c_num == ino)
        {
            inode[i].c_refs++;
            return &inode[i];
        }
        if (finode == NULL && inode[i].c_refs == 0)
        {
            finode = &inode[i];
        }
    }
    finode->c_num = ino;
    finode->c_refs++;
    dprintf("iget: read ino %d at %d:%d\n", ino, itob(ino), itoo(ino));
    bread(itob(ino), buf);
    memcpy(&finode->c_node, (buf + itoo(ino) * sizeof(struct dinode)), sizeof(struct dinode));
    return finode;
}

struct cinode *ialloc()
{
    struct cinode *cp;
loop:
    if (filsys.lifree_n > 0)
    {
        cp = iget(filsys.lifree[--filsys.lifree_n]);
        cp->c_node.i_size = 0;
        cp->c_node.i_mode = 0;
        return cp;
    }
    int blk;
    int i;
    int j;
    struct dinode *p;
    int ino = 0;
    for (blk = 2; blk < filsys.isize; blk++)
    {
        bread(blk, &buf);
        p = buf;
        for (i = 0; i < INODEPERBL; i++)
        {
            if (p->i_mode == 0)
            {
                for (j = 0; j < NINODES; j++)
                {
                    if (inode[j].c_num == ino)
                        goto cont;
                }
                dprintf("ialloc: found free %d\n", ino);
                filsys.lifree[filsys.lifree_n++] = ino;
                if (filsys.lifree_n >= FREEICACHE)
                {
                    goto loop;
                }
            }
        cont:
            ino++;
            p++;
        }
    }
}

iput(struct cinode *inode)
{
    if (inode == NULL || inode->c_refs == 0)
    {
        printf("iput: invalid node\n");
        exit(1);
    }
    if (inode->c_refs == 1)
    {
        if (inode->c_node.i_nlink == 0)
        {
            ifree(inode->c_num);
        }
        blkno_t blk = itob(inode->c_num);
        bread(blk, &buf);
        struct dinode *p = buf;
        p += itoo(inode->c_num);
        *p = inode->c_node;
        bwrite(blk, &buf);
    }
    inode->c_refs--;
}

bmap(struct cinode *inode, uint16_t dest, uint8_t rw)
{
    blkno_t i = 0;
    blkno_t to = dest >> BSHIFT;
    blkno_t blk;
    blkno_t *bp;
    dprintf("bmap: dest %d iblock %d\n", dest, to);
    if (to > BSIZE / sizeof(blkno_t))
    {
        printf("offset too large\n");
        exit(1);
    }
    for (i = 0; i < INODEADDR - 1; i++)
    {
        blk = inode->c_node.i_addr[i];
        if (blk == 0)
        {
            dprintf("bmap: index %d is empty\n", i);
            if (rw == B_READ || (blk = balloc()) == NULL)
                return 0;
            dprintf("bmap: allocated %d at %d\n", blk, i);
            if (i != to)
                inode->c_node.i_size += BSIZE;
            inode->c_node.i_addr[i] = blk;
        }
        dprintf("bmap: dest %d block %d\n", i, to);
        if (i == to)
        {
            return blk;
        }
    }
    blk = inode->c_node.i_addr[INODEADDR - 1];
    if (blk == 0)
    {
        if (rw == B_READ || (blk = balloc()) == NULL)
            return 0;
        if (i != to)
            inode->c_node.i_size += BSIZE;
        inode->c_node.i_addr[INODEADDR - 1] = blk;
        memset(buf, 0, BSIZE);
        bwrite(blk, buf);
    }
    else
    {
        bread(blk, buf);
    }
    bp = buf;
    while (i < to)
    {
        blk = *bp;
        if (blk == 0)
        {
            if (rw == B_READ || (blk = balloc()) == NULL)
                return 0;
            *bp = blk;
            bwrite(inode->c_node.i_addr[INODEADDR - 1], buf);
        }
        if (i == to)
        {
            return blk;
        }
        i++;
    }
    return 0;
}

irw(struct cinode *inode, uint8_t rw, uint16_t dest, uint8_t *buffer, int size)
{
    int n = 0;
    blkno_t blk = bmap(inode, dest, B_WRITE);
    dprintf("irw: to %d\n", blk);
    if (blk == 0)
    {
        return 0;
    }
    uint16_t off = dest & ((1 << BSHIFT) - 1);
    bread(blk, buf);
    if (rw == B_WRITE)
    {
        while (off < BSIZE && n < size)
        {
            buf[off++] = *buffer++;
            n++;
        }
        dprintf("iwrite: written %d\n", n);
        bwrite(blk, buf);
        inode->c_node.i_size = dest + n;
    }
    else
    {
        n = BSIZE - off;
        if ((dest + size) > inode->c_node.i_size)
            n = inode->c_node.i_size - dest;
        if (n < 0)
            return 0;
        if (size < n)
            n = size;
        memcpy(buffer, (buf + off), n);
    }
    return n;
}

iread(struct cinode *inode, uint16_t dest, uint8_t *buffer, int size)
{
    return irw(inode, B_READ, dest, buffer, size);
}

iwrite(struct cinode *inode, uint16_t dest, uint8_t *buffer, int size)
{
    return irw(inode, B_WRITE, dest, buffer, size);
}

iinit()
{
    bread(1, &filsys);
    rootdir = iget(1);
    cwd = iget(1);
}

/* directory entry */
dlink(struct cinode *pino, char *old, char *new, struct cinode *ino)
{
    if (pino == NULL)
    {
        return NULL;
    }
    int i, j;
    blkno_t blk;
    struct direct *p;
    for (i = 0; i < pino->c_node.i_addr; i++)
    {
        blk = pino->c_node.i_addr[i];
        if (blk == 0)
        {
            blk = pino->c_node.i_addr[i] = balloc();
        }
        dprintf("dlink: checking addr %d:%d\n", blk, i);
        bread(blk, buf);
        p = buf;
        j = DIRPERBL;
        while (j-- > 0)
        {
            dprintf("dlink: check %s agains %s\n", old, p->name);
            if (strcmp(old, p->name) == 0)
            {
                if (p->ino != 0 && p->ino != ino->c_num)
                {
                    ifree(p->ino);
                }
                else
                {
                    pino->c_node.i_size += sizeof(struct direct);
                    ino->c_node.i_nlink++;
                }
                memcpy(p->name, new, DIRSIZ);
                dprintf("dlink: found %d:%d for '%s'\n", blk, DIRPERBL - j, p->name);
                p->ino = ino->c_num;
                bwrite(blk, buf);
                return blk;
            }
            p++;
        }
    }
}

mknode(struct cinode **pino, char *name)
{
    struct cinode *n = iname(pino, name);
    if (n != NULL)
    {
        iput(*pino);
        iput(n);
        return NULL;
    }
    n = ialloc();
    n->c_node.i_mode = umask | F_REG;
    n->c_node.i_nlink = 1;
    dlink(*pino, "", name, n);
    return n;
}

iref(struct cinode *ino)
{
    ino->c_refs++;
}

mkdir(char *name)
{
    if (strchr(name, '/') != NULL)
    {
        return;
    }
    struct cinode *pino;
    struct cinode *n = mknode(&pino, name);
    iput(pino);
    if (n == NULL)
    {
        return NULL;
    }
    n->c_node.i_mode = umask | F_DIR;
    dlink(n, "", ".", n);
    dlink(n, "", "..", pino);
    return n;
}

iname(struct cinode **pino, char *name)
{
    struct cinode *ino = NULL;
    char nbuf[DIRSIZ];
    char *cp;
    if (*name == '/')
    {
        *pino = rootdir;
        *name++;
    }
    else
    {
        *pino = cwd;
    }
    iref(*pino);

    do
    {
        if (ino)
        {
            iput(*pino);
            *pino = ino;
            ino = NULL;
        }
        cp = nbuf;
        while (*name && *name != '/')
        {
            *cp++ = *name++;
        }
        *cp = 0;
        struct direct dir;
        int offset = 0;
        while (iread(*pino, offset, &dir, sizeof(struct direct)))
        {
            dprintf("iname: check %s\n", dir.name);
            if (strcmp(dir.name, nbuf) == 0)
            {
                ino = iget(dir.ino);
                break;
            }
            offset += sizeof(struct direct);
        }
        if (ino == NULL)
        {
            return NULL;
        }
        if (*name == '/')
            name++;
    } while (*name);
    return ino;
}

lsmode(uint16_t mode)
{
    int i = 0;
    if (mode & F_REG)
        putchar('-');
    else if (mode & F_DIR)
        putchar('d');
    else if (mode & F_PIPE)
        putchar('p');
    else if (mode & F_BDEV)
        putchar('b');
    else if (mode & F_CDEV)
        putchar('c');

    for (i = 0; i < 3; i++)
    {
        if (mode & OWN_RD)
            putchar('r');
        else
            putchar('-');
        if (mode & OWN_WR)
            putchar('w');
        else
            putchar('-');
        if (mode & OWN_EX)
            putchar('x');
        else
            putchar('-');
        mode = mode << 3;
    }
}

dump_inode(struct cinode *inode)
{
    int i, size;
    struct direct *d;
    char b[BSIZE];
    if (inode == NULL)
    {
        printf("NULL\n");
        return;
    }
    printf("#[%d] l:%d s:%d ", inode->c_num, inode->c_node.i_nlink, inode->c_node.i_size);
    lsmode(inode->c_node.i_mode);
    putchar('\n');

    if (inode->c_node.i_mode & F_DIR)
    {
        size = inode->c_node.i_size;
        for (i = 0; i < INODEADDR; i++)
        {
            if (inode->c_node.i_addr[i] == 0)
            {
                break;
            }
            bread(inode->c_node.i_addr[i], b);
            d = b;
            while (size > 0)
            {
                printf("%s (inode= %d)", d->name, d->ino);
                if (d->name[0] != '.')
                {
                    dump_inode(iget(d->ino));
                }
                putchar('\n');
                d++;
                size -= sizeof(struct direct);
            }
        }
    }
}

hexdump_b(char *buf, int size)
{
    int i, k;
    for (i = 0; i < size; i++)
    {
        printf("%02x ", (int)buf[i]);
        if (i != 0 && i % 16 == 0)
        {
            putchar('\n');
        }
    }
    putchar('\n');
}

hexdump(struct cinode *n)
{
    int i, k;
    char buffer[BSIZE];
    k = iread(n, 0, buffer, 64);
    printf("read %d\n", k);
    for (i = 0; i < k; i++)
    {
        printf("%02x ", (int)buffer[i]);
        if (i != 0 && i % 16 == 0)
        {
            putchar('\n');
        }
    }
    putchar('\n');
}

ls_inode(struct cinode *inode)
{
    int i, n;
    struct direct dir[DIRPERBL];
    struct cinode *f;
    if (inode == NULL)
    {
        printf("ls: NULL\n");
        return;
    }
    if (!(inode->c_node.i_mode & F_DIR))
    {
        printf("ls: not a dir\n");
        return;
    }
    n = iread(inode, 0, dir, sizeof(dir));
    n = n / sizeof(struct direct);
    for (i = 0; i < n; i++)
    {
        f = iget(dir[i].ino);
        printf("%d\t ", f->c_num);
        lsmode(f->c_node.i_mode);
        printf(" %d\t%d\t%s\n",
               f->c_node.i_nlink,
               f->c_node.i_size,
               dir[i].name);
        iput(f);
    }
}

ls(char *name)
{
    struct cinode *p = cwd;
    printf("ls: %s\n", name);
    struct cinode *n = iname(&p, name);
    iput(p);
    if (n == NULL)
    {
        printf("ls: '%s' not found\n", name);
        return;
    }
    ls_inode(n);
    iput(n);
}

dump_refs()
{
    int i;
    for (i = 0; i < NINODES; i++)
    {
        if (inode[i].c_num != 0)
        {
            printf("inode[%d]=%d ref %d\n", i, inode[i].c_num, inode[i].c_refs);
        }
    }
}

strpref(char *s1, char *s2)
{
    return strncmp(s1, s2, strlen(s2));
}

int main()
{
    int i;
    f = fopen("fs.img", "r+");
    iinit();
    while (1)
    {
        char *line = readline("# ");
        char *p;
        if (strcmp(line, "exit") == 0)
        {
            goto done;
        }
        if (strpref(line, "ref") == 0)
        {
            dump_refs();
        }
        else if (strpref(line, "cd") == 0)
        {
            p = line;
            while (*p && *p != ' ')
            {
                p++;
            }
            while (*p && *p == ' ')
            {
                p++;
            }
            struct cinode *n = iname(&cwd, p);
            if (n == NULL)
            {
                iput(cwd);
                printf("cd: %s: no such file or dir\n", p);
            }
            else
            {
                iput(cwd);
                iput(cwd);
                cwd = n;
                dump_refs();
            }
        }
        else if (strpref(line, "mkdir") == 0)
        {
            p = line;
            while (*p && *p != ' ')
            {
                p++;
            }
            while (*p && *p == ' ')
            {
                p++;
            }
            struct cinode *n = mkdir(p);
            if (n)
            {
                iput(n);
            }
        }
        else if (strpref(line, "ls") == 0)
        {
            p = line;
            while (*p && *p != ' ')
            {
                p++;
            }
            while (*p && *p == ' ')
            {
                p++;
            }
            if (!*p)
            {
                ls(".");
            }
            else
            {
                ls(p);
            }
        }
    doneline:
        free(line);
    }

#if 0
    struct cinode *n = mknode(rootdir, "abra");
    /* dump_inode(n); */
    iwrite(n, 0, "hello", 6);
    hexdump(n);
    iput(n);

    printf("ls /\n");
    ls_inode(rootdir);
    n = mkdir(rootdir, "tmp");
    if (n != NULL)
    {
        struct cinode *fn = mknode(n, "test.txt");
        iwrite(fn, 0, "shell\nfille\n", sizeof("shell\nfille\n"));
        iput(fn);
        printf("ls /tmp\n");
        ls_inode(n);
        iput(n);
    }

    struct cinode *pn = rootdir;
    n = iname(&pn, "/tmp/tesdt.txt");
    iput(pn);

    printf("ls /\n");
    ls_inode(rootdir);


    ls("/tmp/../tmp/../tmp");
#endif
done:
    iput(cwd);
    iput(rootdir);

    dump_refs();
    fclose(f);
}