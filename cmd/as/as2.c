#include <stdio.h>
#include "as.h"

struct header hdr;
assemble()
{
    int c;
    fout = fopen(outname, "w");
    hdr.magic = MAGIC_OBJ;
    /* hdr.hasrel = 1; */
    hdr.textsize = segsize[SEGTEXT];
    hdr.datasize = segsize[SEGDATA];
    hdr.bsssize = segsize[SEGBSS];
    hdr.consize = segsize[SEGCONST];
    hdr.trelsize = segsize[SEGTEXT + SIZERELOFF];
    hdr.drelsize = segsize[SEGDATA + SIZERELOFF];
    if (hdr.textsize & 1)
    {
        /* error("text misaligned"); */
    }
    if (hdr.datasize & 1)
    {
        /* error("data misaligned"); */
    }
    hdr.symsize = symscnt - symstart;
    fwrite(&hdr, sizeof(hdr), 1, fout);
    for (passno = 0; passno < sizeof(segout) / sizeof(segout[0]); passno++)
    {
        while ((c = getc(segout[passno])) >= 0)
        {
            putc(c, fout);
        }
    }
    fwrite((syms + symstart), sizeof(struct sym), hdr.symsize, fout);

    fclose(fout);
}