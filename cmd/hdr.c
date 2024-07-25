#include <stdio.h>
#include <stdlib.h>
#include <obj.h>
struct header hdr;
FILE *fi;

main(argc, argv) char **argv;
{
    if (--argc == 0)
    {
        argc = 1;
        argv[1] = "a.out";
    }
    while (argc--)
    {
        fi = fopen(*++argv, "r");
        if (fi == NULL)
        {
            fprintf(stderr, "cannot open %s\n", *argv);
            continue;
        }
        fread((char *)&hdr, 1, sizeof(struct header), fi); /* get magic no. */
        if (IS_MAGIC_VALID(hdr.magic) == 0)
        {
            fprintf(stderr, "%s-- bad format\n", *argv);
            continue;
        }

        printf("\n%s:\n", *argv);
        printf("magic:\t");
        if (hdr.magic == MAGIC_OBJ)
        {
            printf("OBJ");
        }
        if (hdr.magic == MAGIC_RELOC)
        {
            printf("RELOC");
        }
        if (hdr.magic == MAGIC_EXE)
        {
            printf("EXE");
        }
        printf(" load: %04x\n",hdr.load);
        /* printf("hasrel:\t%c\n", hdr.hasrel ? 'y' : 'n'); */
        printf("text: %04x ", hdr.textsize);
        printf("data: %04x ", hdr.datasize);
        printf("bss: %04x ", hdr.bsssize);
        printf("con: %02x ", hdr.consize);
        printf("sym: %04x\n", hdr.symsize);
        printf("trel: %04x\n", hdr.trelsize);
        printf("drel: %04x\n", hdr.drelsize);
        fclose(fi);
    }
}
