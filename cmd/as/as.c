#include <stdio.h>
#include "as.h"

FILE *fout;
FILE *segout[4];
uint8_t passno;
uint16_t lineno;
char* outname = "a.out";

nothing() {}

error(format, args) char *format;
va_list *args;
{
    fprintf(stderr, "line %d: ", lineno);
    fprintf(stderr, format, args);
    fprintf(stderr, "\n");
    exit(2);
}

main(argc, argv) char **argv;
{
    int i;
    if (argc > 1)
    {
        freopen(argv[1], "r", stdin);
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
    }

    syminit();
    fout = fopen("out.tokens", "w");
    INFO("======== PASS 0 ========\n");
    scan();
    fclose(fout);
    symdump();

    freopen("out.tokens", "r", stdin);
    INFO("======== PASS 1 ========\n");
    parse();
    symdump();

    INFO("======== PASS 2 ========\n");
    for (i = 0; i < sizeof(segsize) / sizeof(segsize[0]); i++)
    {
        segsize[i] = 0;
    }
    fseek(stdin, 0, SEEK_SET);

    segout[0] = fopen("out.code", "w");
    segout[1] = fopen("out.data", "w");
    segout[2] = fopen("out.relcode", "w");
    segout[3] = fopen("out.reldata", "w");

    passno = 1;
    parse();
    fclose(stdin);

    for (passno = 0; passno < sizeof(segout) / sizeof(segout[0]); passno++)
    {
        fclose(segout[passno]);
    }
    symdump();

    INFO("======== PASS 3 ========\n");
    segout[0] = fopen("out.code", "r");
    segout[1] = fopen("out.data", "r");
    segout[2] = fopen("out.relcode", "r");
    segout[3] = fopen("out.reldata", "r");
    assemble();
    for (passno = 0; passno < sizeof(segout) / sizeof(segout[0]); passno++)
    {
        fclose(segout[passno]);
    }
    return 0;
}