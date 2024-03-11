#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <obj.h>
#include "ar.h"
#include "dbg.h"

void error(char *format, ...);

/*
 * address reference types
 */
#define PCREL 1
#define LEN1 0
#define LEN2 2
#define LEN4 4

#define REFMASK 0x7
#define REXT (01 << 3)
#define ROFF (02 << 3)

/*FILE *infile;
FILE *rinfile;*/
struct hdr filhdr;

int errlev;
int delarg = 4;

FILE *tout;
FILE *dout;
char doutn[NCPS + 10];
FILE *trout;
char troutn[NCPS + 10];
FILE *drout;
char droutn[NCPS + 10];
FILE *sout;
char soutn[NCPS + 10];

struct symtab *hshtab[NHASH];
uint16_t hshused = 0;
struct symtab *symtab;
struct symtab *symcur;
struct symtab *symend;
uint16_t nsym; /* number of symbols allocated in symtab */
struct symtab *lastsym;
struct symtab cursym;

uint8_t ofilfnd;
char *ofilename = "l.out";
uint16_t infil;
char *filname;

int textbase;
/* cumulative sizes set in pass 1 */
int tsize;
int dsize;
int bsize;
int trsize;
int drsize;
int ssize;

/* symbol relocation; both passes */
int ctrel;
int cdrel;
int cbrel;
int ctorel;
int cdorel;
int cborel;

/* used after pass 1 */
int torigin;
int dorigin;
int borigin;
int database;

uint8_t rflag; /* preserve relocation bits, don't define common */
uint8_t sflag; /* discard all symbols */
uint8_t dflag; /* define common even with rflag */

struct stream
{
    FILE *file;
    int globpos;
    int globsize;
    int pos;
    int len;
    char tag[NCPS];
};

struct stream text;
struct stream reloc;

bswap16(w)
    uint16_t *w;
{
    *w = (*w << 8) | (*w >> 8);
}

char *printsym(sym)
struct symtab *sym;
{
    static char buf[1024];
    char *t = buf;
    t += sprintf(t, "\t'%s'[idx: %d] type=%d (", sym->name, sym->index, sym->type);
    if (sym->type == 0)
        t += sprintf(t, " XUNDEF ");
    if ((sym->type & XABS) == XABS)
        t += sprintf(t, " XABS ");
    if ((sym->type & XDATA) == XDATA)
        t += sprintf(t, " XDATA ");
    if ((sym->type & XTEXT) == XTEXT)
        t += sprintf(t, " XTEXT ");
    if ((sym->type & XBSS) == XBSS)
        t += sprintf(t, " XBSS ");
    if ((sym->type & XDATAO) == XDATAO)
        t += sprintf(t, " XDATAO ");
    if ((sym->type & XBSSO) == XBSSO)
        t += sprintf(t, " XBSSO ");
    if ((sym->type & XTEXTO) == XTEXTO)
        t += sprintf(t, " XTEXTO ");
    if ((sym->type & XABSO) == XABSO)
        t += sprintf(t, " XABSO ");
    if ((sym->type & XUNDEFO) == XUNDEFO)
        t += sprintf(t, " XUNDEFO ");
    if ((sym->type & XTXRN) == XTXRN)
        t += sprintf(t, " XTXRN ");
    if ((sym->type & XXTRN) == XXTRN)
        t += sprintf(t, " XXTRN ");
    if ((sym->type & XTYPE) == XTYPE)
        t += sprintf(t, " XTYPE ");
    if ((sym->type & XFORW) == XFORW)
        t += sprintf(t, " XFORW ");
    t += sprintf(t, ") val=0x%x\n", sym->value);
    *t = '\0';
    return buf;
}

cp8c(from, to) uint8_t *from, *to;
{
    uint8_t *f, *t, *te;

    f = from;
    t = to;
    te = t + NCPS;
    while ((*t++ = *f++) && t < te)
        ;
    while (t < te)
        *t++ = 0;
}

dseek(sp, loc, size) struct stream *sp;
uint16_t loc;
uint16_t size;
{
    TRACE1("[%s] seeking to 0x%04x.0x%04x\n", sp->tag, loc, size);
    fseek(sp->file, loc, SEEK_SET);
    sp->globpos = loc;
    if (size != -1)
    {
        sp->pos = 0;
        sp->len = size;
        if ((sp->globsize - sp->globpos) < size)
        {
            error("failed to seek for %s", sp->tag);
        }
    }
    else
    {
        sp->pos = sp->globpos;
        sp->len = sp->globsize;
    }
}

getfile(cp) uint8_t *cp;
{
    FILE *f;
    if ((f = fopen(cp, "r")) == 0)
    {
        error("cannot open %s", cp);
    }
    text.file = f;
    text.globpos = 0;
    fseek(f, 0, SEEK_END);
    text.globsize = ftell(f);
    strcpy(text.tag, "text");
    fseek(f, 0, SEEK_SET);
    if ((f = fopen(cp, "r")) == 0)
    {
        error("cannot open %s", cp);
    }
    reloc.file = f;
    reloc.globpos = 0;
    reloc.globsize = text.globsize;
    strcpy(reloc.tag, "reloc");
    /* always not library */
    return 0;
}

uint8_t get(sp)
struct stream *sp;
{
    size_t r;
    uint8_t c;

    TRACE3("[%s] pos=%04x (%d) len=%d\n", sp->tag, sp->pos, sp->pos, sp->len);
    if (sp->len < 1)
    {
        error("get: stream overflow");
    }
    r = fread(&c, 1, sizeof(c), sp->file);
    if (r <= 0)
    {
        error("io");
    }
    sp->len--;
    sp->pos++;
    sp->globpos++;
    return c;
}

uint16_t get16(sp)
struct stream *sp;
{
    size_t r;
    uint16_t c;
    TRACE3("[%s] pos=%04x (%d) len=%d\n", sp->tag, sp->pos, sp->pos, sp->len);
    if (sp->len < 2)
    {
        error("get: stream overflow");
    }
    r = fread(&c, 1, sizeof(c), sp->file);
    if (r <= 0)
    {
        error("io");
    }
    sp->len -= 2;
    sp->pos += 2;
    sp->globpos += 2;
    return c;
}

mget(dest, n, sp)
    uint8_t *dest;
uint16_t n;
struct stream *sp;
{
    int r;
    if (sp->len < n)
    {
        error("invalid offset or size for %s", sp->tag);
    }
    r = fread(dest, 1, n, sp->file);
    sp->pos += r;
    sp->len -= r;
    sp->globpos += r;
}

char *dechex(name, size)
char *name;
uint16_t size;
{
    static char buf[256];
    sprintf(buf, "%s: %d (0x%04x)\n", name, size, size);
    return buf;
}

readhdr(off)
{
    dseek(&text, off, sizeof(filhdr));
    mget(&filhdr, sizeof(filhdr), &text);
    if (filhdr.magic != A_MAGIC1 && filhdr.magic != A_MAGIC2 &&
        filhdr.magic != A_MAGIC3 && filhdr.magic != A_MAGIC4)
    {
        error("bad magic");
    }

    /* copied from bsd3 ld. something is fishy about this*/
    if (filhdr.magic == A_MAGIC2)
    {
        cdrel = -filhdr.tsize;
        cbrel = cdrel - filhdr.dsize;
    }
    else
    {
        error("bad format");
    }
}

symreloc()
{
    switch (cursym.type & 0xf)
    {
    case XTEXT:
    case XXTRN + XTEXT:
        cursym.value += ctrel;
        return;
    case XDATA:
    case XXTRN + XDATA:
        cursym.value += cdrel;
        return;
    case XBSS:
    case XXTRN + XBSS:
        cursym.value += cbrel;
        return;
    case XXTRN + XUNDEF:
        return;
    }
    if (cursym.type & XXTRN)
    {
        cursym.type = XXTRN + XABS;
    }
}

struct symtab *
symalloc()
{
    uint8_t *p;
    symcur++;
    if (symcur >= symend)
    {
        symend = sbrk(200 * sizeof(struct symtab));
        if ((int)symend == -1)
        {
            error("no mem");
            /* TODO: delete files */
        }
        symend = sbrk(0);
        p = symend;
        while (p > (uint8_t *)symcur)
        {
            *--p = 0;
        }
    }
    nsym++;
    return (symcur - 1);
}

enter(hp) struct symtab **hp;
{
    uint8_t *p1;
    uint8_t *p2;
    if (*hp == 0)
    {
        *hp = lastsym = symalloc();
        for (p1 = cursym.name, p2 = (*hp)->name; p1 < cursym.name + NCPS;)
            *p2++ = *p1++;
        (*hp)->type = cursym.type;
        (*hp)->index = hp - hshtab;
        (*hp)->value = cursym.value;
        return 1;
    }
    else
    {
        lastsym = *hp;
        return 0;
    }
}

struct symtab **
lookup()
{
    int hash;
    int i;
    char *p1;
    char *p2;
    struct symtab **hp;
    for (hash = 0, p1 = cursym.name; *p1; hash <<= 2, hash += *p1++)
        ;
    hash += p1[-1] << 5;
    hash %= NHASH;
    if (hash < 0)
        hash += NHASH;
    hp = &hshtab[hash];
    hash = 1; /*now, it counts the number of times we rehash*/
    while (*hp)
    {
        p1 = cursym.name;
        p2 = (*hp)->name;
        TRACE1("searching %s found %s\n", p1, p2);

        for (i = 0; (i < NCPS) && *p1 && *p2; i++)
            if (*p1++ != *p2++)
                goto no;
        if (i >= NCPS) /*both symbols are maximal length*/
            return (hp);
        if (*p2 == 0) /*assert *p1 == 0*/
            return (hp);
    no:
        hp += hash;
        hash += 2;
        if (hp >= &hshtab[NHASH])
            hp -= NHASH;
    }
    if (++hshused >= HASHCLOGGED)
    {
        puts("Symbol table overflow");
        exit(2);
    }
    return hp;
}

load1(libflg, off)
{
    long loc;
    int r;
    int ndef;
    struct symtab *sp;
    struct symtab *save;
    readhdr(off);
    loc = off + SYM_OFFSET(filhdr);
    dseek(&text, loc, filhdr.ssize);
    save = symcur;

    ctrel = tsize;
    cdrel += dsize;
    cbrel += bsize;
    TRACE1("trel %d drel %d brel %d\n", ctrel, cdrel, cbrel);
    ndef = 0;
    TRACE1("symbols size %d\n", filhdr.ssize);
    while (text.len > 0)
    {
        mget(&cursym, sizeof(cursym), &text);
        if ((cursym.type & XXTRN) == 0)
        {
            continue;
        }
        TRACE1("searching\n");
        symreloc();
        dumpsymone(&cursym);
        if (enter(lookup(1)))
        {
            /* new symbol, keep going*/
            continue;
        }
        sp = lastsym;
        TRACE1("found\n");
        dumpsymone(sp);
        if (sp->type != XXTRN + XUNDEF)
        {
            TRACE1("not xxtrn\n");
            /* already defined, keep going*/
            continue;
        }
        if (cursym.type == XXTRN + XUNDEF)
        {
            TRACE1("is xxtrn\n");
            /* current is undefined, but found defined */
            if (cursym.value > sp->value)
                sp->value = cursym.value;
            continue;
        }
        if (sp->value != 0 && cursym.type == XXTRN + XTEXT)
        {
            continue;
        }
        TRACE1("new value\n");
        ndef++;
        sp->type = cursym.type;
        sp->value = cursym.value;
        dumpsymone(sp);
        dumpsymone(&cursym);
    }
    if (libflg == 0 || ndef)
    {
        tsize += filhdr.tsize;
        dsize += filhdr.dsize;
        bsize += filhdr.bsize;
        trsize += filhdr.trsize;
        drsize += filhdr.drsize;
        return 1;
    }

    /*
     * No symbols defined by this library member.
     * Rip out the hash table entries and reset the symbol table.
     */
    while (symcur > save)
        hshtab[(--symcur)->index] = 0;
    return 0;
}

symwrite(sp, n, f) struct symtab *sp;
FILE *f;
{
    fwrite(sp, sizeof(*symtab), n, f);
}

mkfsym(s) char *s;
{
    if (sflag)
    {
        return;
    }
    /*if (sflag || xflag)
        return;*/
    cp8c(s, cursym.name);
    cursym.type = XTEXT;
    cursym.value = torigin;
    symwrite(&cursym, 1, sout);
}

load1arg(cp) char *cp;
{
    int noff, nbno;
    if (getfile(cp) == 0)
    {
        load1(0, 0, 0);
    }
    /*nbno = 0;
    noff = 1;
    for (;;)
    {
        dseek(&text, nbno, noff, sizeof archdr);
        if (text.size <= 0)
        {
            libp->bno = -1;
            libp++;
            return;
        }
        mget(&archdr, sizeof archdr);
        if (load1(1, nbno, noff + (sizeof archdr) / 2))
        {
            libp->bno = nbno;
            libp->off = noff;
            libp++;
        }
        noff += (archdr.asize + sizeof archdr) >> 1;
        nbno += (noff >> 8) & 0377;
        noff &= 0377;
    }*/
    fclose(text.file);
    fclose(reloc.file);
}

load2arg(cp)
{
    int noff, nbno;
    if (getfile(cp) == 0)
    {
        load2(0, 0);
    }
    fclose(text.file);
    fclose(reloc.file);
}

load2(libflg, off)
{
    long loc;
    int off1, off2;
    struct symtab *sp;
    int16_t symno;
    readhdr(off);
    loc = off + SYM_OFFSET(filhdr);
    ctrel = torigin;
    cdrel += dorigin;
    cbrel += borigin;
    TRACE1("trel %d drel %d brel %d\n", ctrel, cdrel, cbrel);

    /*
     * Reread the symbol table, recording the numbering
     * of symbols for fixing external references.
     */
    dseek(&text, loc, filhdr.ssize);
    TRACE1("symbols size %d\n", filhdr.ssize);

    symno = -1;
    while (text.len > 0)
    {
        symno++;
        mget(&cursym, sizeof(cursym), &text);
        symreloc();
        TRACE1("Cur\n");
        dumpsymone(&cursym);
        if ((cursym.type & XXTRN) == 0)
        {
            continue;
        }
        if ((sp = *lookup()) == 0)
        {
            error("unexp symbol");
        }
        TRACE1("Found\n");
        dumpsymone(sp);
        if (cursym.type == XXTRN + XUNDEF)
        {
            sp->index = symno;
            continue;
        }
        if (cursym.type != sp->type ||
            cursym.value != sp->value)
        {
            printf(printsym(&cursym));
            printf(printsym(sp));
            error("mul def %s", sp->name);
        }
    }

    off1 = T_OFFSET(filhdr);
    off2 = TREL_OFFSET(filhdr);
    dseek(&text, off + off1, filhdr.tsize);
    dseek(&reloc, off + off2, filhdr.trsize);
    load2td(ctrel, tout, trout);

    off1 = D_OFFSET(filhdr);
    off2 = DREL_OFFSET(filhdr);
    dseek(&text, off + off1, filhdr.dsize);
    dseek(&reloc, off + off2, filhdr.drsize);
    load2td(cdrel, dout, drout);

    torigin += filhdr.tsize;
    dorigin += filhdr.dsize;
    borigin += filhdr.bsize;
    cdorel += filhdr.dsize;
    cborel += filhdr.bsize;
}

load2td(creloc, f, frel)
    uint16_t creloc;
FILE *f, *frel;
{
    int c;
    int r;
    int pos = 0;
    uint16_t tmp;
    uint16_t addr;
    uint16_t sym;
    uint8_t type;
    uint16_t value;
    uint16_t max;

    uint8_t opcode;

    while (1)
    {
        /* simply copy segment if no relocation found */
        if (reloc.len == 0)
        {
            TRACE1("Dumping rest of text of %s. Remaining %d bytes\n", text.tag, reloc.len);
            while (text.len)
            {
                putc(get(&text), f);
            }
            break;
        }
        addr = get16(&reloc);
        TRACE1("reading reloc. Address 0x%04x\n", addr);

        if (rflag)
        {
            /* remember for subsequent link editing */
            tmp = addr + creloc;
            TRACE1("writing frel. Address 0x%04x\n", tmp);
            fwrite(&tmp, sizeof(tmp), 1, frel);
        }
        while (text.pos < addr)
        {
            c = get(&text);
#if TRACE >= 3
            hexdump(&c, ftell(tout), 1, 1);
#endif
            TRACE3("writing f\n");
            putc(c, f); /* advance to proper position */
        }
        sym = get16(&reloc);
        type = get(&reloc);
        switch (type & 6)
        { /* read raw datum according to its length */
        case LEN1:
            value = get(&text);
            break;
        case LEN2:
            value = get16(&text);
            bswap16(&value);
            break;
        case LEN4:
            error("len4 uninmpl");
            break;
        }
        TRACE1("value is %04x\n", value);
        if (type & REXT)
        {
            lastsym = symtab;
            TRACE1("REXT sym index %d\n", sym);
            while (lastsym < symcur)
            {
                if (lastsym->index == sym)
                {
                    break;
                }
                lastsym++;
            }
            if (lastsym->name[0] == '\0')
            {
                error("local symbol not found");
            }
            TRACE1("Found REXT %s\n", lastsym->name);
            if (lastsym->type == XXTRN + XUNDEF)
            { /* still undefined */
                type = (type & (REFMASK + REXT + ROFF));
                sym = nsym + (lastsym - symtab);
            }
            else
            {
                TRACE1("type is %02x\n", lastsym->type);
                dumpsymone(lastsym);
                if (lastsym->type == XXTRN + XDATA && type & ROFF)
                {
                    sym = XDATAO;
                    type &= REFMASK;
                }
                else if (lastsym->type == XXTRN + XBSS && type & ROFF)
                {
                    sym = XBSSO;
                    type &= REFMASK;
                }
                else if (lastsym->type == XXTRN + XABS && type & ROFF)
                {
                    sym = XABSO;
                    type &= REFMASK;
                }
                else if (lastsym->type == XXTRN + XTEXT && type & ROFF)
                {
                    sym = XTEXTO;
                    type &= REFMASK;
                }
                else
                {
                    if (type & ROFF)
                    {
                        if (rflag)
                        {
                            error(0, "!-r; see JFR");
                            rflag = 0;
                        }
                    }
                    else
                    {
                        value += database;
                    }
                    sym = lastsym->type & XTYPE;
                    type &= REFMASK;
                }
                value += lastsym->value - database;
            }
        }
        else
        {
            switch (sym & XTYPE)
            {
            case XTEXT:
                value += ctrel;
                break;
            case XTEXTO:
                value += filhdr.tsize + ctrel - database;
                break;
            case XDATA:
                value += cdrel;
                break;
            case XDATAO:
                value += cdorel;
                break;
            case XBSS:
                value += cbrel;
                break;
            case XBSSO:
                value += cborel - filhdr.dsize;
                break;
            case XABSO:
                value += filhdr.tsize - database;
                break;
            }
        }

        if (rflag)
        { /* remember for subsequent link editing */
            fwrite(&sym, sizeof(sym), 1, frel);
            fwrite(&type, sizeof(type), 1, frel);
        }

        if (type & PCREL)
        {
             error("pcrel unsupported");
        }
        switch (type & 6)
        {
        case LEN1:
            max = UINT8_MAX;
            TRACE1("writing f\n");
            fwrite(&value, 1, 1, f);
            break;
        case LEN2:
            max = MAXADDR;
            bswap16(&value);
            fwrite(&value, 2, 1, f);
            break;
        }
        TRACE1("reloc value: %04x of type %d\n", value, type);
    }
}

middle()
{
    struct symtab *sp;
    int csize, t, corigin, ocsize;
    int nund, rnd;
    char s;

    torigin = 0;
    dorigin = 0;
    borigin = 0;

    /*
     * If there are any undefined symbols, save the relocation bits.
     */
    if (rflag == 0)
    {
        for (sp = symtab; sp < symcur; sp++)
            if (sp->type == XXTRN + XUNDEF && sp->value == 0)
            {
                printf("undef: %s\n", sp->name);
                rflag++;
                dflag = 0;
                break;
            }
    }
    if (rflag)
    {
        sflag = 0;
    }

    /*
     * Assign common locations.
     */
    csize = 0;
    database = tsize + textbase;
    /* junk? removed */
    /*
     * Now set symbols to their final value
     */
    /*csize = round(csize, FW);*/
    torigin = textbase;
    dorigin = database;
    corigin = dorigin + dsize;
    borigin = corigin + csize;

    cdorel = 0;
    cborel = dsize + csize;
    nund = 0;
    for (sp = symtab; sp < symcur; sp++)
    {
        TRACE1("final\n");
        dumpsymone(sp);
        switch (sp->type & XTYPE)
        {
        case XXTRN + XUNDEF:
            /* ORIG CODE
            if ((arflag==0 || dflag) && sp->svalue==0) {
                if (nund==0)
                    printf("Undefined:\n");
                nund++;
                printf("%.8s\n", sp->sname);
            }*/
            errlev |= 01;
            if (sp->value == 0)
            {
                printf("undef: %.8s\n", sp->name);
                nund++;
            }
            continue;

        case XXTRN + XABS:
        default:
            continue;

        case XXTRN + XTEXT:
            sp->value += torigin;
            continue;

        case XXTRN + XDATA:
            sp->value += dorigin;
            continue;

        case XXTRN + XBSS:
            sp->value += borigin;
            continue;

            /*case XXTRN+XUNDEFO:
                sp->stype = (sp->stype & STABTYPS) | (XXTRN+XBSS);
                sp->svalue += corigin;
                continue;*/
        }
    }
    /*if (xflag)
        ssize = 0;*/
    if (sflag)
    {
        ssize = 0;
    }
    bsize += csize;
    nsym = ssize / (sizeof cursym);
}

int tmpnamex;

FILE *
tcreat(namep, name)
char *namep,
    *name;
{
    FILE *fp;
    int i;
    char *n;
    n = namep;
    strcpy(n, name);
    TRACE1("base name: %s\n", name);
    TRACE1("copied name: %s\n", n);
    for (i = 0; n[i]; i++)
    {
        if (n[i] == 'X')
        {
            n[i] = 'A' + tmpnamex++;
        }
    }
    if ((fp = fopen(n, "w")) == NULL)
        error(1, "no temp");
    return (fp);
}

setupout()
{
    uint16_t bss;
    tout = fopen(ofilename, "w");
    if (tout == NULL)
    {
        error("cannot open");
    }
    dout = tcreat(&doutn, "l_data_X");
    /*if (sflag==0 || xflag==0)
        sout = tcreat(&soutn, "/tmp/ldbaXXXXX");
    */
    if (sflag == 0)
    {
        sout = tcreat(&soutn, "l_sym_X");
    }
    if (rflag)
    {
        trout = tcreat(&troutn, "l_rtext_X");
        drout = tcreat(&droutn, "l_rdata_X");
    }
    /*ORIG: filhdr.magic = nflag? A_MAGIC1:A_MAGIC2;*/
    filhdr.magic = A_MAGIC2;
    /*if (zflag)
        filhdr.magic = nflag?0413:0412;
    filhdr.tsize = nflag? tsize:tsize;
    */
    filhdr.tsize = tsize;
    filhdr.dsize = dsize;
    bss = bsize - (filhdr.dsize - dsize);
    if (bss < 0)
        bss = 0;
    filhdr.bsize = bss;
    filhdr.ssize = sflag ? 0 : (ssize + (sizeof cursym) * (symcur - symtab));
    /*if (entrypt) {
        if (entrypt->stype!=EXTERN+TEXT)
            error(0, "Entry point not in text");
        else
            filhdr.a_entry = entrypt->svalue;
    } else
        filhdr.a_entry=0;*/
    filhdr.trsize = (rflag ? trsize : 0);
    filhdr.drsize = (rflag ? drsize : 0);
    if (rflag)
    {
        fwrite(&filhdr, sizeof(filhdr), 1, tout);
    }
}

copy(np) char *np;
{
    int c;
    FILE *fp;

    TRACE1("Copying %s into %s\n", np, ofilename);
    if ((fp = fopen(np, "r")) == NULL)
        error("cannot recopy output %s", np);
    while ((c = getc(fp)) != EOF)
    {
        putc(c, tout);
#if TRACE >= 3
        hexdump(&c, ftell(tout), 1, 1);
#endif
    }
    fclose(fp);
}

finishout()
{
    TRACE1("Combining segments\n");
    /*if (!nflag)
    {
        while (tsize&FW) {
            putc(0, tout); tsize++;
        }
    }*/
    fclose(dout);
    copy(doutn);

    if (rflag)
    {
        fclose(trout);
        copy(troutn);
        fclose(drout);
        copy(droutn);
    }
    if (rflag && !sflag)
    {
        fclose(sout);
        copy(soutn);
        symwrite(symtab, symcur - symtab, tout);
    }
    /*
    if (sflag==0) {
        if (xflag==0) {
            fclose(sout);
            copy(soutn);
        }
        symwrite(symtab, nextsym-symtab, tout);
    }*/
    fclose(tout);
    /*if (!ofilfnd) {
        unlink("a.out");
        link("l.out", "a.out");
        ofilename = "a.out";
    }*/
}

delexit()
{
    /*unlink("l.out");*/
    unlink(doutn);
    unlink(troutn);
    unlink(droutn);
    unlink(soutn);
    if (delarg == 0)
        chmod(ofilename, 0777 & ~umask(0));
    exit(delarg);
}

dumpsymone(sym) struct symtab *sym;
{
    TRACE1(printsym(sym));
}

dumpsym()
{
    int i;
    TRACE1("symtab size %d\n", hshused);
    lastsym = symtab;
    while (lastsym < symcur)
    {
        dumpsymone(lastsym);
        lastsym++;
    }
    TRACE1("hashtab size: %d\n", NHASH);
    for (i = 0; i < NHASH; i++)
    {
        if (hshtab[i] != NULL)
        {
            lastsym = hshtab[i];
            TRACE1("hash %d\n", i);
            dumpsymone(lastsym);
        }
    }
}

void error(char *format, ...)
{
    fprintf(stderr, "error: ");
    va_list argp;
    va_start(argp, format);
    vfprintf(stderr, format, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    exit(1);
}

main(argc, argv) int argc;
char **argv;
{
    int c;
    int i;
    char *p;
    if (argc == 1)
    {
        error("no file");
    }

    symcur = symtab = sbrk(200 * sizeof(struct symtab));
    symend = sbrk(0);

    puts("pass 1");
    /* pass 1 */
    for (c = 1; c < argc; c++)
    {
        p = argv[c];
        if (p[0] == '-')
        {
            for (i = 1; p[i]; i++)
            {
                switch (p[i])
                {
                case 'o':
                    if (++c >= argc)
                    {
                        error("bad output file");
                    }
                    ofilename = argv[c];
                    TRACE1("-o %s\n", ofilename);
                    ofilfnd++;
                    break;
                case 's':
                    TRACE1("-s\n");
                    sflag++;
                    break;
                case 'r':
                    TRACE1("-r\n");
                    rflag++;
                    break;
                default:
                    error("bad flag %c",p[i]);
                    break;
                }
            }
        }
        else
        {
            load1arg(p);
        }
    }

    middle();
    setupout();
    puts("pass 2");
    /* pass 2 */
    for (c = 1; c < argc; c++)
    {
        p = argv[c];
        if (p[0] == '-')
        {
            for (i = 1; p[i]; i++)
            {
                switch (p[i])
                {
                case 'o':
                    c++;
                case 'r':
                case 's':
                default:
                    break;
                }
            }
        }
        else
        {
            load2arg(p);
        }
    }

    finishout();
    dumpsym();
    delarg = errlev;
    delexit();
}

/*
-s  `squash'  the  output,  that is, remove the symbol table
    and relocation bits to save space (but impair  the  use-
    fulness  of the debugger).  This information can also be
    removed by strip.

-u  take the following argument as a symbol and enter it  as
    undefined in the symbol table.  This is useful for load-
    ing wholly from a library, since  initially  the  symbol
    table  is empty and an unresolved reference is needed to
    force the loading of the first routine.

-l  This option is an abbreviation for a library  name.   -l
    alone  stands  for  `/lib/liba.a', which is the standard
    system library  for  assembly  language  programs.   -lx
    stands  for  `/lib/libx.a'  where x is any character.  A
    library is searched when its name is encountered, so the
    placement of a -l is significant.

-x  do not preserve local (non-.globl) symbols in the output
    symbol table; only enter external symbols.  This  option
    saves some space in the output file.

-X  Save  local  symbols  except for those whose names begin
    with `L'.  This option is used by cc to  discard  inter-
    nally  generated labels while retaining symbols local to
    routines.

-r  generate relocation bits in the output file so  that  it
    can  be  the  subject of another ld run.  This flag also
    prevents final definitions from being  given  to  common
    symbols,  and suppresses the `undefined symbol' diagnos-
    tics.

-d  force definition of common storage even if the  -r  flag
    is present.

-n  Arrange  that when the output file is executed, the text
    portion will be read-only and shared among all users ex-
    ecuting  the  file.  This involves moving the data areas
    up the the first possible 4K word boundary following the
    end of the text.

-i  When  the  output file is executed, the program text and
    data areas will live in separate  address  spaces.   The
    only  difference between this option and -n is that here
    the data starts at location 0.
*/