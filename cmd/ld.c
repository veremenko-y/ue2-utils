#include <obj.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dbg.h>

/*
Symbols format:
symbol:
    type:
        UNDEF        = 0
        ABSOLUTE     = 1 << 0
        RELATIVE     = 1 << 1
        CONST        = 1 << 2
        # Additional flags
        EXPORT       = 1 << 6
        CONST EXPORT = 1 << 7


Relocation format:

text word
    <ins:4>[<addr:12>|<value:12>]
relocation
    [<const:1><segment:3>]<value:12>
    if rel = 0 => nothing to relocate


Immediate value
    text := <symbol.value>|<value>
    rel := 0
    symbol.segment := 0 - defined, doesn't matter

Absolute const value
    symbol["#<value>"] := <value> + <const-expression>
    symbol.type = ABS
    text := <value> + <const-expression>
    rel := <const:1><value=0>

Absolute symbol
    text := <value>|<symbol.value>
    rel := 0

if not const
Defined: -- local
        text := <symbol.value + const-expression>
        rel := <symbol.segment:3><value=0>
Undefined: -- import
        text := <symbol.value> + <const-expression>
        rel := <symbol.index + 1>

if const
Defined: -- local
        newsymbol["#<symbol.name>"] := <symbol.index>
        newsymbol.type = CONST
        text := <const-expression>
        rel := <const:1><newsymbol.index +1>
Undefined:
        newsymbol["#<symbol.name>"] := <symbol.index>
        newsymbol.type = CONST
        symbol.type |= CONSTEXPORT
        text := <const-expression>
        rel := <const:1><newsymbol.index +1>


*/

struct lsym
{
    uint8_t name[NAMESZ + 1];
    uint8_t type;
    uint8_t segm;
    addr_t value;
    addr_t caddr;
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

#define GETCONSTADDR(x) ((corigin + consts[x] - 1))

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

uint16_t torigin;
uint16_t dorigin;
uint16_t borigin;
uint16_t corigin;

uint16_t tsize;
uint16_t dsize;
uint16_t csize;
uint16_t bsize;

FILE *fout;
FILE *fout2;
int errors;

/* output params */
char *outname = "a.out";
uint8_t b_flag; /* binary output */
uint8_t s_flag; /* strip output */
uint8_t x_flag; /* only preserve external symbols */

FILE *flbl; /* output labels */

syminit()
{
    symssize = 100;
    syms = calloc(symssize, sizeof(struct lsym));
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

symidxfind(idx) irword_t idx;
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

symprintp(struct sym *p, int i)
{
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

symprint(int i)
{
    if (i >= symscnt)
    {
        printf("NOT YET\n");
        return;
    }

    struct sym *p = &syms[i];
    symprintp(p, i);
}

symdump()
{
    uint16_t i;
    printf("Symbols:\n");
    for (i = 0; i < symscnt; i++)
    {
        printf("cadr[%04x] ", syms[i].caddr);
        symprint(i);
        puts("");
    }
}
/* #define symdump(...) */

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
    if (IS_RELOCATABLE(hdr.magic) == 0)
        error("invalid file");

    fseek(f, SYMOFFSET(hdr), SEEK_SET);
    while ((read = fread(&sym, sizeof(sym), 1, f)) > 0)
    {
        if ((sym.type & ~(SYMCONST)) != SYMUNDEF &&
            (sym.type & SYMEXPORT) == 0 &&
            (sym.type & SYMCOEXPORT) == 0)
        {
            if (x_flag)
            {
                rsymcnt++;
                continue;
            }
        }
        name = &sym.name;
        memcpy(strbuf, sym.name, NAMESZ + 1);
        symfind();
        INFO("searched %s found %s N %d\n", strbuf, cursym->name, cursymn);
        if (!cursym->name[0])
        {
            symnew();
            *(struct sym *)cursym = sym;
            cursym->caddr = rsymcnt;
            symreloc();
            /* printf("new %p ", cursym);
            symprint(cursymn);
            puts(""); */
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
    if (IS_MAGIC_VALID(hdr.magic) == 0)
        error("invalid file");

    fseek(f, SYMOFFSET(hdr), SEEK_SET);
    while ((read = fread(&sym, sizeof(sym), 1, f)) > 0)
    {
        /* Find unresolved consts */
        /* symprintp(&sym, 0);
        puts(""); */
        memcpy(strbuf, sym.name, NAMESZ);
        symfind();
        if (cursym->name[0] && (cursym->type & SYMTYPE) == SYMUNDEF)
        {
            printf("ld: '%s' undefined\n", cursym->name);
            errors++;
            continue;
        }
        if ((sym.type & (SYMCONST | SYMREL)) != SYMCONST)
        {
            continue;
        }

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
            printf("searching %s\n", sym.name + 1);
            memcpy(strbuf, sym.name + 1, NAMESZ);
            symfind();
            if (!cursym->name[0] || (cursym->type & SYMTYPE) == SYMUNDEF)
            {
                error("undef '%s'", sym.name);
            }
            else
            {
                /*
                probably dont neeed (see below)
                cursym->type &= ~(SYMCOEXPORT); */
            }
        }
        if ((consym->type & (SYMCONST | SYMREL)) == SYMCONST) /* e.g. undefined const */
        {
            if (consts[cursym->value] == 0)
            {
                consts[cursym->value] = ++consize;
                consym->value = consts[cursym->value] + dreloc - 1;
            }
        }

        /*
        we want to keep constants exported
        consym->type &= ~(SYMABS | SYMEXPORT | SYMCOEXPORT);*/
        consym->type |= SYMREL;
        consym->segm = SEGDATA;
    }
}

/* freadir(irword_t *ir, FILE *f)
{
    int i, c, r;
    r = 0;
    *ir = 0;
    for(i = 0; i < IRSIZE; i++)
    {
        *ir <<= 8;
        c = getc(f);
        if(c >= 0) r++;
        *ir |= c;
    }
    return r;
} */

fwriteir(irword_t *ir, FILE *f)
{
    int i;
    char c;
    uint8_t buf[IRSIZE];
    for (i = 0; i < IRSIZE; i++)
    {
        buf[i] = *ir & 0xff;
        *ir >>= 8;
        /* c = (*ir) & (0xff << (IRSIZE - i - 1)); */
        /* c = ((char *)ir)[i]; */
        /* printf("%x at %d\n", c, IRSIZE - i - 1); */
        /* putc(c, f); */
    }
    fwrite(buf, sizeof(uint8_t), IRSIZE, f);
}

/* Resolve relocations and write output */
load3(f, frel, iscode)
    FILE *f;
FILE *frel;
{
    FILE *out = fout;
    fread(&hdr, sizeof(hdr), 1, f);
    if (IS_MAGIC_VALID(hdr.magic) == 0)
        error("invalid file");
    fseek(f, CODEOFFSET(hdr), SEEK_SET);
    fseek(frel, RELCODEOFFSET(hdr), SEEK_SET);
    irword_t code;
    irword_t rel;
    char codebuf[IRSIZE];
    char relbuf[IRSIZE];
    int addr = 0;
    int size = hdr.textsize + hdr.datasize;
    int read, c;
    while (addr < size)
    {
        if (addr == hdr.textsize)
        {
            out = fout2;
        }
        code = read = rel = 0;
        while (read < IRSIZE)
        {
            c = fgetc(f);
            if (c < 0)
                break;
            codebuf[read] = c;
            code <<= 8;
            code |= c;
            if (read == 0 || rel != 0)
            {
                c = fgetc(frel);
                /* printf("%x %x %d\n", rel, c, read); */
                relbuf[read] = c;
                rel <<= 8;
                rel |= c;
            }
            read++;
            if(rel == 0) {
                break;
            }
        }
        /* printf("code %x rel %x\n", code, rel); */
        if (rel == 0)
        {
            fwrite(codebuf, sizeof(codebuf[0]), read, out);
            addr += read;
            continue;
        }
        else if (rel & RELCONST)
        {
            if ((rel & ~(RELCONST)) == 0)
            {
                printf("%x at $%x\n", code & DATAMAX, GETCONSTADDR(code & 0xff));
                code = (code & IRMASK) | GETCONSTADDR(code & 0xff); /* Set const value from table */
            }
            else
            {
                symidxfind((rel & ~(RELCONST)));
                code = (code & IRMASK) + cursym->value;
            }
            INFO("CONST %04x ==> %04x\n", rel & ~(RELCONST), code);
        }
        else if (rel & RELSEG)
        {
            int seg = ((rel & RELSEG) >> RELSEGSHIFT) - 1;
            INFO("REG %d %d\n", seg, rel);
            INFO("%04x ==> ", code);
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
            INFO("%04x\n", code);
            INFO("%d\n", rel & ~(RELSEG));
        }
        else if (rel != 0)
        {
            puts("index");
            symidxfind(rel - 1);
            INFO("REF %d name %s\n", rel - 1, cursym->name);
            code = code + cursym->value; /* value + reloc offset */
            INFO("%04x\n", code);
        }
        /* bswap(&code); */
        fwriteir(&code, out);
        /* fwrite(&code, sizeof(code), 1, out); */
        addr += read;
    }
    treloc += hdr.textsize;
    dreloc += hdr.datasize;
    breloc += hdr.bsssize;
}

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
            switch (argv[args][1])
            {
            case 'b':
                b_flag++;
                break;
            case 'x':
                x_flag++;
                break;
            case 's':
                s_flag++;
                break;
            case 'L':
                char tmp[256];
                sprintf(tmp, "%s.labels", outname);
                flbl = fopen(tmp, "w");
                break;
            case 'o':
                if (++args < argc)
                {
                    outname = argv[args];
                }
                else
                {
                    error("no name");
                }
                break;
            default:
                error("invalid flag '%c'", argv[args][1]);
                break;
            }
            args++;
        }
        else
        {
            break;
        }
    }
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        if (fin == NULL)
        {
            error("can't open %s", argv[i]);
        }
        load1(fin);
        fclose(fin);
    }
    INFO("pass 2");
    torigin = 0;      /* TODO: add load addr */
    dorigin = treloc; /* for constant region */
    corigin = dorigin + dreloc;
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        load2(fin);
        fclose(fin);
    }

    if (errors > 0)
    {
        error("ld: failed to link");
    }

    borigin = corigin + consize;
    INFO("data: %x consts: %x bss: %x\n", dorigin, corigin, borigin);
    for (i = 0; i < symscnt; i++)
    {
        p = &syms[i];
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
    }

  /*   symdump(); */

    INFO("consts: %d", consize);
    for (i = 0; i < 256; i++)
    {
        if (consts[i] != 0)
        {
            INFO("[$%x]=$%x\n", i, GETCONSTADDR(i));
        }
    }

    fout = fopen(outname, "w");
    fout2 = tmpfile();
    tsize = treloc;
    dsize = dreloc;
    csize = consize;
    bsize = breloc;
    if (!b_flag)
    {
        hdr.magic = MAGIC_EXE;
        /* hdr.hasrel = 0; */
        hdr.textsize = tsize;
        hdr.datasize = dsize + consize;
        hdr.consize = csize;
        hdr.bsssize = bsize;
        hdr.symsize = symscnt;

        if (b_flag || s_flag)
        {
            hdr.symsize = 0;
        }
        fwrite(&hdr, sizeof(hdr), 1, fout);
    }
    treloc = dreloc = breloc = 0;
    INFO("pass 3");
    for (i = args; i < argc; i++)
    {
        fin = fopen(argv[i], "r");
        finr = fopen(argv[i], "r");
        load3(fin, finr, fout, fout2);
        fclose(fin);
        fclose(finr);
    }
    fseek(fout2, 0, SEEK_SET);
    while ((i = getc(fout2)) >= 0)
    {
        putc(i, fout);
    }
    fclose(fout2);

    for (i = 0; i < UINT8_MAX + 1; i++)
    {
        if (consts[i] != 0)
        {
            putc(i, fout);
        }
    }

    for (i = 0; i < symscnt; i++)
    {
        if (!b_flag && !s_flag)
        {
            fwrite(&syms[i], sizeof(struct sym), 1, fout);
        }
        if (flbl != NULL && (syms[i].type & ~(SYMCONST)) != SYMUNDEF)
        {
            fprintf(flbl, "%s=%04x\n", syms[i].name, syms[i].value);
        }
    }
    fclose(fout);
    if (flbl != NULL)
        fclose(flbl);
}