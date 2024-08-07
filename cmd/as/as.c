#include <stdio.h>
#include "as.h"

FILE *fin;
FILE *fout;
FILE *segout[4];
uint8_t passno;
uint16_t lineno;
uint16_t charno;
char* outname = "a.out";

error(format, args) char *format;
va_list *args;
{
    fprintf(stderr, "pass %d line %d at %d: ", passno, lineno, charno);
    fprintf(stderr, format, args);
    fprintf(stderr, "\n");
    unlink(outname);
    exit(EXIT_FAILURE);
}

main(argc, argv) char **argv;
{
    int i;
    if (argc > 1)
    {
        fin = fopen(argv[1], "r");
        argc--;
        i = 2;
        while (argc > 1)
        {
            argc--;
            if(argv[i][0] != '-')
                error("invalid arg '%s'", argv[1]);
            switch (argv[i][1])
            {
            case 'o':
                if(argc <= 1)
                    error("missing name");
                argc--;
                outname = argv[++i];
                break;
            default:
                error("switch '%c'", argv[1][1]);
            }
            i++;
        }
    }else {
        fin = stdin;
    }

    syminit();
    fout = tmpfile();
    INFO("pass 0");
    scan();
    symdump();

    fclose(fin);
    fin = fout;
    fout = NULL;
    fseek(fin, 0, SEEK_SET);

    INFO("pass 1");
    parse();
    symdump();

    INFO("pass 2");
    for (i = 0; i < sizeof(segsize) / sizeof(segsize[0]); i++)
    {
        segsize[i] = 0;
    }
    fseek(fin, 0, SEEK_SET);

    segout[0] = tmpfile();
    segout[1] = tmpfile();
    segout[2] = tmpfile();
    segout[3] = tmpfile();

    passno = 1;
    parse();

    symdump();

    INFO("pass 3");
    fseek(segout[0], 0, SEEK_SET);
    fseek(segout[1], 0, SEEK_SET);
    fseek(segout[2], 0, SEEK_SET);
    fseek(segout[3], 0, SEEK_SET);
    assemble();

/*     for (i = 0; i < sizeof(segout) / sizeof(segout[0]); i++)
    {
        fclose(segout[i]);
        segout[i] = NULL;
    }
 */    return 0;
}