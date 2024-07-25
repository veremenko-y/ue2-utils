#ifndef FS_H
#define FS_H

#include <stdint.h>

#define B_WRITE 0
#define B_READ 1

#define FREEBCACHE 32 /* free blocks in superblock/memory */
#define FREEICACHE 32 /* free i-nodes in superblock/memory */
#define BSIZE 512
#define BSHIFT 9 /* LOG2(BSIZE) */
#define	DIRSIZ	14
#define INODEPERBL (BSIZE / sizeof(struct dinode))
#define DIRPERBL (BSIZE / sizeof(struct direct))
/* Bit masks for i_mode and st_mode */

#define OTH_EX  0001
#define OTH_WR  0002
#define OTH_RD  0004
#define GRP_EX  0010
#define GRP_WR  0020
#define GRP_RD  0040
#define OWN_EX  0100
#define OWN_WR  0200
#define OWN_RD  0400

#define SAV_TXT 01000
#define SET_GID 02000
#define SET_UID 04000

#define MODE_MASK 07777

#define F_REG   0100000
#define F_DIR   040000
#define F_PIPE  010000
#define F_BDEV  060000
#define F_CDEV  020000

#define F_MASK  0170000

typedef uint16_t blkno_t;
typedef uint16_t ino_t;


struct filesys
{
    uint16_t fsize;  /* size in blocks */
    uint16_t isize;  /* i-list size */
    uint16_t tfree;  /* total free blocks */
    uint16_t tinode; /* total free inodes */

    uint16_t lfree_n;
    blkno_t lfree[FREEBCACHE];

    uint16_t lifree_n;
    ino_t lifree[FREEICACHE];
};

struct	direct
{
	ino_t	ino;
	char	name[DIRSIZ];
};

#define INODEADDR 20

struct dinode {
    uint16_t i_mode; /* mode and type of file */
    uint16_t i_nlink; /* number of links to file */
    uint16_t i_uid;    /* owner's user id */
    uint16_t i_gid;    /* owner's group id */
    uint32_t i_size;    /* number of bytes in file */
    uint32_t i_atime;    /* time last accessed */ 
    uint32_t i_mtime;    /* time last modified */
    uint32_t i_ctime;    /* time created */ 
    blkno_t  i_addr[INODEADDR];  /* disk block addresses */
};               /* Exactly 64 bytes long! */

/* In memory inode structure */
typedef struct cinode {
    uint16_t   c_magic;         /* Used to check for corruption. */
    uint16_t   c_dev;           /* Inode's device */
    uint16_t   c_num;           /* Inode # */
    struct dinode     c_node;		/* On disk inode data */
    uint8_t    c_refs;          /* In-core reference count */
    uint8_t    c_readers;	/* Count of readers by oft entry */
    uint8_t    c_writers;	/* Count of writers by oft entry */
    uint8_t    c_flags;           
#define CDIRTY		0x80	/* Modified flag. */
#define CRDONLY		0x40	/* On a read only file system */
#define CFLOCK		0x0F	/* flock bits */
#define CFLEX		0x0F	/* locked exclusive */
#define CFMAX		0x0E	/* highest shared lock count permitted */
   /* uint8_t     c_super; */		/* Superblock index */
} cinode, *inoptr;

struct freeblk
{
    uint16_t lfree_n;
    blkno_t lfree[FREEBCACHE];
};


void dump_filesys(struct filesys *fs)
{
    int i;
    printf("fsize= %d\n", fs->fsize);
    printf("isize= %d\n", fs->isize);
    printf("tfree= %d\n", fs->tfree);
    printf("tinode= %d\n", fs->tinode);
    /* for (i = 0; i < fs->lfree_n; i++)
    {
        printf("lfree[%d]= %d\n", i, fs->lfree[i]);
    } */
    printf("lfree_n= %d\n", fs->lfree_n);
/*     for (i = 0; i < fs->lifree_n; i++)
    {
        printf("lifree[%d]= %d\n", i, fs->lifree[i]);
    } */
    printf("lifree_n= %d\n", fs->lifree_n);
}


#endif