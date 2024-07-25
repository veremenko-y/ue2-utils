#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>

#define BLKSZ 256
#define FATBLKN 15
#define HDRROOT 16
#define HDRNODES 9
#define HDRLAST (HDRROOT + HDRSN - 1)
#define HDRSN 16
#define NODEBLKS 8

#define MAXBLK (8 * BLKSZ * FATBLKN)
#define MAXNODES (HDRSN * HDRNODES)
#define MAXFSSIZE (MAXBLK * BLKSZ)

#define INODE(i, j) ((i << 4) + j)

#define DBG 1

#if DBG == 1
#define dbg(x, ...)
#else
void dbg(const char *fmt, va_list argp)
{
    printf(fmt, argp);
}
#endif

/* Structure representing file.
 nlen - length of the name 0 if free
 size - size in bytes
 blocks - N blocks in the header, otherwise follow `next` to get another block with remaining data*/
struct u2fs_node
{
    char nlen;
    char name[7];
    unsigned short size;
    unsigned short blocks[NODEBLKS];
    unsigned short next;
} __attribute__((packed));

struct u2fs_header
{
    struct u2fs_node nodes[HDRNODES];
    char padding[4];
} __attribute__((packed));

struct u2fs_file
{
    int inode;
    unsigned short pos;
    struct u2fs_node node;
    unsigned short nextblks[BLKSZ / sizeof(unsigned short)];
};

struct u2fs_fs
{
    int blk;
    int node;
};

// char *files[] = {
//     "sh.bin",
//     "mon.bin",
//     "ls.bin"};

// #define FILES 3
//char bootpath[] = "swboot.bin";

int fsize(FILE *f)
{
    fseek(f, 0, SEEK_END);
    int sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    return sz;
}

unsigned short bswap_16(unsigned short i)
{
    return (i << 8) | (i >> 8);
}

char *strindex(char *s, char c)
{
    do
    {
        if (*s == c)
        {
            return s;
        }
    } while (*s++ != '\0');
}

FILE *fimg;

int read_block(int blk, char *buf)
{
    fseek(fimg, blk * BLKSZ, SEEK_SET);
    int read = fread(buf, sizeof(char), BLKSZ, fimg);
    if (read < BLKSZ)
    {
        memset((buf + read), 0, BLKSZ - read);
    }
    return read;
}

int write_block(int blk, char *buf)
{
    fseek(fimg, blk * BLKSZ, SEEK_SET);
    int written = fwrite(buf, sizeof(char), BLKSZ, fimg);
    return written;
}

int allocate_block()
{
    char buf[BLKSZ];
    for (int i = 1; i < FATBLKN; i++)
    {
        read_block(i, buf);
        for (int j = 0; j < BLKSZ; j++)
        {
            if (buf[j] != 0xFF)
            {
                int alloc = 1 << 7;
                for (int k = 0; k < 8; k++)
                {
                    if ((buf[j] & alloc) == 0)
                    {
                        buf[j] |= alloc;
                        write_block(i, buf);
                        return ((i - 1) * 256) + (j * 8) + k;
                    }
                    alloc >>= 1;
                }
            }
        }
    }
    return -1;
}

// int create_node(char *name)
// {
//     struct u2fs_header *hdr;
//     char buf[BLKSZ];
//     for (int i = HDRROOT; i <= HDRLAST; i++)
//     {
//         read_block(i, buf);
//         hdr = (struct u2fs_header *)&buf;
//         for (int j = 0; j < HDRNODES; j++)
//         {
//             if (hdr->nodes[j].nlen == 0)
//             {
//                 hdr->nodes[j].nlen = strlen(name);
//                 strcpy(hdr->nodes[j].name, name);
//                 hdr->nodes[j].blocks[0] = bswap_16(allocate_block());
//                 return INODE(i, j);
//             }
//         }
//         write_block(i, buf);
//     }
// }

void read_node(int inode, struct u2fs_node *node)
{
    int blknum = inode >> 4;
    int nodenum = inode & 0xf;
    char buf[BLKSZ];
    read_block(blknum, buf);
    struct u2fs_header *hdr = (struct u2fs_header *)&buf;
}


int u2fs_size(struct u2fs_file *f) {
    return bswap_16(f->node.size);
}


struct u2fs_file *u2fs_find(char *name)
{
    struct u2fs_header hdr;
    for (int i = HDRROOT; i <= HDRLAST; i++)
    {
        read_block(i, (char *)&hdr);
        for (int j = 0; j < HDRNODES; j++)
        {
            if (strcmp(hdr.nodes[j].name, name) == 0)
            {
                struct u2fs_file *f = malloc(sizeof(struct u2fs_file));
                memset(f, 0, sizeof(struct u2fs_file));
                f->inode = INODE(i, j);
                f->node = hdr.nodes[j];
                if (f->node.next != 0)
                {
                    read_block(bswap_16(f->node.next), (char *)&f->nextblks);
                }
                return f;
            }
        }
    }
    return NULL;
}

struct u2fs_file *u2fs_create(char *name)
{
    struct u2fs_header hdr;
    for (int i = HDRROOT; i <= HDRLAST; i++)
    {
        read_block(i, (char *)&hdr);
        for (int j = 0; j < HDRNODES; j++)
        {
            if (hdr.nodes[j].nlen == 0)
            {
                hdr.nodes[j].nlen = strlen(name);
                strcpy(hdr.nodes[j].name, name);
                write_block(i, (char *)&hdr);
                struct u2fs_file *f = malloc(sizeof(struct u2fs_file));
                memset(f, 0, sizeof(struct u2fs_file));
                f->inode = INODE(i, j);
                f->node = hdr.nodes[j];
                if (f->node.next != 0)
                {
                    read_block(bswap_16(f->node.next), (char *)&f->nextblks);
                }
                return f;
            }
        }
    }
    return NULL;
}

struct u2fs_fs *u2fs_openfs()
{
    struct u2fs_fs *fs = malloc(sizeof(struct u2fs_fs));
    fs->blk = HDRROOT;
    fs->node = 0;
    return fs;
}

struct u2fs_node *u2fs_readnode(struct u2fs_fs *fs)
{
    static struct u2fs_node n;
    if (fs->blk > HDRLAST)
    {
        return NULL;
    }
    struct u2fs_header hdr;
    read_block(fs->blk, (char *)&hdr);
    n.nlen = 0;
    while (1)
    {
        if (hdr.nodes[fs->node].nlen != 0)
        {
            n = hdr.nodes[fs->node];
        }
        fs->node++;
        if (fs->node >= HDRNODES)
        {
            fs->node = 0;
            fs->blk++;
            if (fs->blk > HDRLAST)
            {
                return NULL;
            }
            read_block(fs->blk, (char *)&hdr);
        }
        if (n.nlen != 0)
        {
            return &n;
        }
    }
    return NULL;
}

void u2fs_closefs(struct u2fs_fs *fs)
{
    free(fs);
}

struct u2fs_file *u2fs_open(char *name)
{
    struct u2fs_file *file = u2fs_find(name);
    if (file == NULL)
    {
        file = u2fs_create(name);
    }
    return file;
}

int u2fs_get_file_block(struct u2fs_file *f, int pos)
{
    int size = u2fs_size(f);
    if (pos >= size)
    {
        return 0;
    }
    int blkPos = f->pos >> 8;
    int blk;
    if (blkPos < NODEBLKS)
    {
        blk = bswap_16(f->node.blocks[blkPos]);
        return blk;
    }

    if (f->node.next == 0)
    {
        return 0;
    }
    blkPos -= 2;
    return bswap_16(f->nextblks[blkPos]);
}

void u2fs_upd_node(struct u2fs_file *f)
{
    struct u2fs_header hdr;
    int blk = f->inode >> 4;
    int node = f->inode & 0xf;
    read_block(blk, (char *)&hdr);
    hdr.nodes[node] = f->node;
    write_block(blk, (char *)&hdr);
}

void u2fs_grow(struct u2fs_file *f, int grow)
{
    int offset = f->pos;
    while (grow > 0)
    {
        int blkPos = offset >> 8;
        int blk;
        if (blkPos < NODEBLKS)
        {
            if (f->node.blocks[blkPos] == 0)
            {
                blk = allocate_block();
                f->node.blocks[blkPos] = bswap_16(blk);
            }
        }
        else
        {
            if (f->node.next == 0)
            {
                blk = allocate_block();
                f->node.next = bswap_16(blk);
                read_block(blk, (char *)&f->nextblks);
            }
            blkPos -= 2;
            if (f->nextblks[blkPos] == 0)
            {
                blk = allocate_block();
                f->nextblks[blkPos] = bswap_16(blk);
                write_block(bswap_16(f->node.next), (char *)&f->nextblks);
            }
        }
        int rem = BLKSZ - (u2fs_size(f) & 0xff);
        if (rem <= grow)
        {
            grow -= rem;
            offset += rem;
            f->node.size = bswap_16(u2fs_size(f) + rem);
        }
        else
        {
            f->node.size = bswap_16(u2fs_size(f) + grow);
            offset += grow;
            grow -= grow;
        }
        u2fs_upd_node(f);
    }
}

void u2fs_write(const char *ptr, int size, int n, struct u2fs_file *f)
{
    int cnt = size * n;
    int new_size = f->pos + cnt;
    char buf[BLKSZ];
    if (new_size > u2fs_size(f))
    {
        u2fs_grow(f, cnt);
    }
    while (cnt > 0)
    {
        int bytes = cnt;
        if (bytes > BLKSZ)
        {
            bytes = BLKSZ;
        }
        int blk = u2fs_get_file_block(f, f->pos);
        memcpy(buf, ptr, bytes);
        if (bytes < BLKSZ)
        {
            memset(buf + bytes, 0, BLKSZ - bytes);
        }
        write_block(blk, buf);
        ptr += bytes;
        cnt -= bytes;
        f->pos += bytes;
    }
}

void u2fs_close(struct u2fs_file *f)
{
    free(f);
}

int read_img(char * imgpath)
{
    char buf[BLKSZ];
    struct u2fs_header *hdr;

    if (imgpath == NULL) {
        printf("error: image path not supplied.\n");
        return -1;
    }
    fimg = fopen(imgpath, "r");

    memset(buf, 0, BLKSZ);

    int fssize = 0;
    for (int i = 1; i < 1 + FATBLKN; i++)
    {
        read_block(i, buf);
        for (int j = 0; j < BLKSZ; j++)
        {
            char fat = buf[j];
            for (int k = 0; k < 8; k++)
            {
                if ((fat & 0x01) != 0)
                {
                    fssize++;
                }
                fat >>= 1;
            }
        }
    }
    printf("MAX FILES: %d\n", MAXNODES);
    printf("BLOCKS Total: %d Used: %d Free: %d.\n", MAXBLK, fssize, MAXBLK - fssize);
    printf("\tBOOT: %d\n", 1);
    printf("\tFAT: %d\n", FATBLKN);
    printf("\tHEADERS: %d\n", HDRSN);
    int overhead = FATBLKN + HDRSN + 1;
    printf("\tDATA: %d\n", fssize - overhead);
    printf("BYTES Total: %d Used: %d Free: %d.\n", MAXBLK * 256, fssize * 256, (MAXBLK - fssize) * 256);
    struct u2fs_fs *fs = u2fs_openfs();
    struct u2fs_node *node;
    while ((node = u2fs_readnode(fs)) != NULL)
    {
        printf("\t%s\t%d\t%d bytes\n",
               node->name,
               (node->size & 0xff),
               bswap_16(node->size));
    }

    fclose(fimg);
}

int create_img(char* imgpath, char* bootpath, char** files, int filesn)
{
    char blockBuf[BLKSZ];
    struct u2fs_header *hdr;

    if (imgpath == NULL) {
        printf("error: image path not supplied.\n");
        return -1;
    }
    fimg = fopen(imgpath, "w+");

    memset(blockBuf, 0, BLKSZ);
    fseek(fimg, MAXFSSIZE - 1, SEEK_SET);
    fwrite(blockBuf, sizeof(char), 1, fimg); // set img file size
    int reservedBlocks = 1 + FATBLKN + HDRSN;
    printf("\nReserved blocks: %d.\n", reservedBlocks);
    while (reservedBlocks-- > 0)
    {
        allocate_block();
    }

    FILE *f = fopen(bootpath, "r");
    if(f == NULL) {
        printf("warning: boot image not supplied.\n");
    }else {
        int size = fsize(f);
        printf("writting bootstrap '%s', %d bytes.\n", bootpath, size);
        fread(blockBuf, sizeof(char), BLKSZ, f);
        fclose(f);
    }
    fseek(fimg, 0, SEEK_SET);
    fwrite(blockBuf, sizeof(char), BLKSZ, fimg);
    printf("files to write %d\n",filesn);

    for (int i = 0; i < filesn; i++)
    {
        f = fopen(files[i], "r");
        int size = fsize(f);

        char *dot = strindex(files[i], '.');
        char name[8];
        int j;
        for (j = 0; j < dot - files[i]; j++)
        {
            name[j] = toupper(files[i][j]);
        }
        name[j] = 0;
        printf("writing %s '%s', %d bytes.\n", name, files[i], size);
        struct u2fs_file *file = u2fs_open(name);
        if (!file)
        {
            printf("error: could not open file %s\n", name);
            return -1;
        }
        char buf[BLKSZ];
        int r;

        while ((r = fread(buf, sizeof(char), BLKSZ, f)) != 0)
        {
            u2fs_write(buf, sizeof(char), r, file);
        }

        u2fs_close(file);
        printf("done\n");

        fclose(f);
    }
    fclose(fimg);
}

int main(int argc, char **argv)
{
    if(sizeof(struct u2fs_header) != BLKSZ) {
        printf("Block size is incorrect %d\n", sizeof(struct u2fs_header));
        return -1;
    }

    int option;
    int is_read = 0;
    char* imgpath = NULL;
    char* bootpath = NULL;
    char* files[MAXNODES];
    opterr = 0; /* turn off error messages */
    while ((option = getopt(argc, argv, "ho:r:b:")) != -1)
    {
        switch (option)
        {
        case 'h': /* help */
            printf("usage: %s [-h] [-r img] [-o img]\n", argv[0]);
            break;
        case 'o': /* out */
            imgpath = optarg;
            break;
        case 'b':
            bootpath = optarg;
            break;
        case 'r': /* read */
            is_read = 1;
            imgpath = optarg;
            break;
        default: /* ? */
            printf("option not recognized: %c\n", optopt);
        }
    }
    int filesn = 0;
    for (int i = optind; i < argc; i++)
    {
        files[filesn++] = argv[i];
    }
    if(is_read != 0) {
        return read_img(imgpath);
    }else {
        return create_img(imgpath, bootpath, files, filesn);
    }
    // printf("sizeof(struct u2fs_header) = %d\n", sizeof(struct u2fs_header));
    // printf("sizeof(struct u2fs_node) = %d\n", sizeof(struct u2fs_node));
    // if (argc > 1 && strcmp(argv[1], "-read") == 0)
    // {
    //     return read_img();
    // }
    // else
    // {
    //     return create_img(argc, argv);
    // }
}