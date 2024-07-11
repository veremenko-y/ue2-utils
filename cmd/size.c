#include <stdio.h>
#include <obj.h>

/*
    size -- determine object size
*/

print2(name, size)
    uint8_t *name;
uint16_t size;
{
    printf("%s: %d (0x%04x)\n", name, size, size);
}

main(argc, argv) char **argv;
{
    struct header hdr;
    int sum;
    int gorp, i;
    int symsize;
    FILE *f;

    if (argc == 1)
    {
        *argv = "a.out";
        argc++;
        --argv;
    }
    gorp = argc;
    while (--argc)
    {
        ++argv;
        if ((f = fopen(*argv, "r")) == NULL)
        {
            printf("size: %s not found\n", *argv);
            continue;
        }
        fread((char *)&hdr, sizeof(hdr), 1, f);
        if (IS_MAGIC_VALID(hdr.magic) == 0)
        {
            printf("size: %s not an object file\n", *argv);
            fclose(f);
            continue;
        }
        if (gorp > 2)
            printf("\n%s: \n", *argv);
        printf("text: %u (0x%04x) data: %u (0x%04x) bss: %u (0x%04x) ",
               hdr.textsize, hdr.textsize,
               hdr.datasize, hdr.datasize,
               hdr.bsssize, hdr.bsssize);
        sum = hdr.textsize + hdr.datasize + hdr.bsssize;
        printf("total: %u (0x%04x)\n", sum);
        if (hdr.symsize)
        {
            symsize = hdr.symsize * sizeof(struct sym);
            printf("sym: %u (0x%04x)\t%u entries\n",
                   symsize, symsize, hdr.symsize);
        }
        else
        {
            symsize = 0;
        }

        sum = hdr.textsize +
              hdr.datasize +
              symsize +
              (hdr.hasrel ? hdr.textsize + hdr.datasize : 0) +
              sizeof(struct header);

#if 0    
        printf("hasrel: %d rel: %d\n", hdr.hasrel, (hdr.hasrel ? (hdr.textsize + hdr.datasize) : 0));
        printf("hdr.symsize: %d symsize: %d\n", hdr.symsize, symsize);
#endif
        printf("file size: %u (0x%04x)\n", sum, sum);

        fseek(f, 0L, SEEK_END);
        i = ftell(f);
        if (i != sum)
        {
            fprintf(stderr, "!!! file size mismatch !!!\n");
            fprintf(stderr, "expected %d got %d\n", sum, i);
        }
        fclose(f);
    }
}
