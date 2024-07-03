#include <obj.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

struct lsym
{
    uint8_t name[NAMESZ + 1];
    uint8_t type;
    uint8_t segm;
    word_t value;
    word_t caddr;
} PACKED;

error(format, args) char *format;
va_list *args;
{
    fprintf(stderr, format, args);
    fprintf(stderr, "\n");
    exit(2);
}

bswap(p)
    uint16_t *p;
{
    *p = (*p >> 8) | (*p << 8);
}

/* symbols */
struct lsym *syms;
struct lsym *cursym;
uint16_t rsymcnt;
uint16_t symscnt;
uint16_t cursymn;
uint16_t symssize;
char strbuf[NAMESZ + 1];

/* file & relocations */
struct header hdr;
struct sym sym;
uint16_t treloc;
uint16_t dreloc;
uint16_t breloc;

uint16_t consize;
uint16_t consts[UINT8_MAX + 1];

uint16_t dorigin;
uint16_t borigin;
uint16_t corigin;

uint16_t tsize;
uint16_t dsize;
uint16_t csize;
uint16_t bsize;

FILE *fout;
FILE *fout2;

syminit()
{
    symssize = 100;
    syms = malloc(symssize * sizeof(struct lsym));
}

symfind()
{
    cursymn = 0;
    cursym = syms;
    while (cursymn < symscnt)
    {
        if (cursym->name[0] == 0)
            break;
        if (strcmp(strbuf, cursym->name) == 0)
            break;
        cursym++;
        cursymn++;
    }
}

symidxfind(idx) uint16_t idx;
{
    cursymn = 0;
    cursym = syms;
    while (cursymn < symscnt)
    {
        if (cursym->name[0] == 0)
            break;
        if (cursym->caddr == idx)
            break;
        cursym++;
        cursymn++;
    }
}

symnew()
{
    cursymn = symscnt;
    cursym = &syms[symscnt++];
    if (symscnt >= symssize)
    {
        error("oo syms");
    }
}

symprint(int i)
{
    if (i >= symscnt)
    {
        printf("NOT YET");
        return;
    }

    struct sym *p = &syms[i];
    printf("%-8s[%04x]%8s", p->name, i, "");
    if (p->segm == SEGTEXT)
    {
        printf("[CODE] ");
    }
    else if (p->segm == SEGDATA)
    {
        printf("[DATA] ");
    }
    else if (p->segm == SEGBSS)
    {
        printf("[BSS] ");
    }
    else
    {
        printf("[<invalid>%d] ", p->segm);
    }
    int t = p->type & SYMTYPE;
    if (t == SYMUNDEF)
        printf("UNDEF ");
    if (t & SYMABS)
        printf("ABS ");
    if (t & SYMREL)
        printf("REL ");
    if (t & SYMCONST)
    {
        if ((t & ~(SYMCONST)) == SYMUNDEF)
        {
            printf("UNDEF ");
        }
        printf("CONST ");
    }

    if (p->type & SYMEXPORT)
        printf("EXPORT ");
    if (p->type & SYMCOEXPORT)
        printf("CONEXPORT ");
    printf("= 0x%x", p->value);
}

symdump()
{
    uint16_t i;
    printf("Symbols:\n");
    for (i = 0; i < symscnt; i++)
    {
        printf("[%04x] ", syms[i].caddr);
        symprint(i);
        puts("");
    }
}

symreloc()
{
    if (cursym->type & SYMABS)
    {
        return;
    }
    switch (cursym->segm)
    {
    case SEGTEXT:
        cursym->value += treloc;
        break;
    case SEGDATA:
        cursym->value += dreloc;
        break;
    case SEGBSS:
        cursym->value += breloc;
        break;
    }
}

/* Find and resolve exported and undefined symbols */
load1(f)
    FILE *f;
{
    int read;
    char *name;
    fread(&hdr, sizeof(hdr), 1, f);
    if (hdr.magic != MAGIC)
        error("invalid file");

    fseek(f, SYMOFFSET(hdr), SEEK_SET);
    while ((read = fread(&sym, sizeof(sym), 1, f)) > 0)
    {
        if ((sym.type & ~(SYMCONST)) != SYMUNDEF &&
            (sym.type & SYMEXPORT) == 0 &&
            (sym.type & SYMCOEXPORT) == 0)
        {
            rsymcnt++;
            continue;
        }
        name = &sym.name;
        memcpy(strbuf, sym.name, NAMESZ + 1);
        symfind();
        if (!cursym->name[0])
        {
            symnew();
            *(struct sym *)cursym = sym;
            cursym->caddr = rsymcnt;
            symreloc();
            printf("new %p ", cursym);
            symprint(cursymn);
            puts("");
        }
        else
        {
            if ((cursym->type & SYMTYPE) != SYMUNDEF)
            {
                continue; /*  already defined, skipping */
            }
            if (sym.type == SYMUNDEF)
            {
                continue; /* still undefined, skipping */
            }
            cursym->type = sym.type;
            cursym->segm = sym.segm;
            cursym->value = sym.value;
            symreloc();
        }
        rsymcnt++;
    }

    treloc += hdr.textsize;
    dreloc += hdr.datasize;
    breloc += hdr.bsssize;
}

/* Resolve constant relocaitons */
load2(f)
    FILE *f;
{
    int read;
    struct lsym *consym;
    fread(&hdr, sizeof(hdr), 1, f);
    if (hdr.magic != MAGIC)
        error("invalid file");

    fseek(f, SYMOFFSET(hdr), SEEK_SET);
    while ((read = fread(&sym, sizeof(sym), 1, f)) > 0)
    {
        /* Find unresolved consts */
        if ((sym.type & (SYMCONST | SYMREL)) != SYMCONST)
        {
            continue;
        }
        memcpy(strbuf, sym.name, NAMESZ);
        symfind();
        if ((cursym->type & (SYMCONST | SYMREL)) != SYMCONST)
        {
            continue;
        }
        consym = cursym;
        if (cursym->type & SYMABS)
        {
        }
        else
        {

            memcpy(strbuf, sym.name + 1, NAMESZ);
            symfind();
            if (!cursym->name[0] || (cursym->type & SYMTYPE) == SYMUNDEF)
            {
                error("undef '%s'", sym.name);
            }
            else
            {
                cursym->type &= ~(SYMCOEXPORT);
            }
        }
        if (consts[cursym->value] == 0)
        {
            consts[cursym->value] = ++consize;
        }
        consym->value = consts[cursym->value] + dreloc - 1;
        consym->type &= ~(SYMABS | SYMEXPORT | SYMCOEXPORT);
        consym->type |= SYMREL;
        consym->segm = SEGDATA;
    }
}

/* Resolve relocations and write output */
load3(f, frel, iscode)
    FILE *f;
FILE *frel;
{
    FILE *out = fout;
    fread(&hdr, sizeof(hdr), 1, f);
    if (hdr.magic != MAGIC)
        error("invalid file");
    fseek(f, CODEOFFSET(hdr), SEEK_SET);
    fseek(frel, RELCODEOFFSET(hdr), SEEK_SET);
    word_t code;
    word_t rel;
    int addr = 0;
    int size = hdr.textsize + hdr.datasize;
    while (addr < size)
    {
        if(addr == hdr.textsize) {
            out = fout2;
        }
        fread(&code, sizeof(code), 1, f);
        bswap(&code);
        fread(&rel, sizeof(rel), 1, frel);
        printf("%04x: ", addr);
        if (rel & RELCONST)
        {
            printf("CONST %04x\n", rel & ~(RELCONST));
            symidxfind((rel & ~(RELCONST)));
            symprint(cursymn);
            printf("\n%04x ==> ", code);
            code = (code & 0xf000) + cursym->value;
            printf("%04x\n", code);
        }
        else if (rel & RELSEG)
        {
            int seg = (((rel & RELSEG) >> (RELSEGSHIFT))) - 1;
            printf("REG %d %d\n", seg, rel);
            printf("%04x ==> ", code);
            switch (seg)
            {
            case SEGTEXT:
                code += treloc;
                break;
            case SEGDATA:
                code += dorigin + dreloc;
                break;
            case SEGBSS:
                code += borigin + breloc;
                break;
            default:
                error("invalid reloc");
            }
            printf("%04x\n", code);
            printf("%d\n", rel & ~(RELSEG));
        }
        else if (rel != 0)
        {
            rel--;
            printf("REF %04x\n", rel);
            symidxfind(rel);
            symprint(cursymn);
            printf("\n%04x ==> ", code);
            code = (code & 0xf000) + cursym->value;
            printf("%04x\n", code);
        }
        else
        {
            printf("%04x\n", code);
        }
        bswap(&code);
        fwrite(&code, sizeof(code), 1, out);
        addr += sizeof(word_t);
    }
    treloc += hdr.textsize;
    dreloc += hdr.datasize;
    breloc += hdr.bsssize;
}

uint8_t b_flag;

main(argc, argv) int argc;
char **argv;
{
    int i;
    int args;
    FILE *fin;
    FILE *finr;
    struct sym *p;
    syminit();
    args = 1;
    while (args < argc)
    {
        if (argv[args][0] == '-')
        {
            if (argv[args][1] == 'b')
            {
                b_flag++;
            }
            args++;
        }
        else
        {
            break;
        }
    }
    printf("%d\n", args);
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        puts(argv[i]);
        load1(fin);
        fclose(fin);
    }
    dorigin = treloc; /* for constant region */
    corigin = dorigin + dreloc;
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        puts(argv[i]);
        load2(fin);
        fclose(fin);
    }

    borigin = corigin + consize;
    printf("do %x co %x bo %x\n", dorigin, corigin, borigin);
    for (i = 0; i < symscnt; i++)
    {
        p = &syms[i];
        symprint(i);
        putchar('\n');
        switch (p->segm)
        {
        case SEGDATA:
            p->value += dorigin;
            break;
        case SEGBSS:
            p->value += borigin;
            break;
        case SEGTEXT:
            break; /* nothing */
        default:
            error("duh %d %d", i, p->segm);
            break;
        }
        symprint(i);
        putchar('\n');
    }

    symdump();

    printf("consts: %d\n", consize);
    for (i = 0; i < 256; i++)
    {
        if (consts[i] != 0)
        {
            printf("[$%x]=$%x\n", i, (corigin + consts[i] - 1));
        }
    }

    fout = fopen("a.out", "w");
    fout2 = fopen("a.outd", "w");
    tsize = treloc;
    dsize = dreloc;
    csize = consize;
    bsize = breloc;
    if (!b_flag)
    {
        hdr.hasrel = 0;
        hdr.textsize = tsize;
        hdr.datasize = dsize + consize;
        hdr.consize = csize;
        hdr.bsssize = bsize;
        hdr.symsize = symscnt;
        fwrite(&hdr, sizeof(hdr), 1, fout);
    }
    treloc = dreloc = breloc = 0;
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        finr = fopen(argv[i], "r");
        puts(argv[i]);
        load3(fin, finr, fout, fout2);
        fclose(fin);
        fclose(finr);
    }
    fclose(fout2);
    fin = fopen("a.outd", "r");
    while((i = getc(fin)) >= 0)
    {
        putc(i, fout);
    }
    fclose(fin);
    unlink("a.outd");

    for (i = 0; i < UINT8_MAX + 1; i++)
    {
        if (consts[i] != 0)
        {
            putc(i, fout);
        }
    }
    if (!b_flag)
    {
        for (i = 0; i < symscnt; i++)
        {
            fwrite(&syms[i], sizeof(struct sym), 1, fout);
        }
    }
    fclose(fout);
}