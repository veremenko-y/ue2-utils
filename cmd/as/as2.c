#include <stdio.h>
#include "as.h"

assemble()
{
    int c;
    fout = fopen("a.out", "w");
    putc(MAGIC, fout);
    putc(1, fout);
    fwrite(&segsize[SEGTEXT], sizeof(segsize[SEGTEXT]), 1, fout);
    fwrite(&segsize[SEGDATA], sizeof(segsize[SEGTEXT]), 1, fout);
    fwrite(&segsize[SEGBSS], sizeof(segsize[SEGTEXT]), 1, fout);
    symscnt -= symstart;
    fwrite(&symscnt, sizeof(symscnt), 1, fout);

    for(passno = 0; passno < sizeof(segout) / sizeof(segout[0]); passno++)
    {
        while((c = getc(segout[passno])) >= 0)
        {
            putc(c, fout);
        }
    }
    fwrite((syms + symstart), sizeof(struct sym), symscnt, fout);

    fclose(fout);
}