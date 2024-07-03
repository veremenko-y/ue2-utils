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
    struct header buf;
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
        fread((char *)&buf, sizeof(buf), 1, f);
        if (buf.magic != MAGIC)
        {
            printf("size: %s not an object file\n", *argv);
            fclose(f);
            continue;
        }
        if (gorp > 2)
            printf("\n%s: \n", *argv);
        puts("===segments===");
        printf("text: %u (0x%04x)\ndata: %u (0x%04x)\nbss: %u (0x%04x)\n",
            buf.textsize, buf.textsize,
            buf.datasize, buf.datasize,
            buf.bsssize, buf.bsssize);
        sum = buf.textsize + buf.datasize + buf.bsssize;
        printf("total: %u (0x%04x)\n\n", sum);

        puts("===aux===");
        print2("header", sizeof(struct header));
            print2("const", buf.consize);
        if (buf.hasrel)
        {
            print2("rtext", buf.textsize);
            print2("rdata", buf.datasize);
        }
        /* printf("entry: 0x%04x\n", buf.entry); */
        if (buf.symsize)
        {
            symsize = buf.symsize * sizeof(struct sym);
            printf("sym: %u (0x%04x)\t%u entries\n",
                   symsize, symsize, buf.symsize);
        }

        sum = buf.textsize +
              buf.datasize +
              symsize +
              (buf.hasrel ? buf.textsize + buf.datasize : 0) +
              sizeof(struct header);

        printf("\nfile size: %u (0x%04x)\n", sum, sum);

/*         printf("\n===offsets==\n");

        print2("text off", T_OFFSET(buf));
        print2("data off", D_OFFSET(buf));
        print2("rtext off", TREL_OFFSET(buf));
        print2("rdata off", DREL_OFFSET(buf));
        print2("sym off", SYM_OFFSET(buf)); */

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
