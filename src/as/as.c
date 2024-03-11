#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "as.h"
#include "ue2.h"
#include "dbg.h"

#if TRACE >= 1
#define unlink(...)
#endif

struct hdr hdr = {
    0410,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

uint8_t oflg;
uint8_t ofilen[16] = "a.out";
uint16_t lineno;
#define TMPC 6
char tasfilen[] = "outsegXXX";

FILE *txtfil;
FILE *relfil;

uint8_t passno = 1;
struct symtab *hshtab[NHASH];
uint16_t hshused = 0;
uint16_t tsize;
uint16_t dsize;
uintptr_t tok;

/*struct symtab symtab[1000];*/
struct symtab *symtab;
struct symtab *symcur;
struct symtab *symend;
struct symtab *lastnam;
uint16_t nsyms; /*number in the symbol table*/
uint8_t strval[NCPS + 2];

uint8_t seg_number;
struct exp usedot[NLOC + NLOC];
FILE *usefile[NLOC + NLOC];
FILE *rusefile[NLOC + NLOC];
struct exp *dotp = &usedot[0]; /*current dot*/
struct exp args[6];

uintptr_t lexval;

uint8_t reflen[] = {0, 0, 1, 1, 2, 2, 4, 4, 8, 8}; /* length of relocation value */
uint16_t datbase = 0;                              /* base of the data segment */

error(char *format, ...)
{
    fprintf(stderr, "line %d: ", lineno);
    va_list argp;
    va_start(argp, format);
    vfprintf(stderr, format, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    exit(2);
}

expect(int val, char *arg)
{
    tok = lex();
    if (tok != val)
    {
        error("exp '%s' got %d\n", arg, tok);
    }
}

struct symtab *
symalloc()
{
    char *p;
    symcur++;
    if (symcur >= symend)
    {
        TRACE1("symalloc brk");
        symend = sbrk(200 * sizeof(struct symtab));
        if (symend == -1)
        {
            error("no mem");
            /* TODO: delete files */
        }
        symend = sbrk(0);
        p = symend;
        while (p > (char *)symcur)
        {
            *--p = 0;
        }
    }
    nsyms++;
    return (symcur - 1);
}

struct symtab **
lookup(install)
uint8_t install;
{
    uint16_t hash;
    uint16_t i;
    uint8_t *p1;
    uint8_t *p2;
    struct symtab **hp;
    for (hash = 0, p1 = strval; *p1; hash <<= 2, hash += *p1++)
        ;
    hash += p1[-1] << 5;
    hash %= NHASH;
    if (hash < 0)
        hash += NHASH;
    hp = &hshtab[hash];
    hash = 1; /*now, it counts the number of times we rehash*/
    while (*hp)
    {
        p1 = strval;
        p2 = (*hp)->name;
        for (i = 0; (i < NCPS) && *p1; i++)
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
        error("sym ovf");
    }

    if (install)
    {
        *hp = symalloc();
        for (p1 = strval, p2 = (*hp)->name; p1 < strval + NCPS;)
            *p2++ = *p1++;
        (*hp)->tag = 0;
    }
    return hp;
}

scan()
{
    int c;
    uintptr_t intval;
    char base;
    char *cp;
    struct symtab *op;

    int count = 0;
loop:
    tok = (type + 1)[c = getchar()];
    switch (tok)
    {
    case EOF:
        tok = TEOF;
        goto done;
    case TCOMM:
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        tok = TNL;
        lineno++;
        goto ret;
    case TNL:
        lineno++;
        goto ret;
    case TSPACE:
        goto loop;
    case TLSH:
        tok = (type + 1)[c = getchar()];
        if (tok != TLSH)
        {
            ungetc(c, stdin);
            tok = ILOB;
        }
        break;
    case TRSH:
        tok = (type + 1)[c = getchar()];
        if (tok != TLSH)
        {
            ungetc(c, stdin);
            tok = IHIB;
        }
    case TSQ:
        intval = getchar();
        tok = (type + 1)[c = getchar()];
        if (tok != TSQ)
        {
            error("exp '");
        }
        tok = TINT;
        goto ret;
    case TDQ:
        while ((tok = (type + 1)[c = getchar()]) != EOF && tok != TDQ)
        {
            if (tok == TNL)
            {
                error("unexp nl");
            }
            putc(TINT, txtfil);
            lexval = c;
            fwrite(&lexval, sizeof(lexval), 1, txtfil);
        }
        if (tok != TDQ)
        {
            error("exp \"");
        }
        goto loop;
    case TALPH:
        count++;
        tok = TNAME;
        cp = &strval[0];
        do
        {
            if (cp < &strval[NCPS])
            {
                *cp++ = c;
            }
        } while ((type + 1)[c = getchar()] == TALPH || (type + 1)[c] == TDIG);
        *cp++ = c;
        *cp = 0;
        op = *lookup(0);
        if (op != NULL)
        {
            goto ret;
        }
        ungetc(c, stdin);
        *--cp = 0;
        op = *lookup(1);
        if (op != NULL)
        {
            goto ret;
        }
        tok = TNAME;
        goto ret;
    case TDIG:
        intval = c - '0';
        if (c == '0')
        {
            c = getchar();
            if (c == 'x')
            {
                base = 16;
            }
            else if (c == 'b')
            {
                base = 2;
            }
            else
            {
                base = 8;
                ungetc(c, stdin);
            }
        }
        else
        {
            base = 10;
        }
        while ((type + 1)[c = getchar()] == TDIG || (base == 16 && (c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F')))
        {
            if (base == 8)
            {
                intval <<= 3;
            }
            else if (base == 10)
            {
                intval *= 10;
            }
            else if (base == 2)
            {
                intval <<= 1;
            }
            else
            {
                intval <<= 4;
            }
            if (c >= 'a' && c <= 'f')
                c -= 'a' - 10 - '0';
            else if (c >= 'A' && c <= 'F')
                c -= 'A' - 10 - '0';
            intval += c - '0';
        }
        ungetc(c, stdin);
        tok = TINT;
        goto ret;
    default:
        goto ret;
    }
ret:
    putc(tok, txtfil);
    if (tok == TNAME)
    {
        lexval = op;
        fwrite(&lexval, sizeof(lexval), 1, txtfil);
    }
    else if (tok == TINT)
    {
        lexval = intval;
        fwrite(&lexval, sizeof(lexval), 1, txtfil);
    }
    else
    {
        putc(c, txtfil);
    }
    if (tok != TEOF)
        goto loop;
done:
}

uintptr_t
lexpeek()
{
    int t = getchar();
    ungetc(t, stdin);
    return t;
}

lex()
{
    tok = getchar();
    if (tok == EOF)
    {
        tok = TEOF;
        return tok;
    }
    switch (tok)
    {
    case TNAME:
    case TINT:
        fread(&lexval, sizeof(lexval), 1, stdin);
        break;
    case TNL:
        lineno++;
        lexval = getchar();
        break;
    case TSEMI:
        lexval = getchar();
        tok = TNL;
        break;
    default:
        lexval = getchar();
        break;
    }
    return tok;
}

/*
 * Tables for combination of operands.
 */
/* clang-format off */
/* table for + */
uint8_t pltab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,
/*ABS*/	XUNDEF,	XABS,	XTEXT,	XDATA,	XBSS,	XXTRN,
/*TXT*/	XUNDEF,	XTEXT,	ERR,	ERR,	ERR,	ERR,
/*DAT*/	XUNDEF,	XDATA,	ERR,	ERR,	ERR,	ERR,
/*BSS*/	XUNDEF,	XBSS,	ERR,	ERR,	ERR,	ERR,
/*EXT*/	XUNDEF,	XXTRN,	ERR,	ERR,	ERR,	ERR,
};

/* table for - */
uint8_t mintab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,
/*ABS*/	XUNDEF,	XABS,	ERR,	ERR,	ERR,	ERR,
/*TXT*/	XUNDEF,	XTEXT,	XABS,	ERR,	ERR,	ERR,
/*DAT*/	XUNDEF,	XDATA,	ERR,	XABS,	ERR,	ERR,
/*BSS*/	XUNDEF,	XBSS,	ERR,	ERR,	XABS,	ERR,
/*EXT*/	XUNDEF,	XXTRN,	ERR,	ERR,	ERR,	ERR,
};

/* table for other operators */
uint8_t othtab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,	XUNDEF,
/*ABS*/	XUNDEF,	XABS,	ERR,	ERR,	ERR,	ERR,
/*TXT*/	XUNDEF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*DAT*/	XUNDEF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*BSS*/	XUNDEF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*EXT*/	XUNDEF,	ERR,	ERR,	ERR,	ERR,	ERR,
};
/* clang-format on */

struct exp *
combine(op, r1, r2)
register struct exp *r1, *r2;
{
    register t1, t2, type, loc;

    t1 = r1->xtype & XTYPE;
    t2 = r2->xtype & XTYPE;
    if (r1->xtype == XXTRN + XUNDEF)
        t1 = XTXRN;
    if (r2->xtype == XXTRN + XUNDEF)
        t2 = XTXRN;
    if (passno == 1)
        if (r1->xloc != r2->xloc && t1 == t2)
            t1 = t2 = XTXRN; /* error on != loc ctrs */
    t1 >>= 1;
    t2 >>= 1;
    switch (op)
    {

    case TPLUS:
        r1->xvalue += r2->xvalue;
        type = pltab[t1][t2];
        break;

    case TMINUS:
        r1->xvalue -= r2->xvalue;
        type = mintab[t1][t2];
        break;

        /*case TIOR:
            r1->xvalue |= r2->xvalue;
            goto comm;

        case TXOR:
            r1->xvalue ^= r2->xvalue;
            goto comm;

        case TAND:
            r1->xvalue &= r2->xvalue;
            goto comm;*/

    case TLSH:
        r1->xvalue <<= r2->xvalue;
        goto comm;
    case TRSH:
        r1->xvalue >>= r2->xvalue;
        goto comm;
        /*
        case TTILDE:
            r1->xvalue |= ~r2->xvalue;
            goto comm;*/

    case TMUL:
        r1->xvalue *= r2->xvalue;
        goto comm;

    case TDIV:
        if (r2->xvalue == 0)
            error("Divide check");
        else
            r1->xvalue /= r2->xvalue;
        goto comm;
    comm:
        type = othtab[t1][t2];
        break;
    default:
        error("Internal error: unknown operator");
    }
    if (t2 == (XTXRN >> 1))
        r1->xname = r2->xname;
    r1->xtype = type | (r1->xtype | r2->xtype) & (XFORW | XXTRN);
    if (type == ERR)
        error("Relocation error");
    return (r1);
}

parseex(e) struct exp *e;
{
    int peek;
    struct symtab *sp;
    struct exp right;

    tok = lex();
    e->xloc = 0;
    e->xvalue = lexval;

    if (tok == TLP)
    {
        parseex(e);
        expect(TRP, ')');
    }
    else if (tok == TINT)
    {
        e->xtype = XABS;
        e->xname = NULL;
    }
    else if (tok == TNAME)
    {
        sp = lexval;
        e->xtype = sp->type;
        e->xvalue = sp->value;
        /* write 0 into the segment
            index will be picked  from relocaiton table instead */
        e->xname = sp;
    }
    else
    {
        error("exp invalid");
    }
    if (tok == TNL)
    {
        return 0;
    }

    peek = lexpeek();
    if (peek == TMUL || peek == TDIV)
    {
        lex();
        parseex(&right);
        combine(peek, e, &right);
    }
    else if (peek == TLSH || peek == TRSH)
    {
        lex();
        parseex(&right);
        combine(peek, e, &right);
    }
    else if (peek == TPLUS || peek == TMINUS)
    {
        lex();
        parseex(&right);
        combine(peek, e, &right);
    }

    return 1;
}

#if 0
parseargs(def, len) struct modexpr *def;
{
    int i;
    int j;
    int arg;
    int argi;
    struct symtab *sp;
    i = j = argi = 0;
    tok = lex();
    while (i < len)
    {
        while (def[i].tokens[j] != 0)
        {
            arg = def[i].args & (1 << (7 - j));
            if (def[i].tokens[j] == IEXPR)
            {
                if (parseex(&args[argi]))
                {
                    argi++;
                }
                else
                {
                    goto next;
                }
            }
            else
            {
                sp = lexval;
                if (tok != def[i].tokens[j] && (def[i].tokens[j] != TMINUS || (tok != TPLUS && j != 1)) && (tok != TNAME || sp->tag != def[i].tokens[j]))
                {
                    goto next;
                }
                if (arg)
                {
                    sp = lexval;
                    args[argi].xloc = 0;
                    args[argi].yvalue = 0;
                    args[argi].xvalue = lexval;
                    if (tok == TINT)
                    {
                        args[argi].xtype = XABS;
                    }
                    else
                    {
                        args[argi].xtype = sp->type;
                        args[argi].xname = sp;
                    }
                    argi++;
                }
            }
            j++;
            /* So we don't pull too much past definition */
            if (def[i].tokens[j] != 0)
                tok = lex();
        }
        return &def[i];
    next:
        i++; /* try next one */
    }
    return NULL;
}
#endif

outb(val)
{
    dotp->xvalue++;
    if (passno == 2)
    {
        fwrite(&val, 1, 1, txtfil);
        TRACE2("OUTB %02x\n", (unsigned char)val);
    }
}

outw(val)
{
    char t;
    dotp->xvalue += 2;
    if (passno == 2)
    {
        t = val >> 8;
        fwrite(&t, 1, 1, txtfil);
        fwrite(&val, 1, 1, txtfil);
        TRACE2("OUTW %04x\n", val);
    }
}

mkfile(num, c) char c;
{
    tasfilen[TMPC] = 'a' + num;
    tasfilen[TMPC + 1] = c;
}

parse()
{
    int state = 0;
    int i, xtrab;
    int j;
    struct symtab *sym;
    struct instab *ins;
    struct modexpr *mod;
    int rel;
    while ((tok = lex()) != TEOF)
    {
    top:
        switch (tok)
        {
        case TEOF:
            return;
        case TNL:
        case TSEMI:
            state = 0;
            break;
        case TNAME:
            sym = (struct symtab *)lexval;
            switch (sym->tag)
            {
            case IDATA:
            case ITEXT:
            {
                seg_number = 0;
                if (sym->tag == IDATA)
                {
                    seg_number = seg_number + NLOC;
                }
                TRACE2("Switching Segment IDATA=%d TOK %d\n", sym->tag == IDATA, sym->tag);
                dotp = &usedot[seg_number];
                if (usefile[seg_number] == NULL)
                {
                    mkfile(seg_number, 't');
                    if ((usefile[seg_number] = fopen(tasfilen, "wb")) == NULL)
                    {
                        error("no temp %s", tasfilen);
                    }
                    mkfile(seg_number, 'R');
                    if ((rusefile[seg_number] = fopen(tasfilen, "wb")) == NULL)
                    {
                        error("no temp %s", tasfilen);
                    }
                }
                TRACE2("SEG NUM %d\n", seg_number);
                txtfil = usefile[seg_number];
                relfil = rusefile[seg_number];
                break;
            }
            case ISET:
            { /*.set name,expr*/
                tok = lex();
                if (tok != TNAME)
                {
                    error("exp name");
                }
                sym = (struct symtab *)lexval;
                sym->type = XABS;
                expect(TCOMMA, ",");
                tok = lex();
                if (tok != TINT)
                {
                    error("exp value");
                }
                sym->value = lexval;
                break;
            }
            case IGLOBAL:
            { /*.globl <name> */
                tok = lex();
                if (tok != TNAME)
                {
                    error("exp name");
                }
                sym = (struct symtab *)lexval;
                sym->type |= XXTRN;
                break;
            }
            case IWORD:
            case IBYTE:
            { /* .byte val[, val] | .word val[, val]*/
                state = tok;
                while ((tok = lex()) != TNL)
                {
                    if (tok == TCOMMA)
                    {
                        continue; /* skip comma */
                    }
                    i = lexval;
                    if (tok != TINT)
                    {
                        sym = (struct instab *)lexval;
                        i = sym->value;
                    }
                    TRACE2("DATA %d tok=%d val=%d\n", state == IWORD ? 2 : 1, tok, i);

                    if (tok == TINT)
                    {
                        if (state == IWORD)
                        {
                            outw(i);
                        }
                        else
                        {
                            outb(i);
                        }
                    }
                    else
                    {
                        error("unexp");
                    }
                }
                goto top;
                break;
            }
            case TINTSTR:
                ins = (struct instab *)lexval;
                TRACE2("0x%x: INSTR %s\n", dotp->xvalue, ins->name);
                state = 0;
                xtrab = 0;
                switch (ins->access)
                {
                case MNONE:
                    if (passno == 1)
                    {
                        dotp->xvalue += 2;
                    }
                    else
                    {
                        outb(ins->opcode << 4);
                        outb(0);
                    }
                    break;
                case MIMM:
                    if (passno == 1)
                    {
                        if (!parseex(&args[0]))
                        {
                            error("unexp arg");
                        }
                        dotp->xvalue += 2;
                    }
                    else
                    {
                        if (!parseex(&args[0]))
                        {
                            error("unexp arg");
                        }
                        xtrab = LEN1;
                        rel = args[0].xvalue;
                        outb(ins->opcode << 4);
                        if (args[0].xtype == XABS)
                        {
                            /* yaros: todo: add range check [0,255]*/
                            TRACE1("0x%04x: ABS %d\n", dotp->xvalue, rel);
                        }
                        else
                        {
                            sym = args[0].xname;
                            TRACE1("0x%04x: IMMREL %s=%d %d\n", dotp->xvalue, sym->name, sym->value, sym->type);
                            /*sym = args[0].xname; unused, debug*/
                        }
                        outrel(&args[0].xvalue, LEN1, args[0].xtype, args[0].xname);
                        /*
                        no relative things
                        if (rel > int_max || rel < -127)
                        {
                            error("out of range");
                        }*/
                    }
                    break;
                case MABS:
                    if (passno == 1)
                    {
                        dotp->xvalue += 2;
                        if (!parseex(&args[0]))
                        {
                            error("unexp arg");
                        }
                    }
                    else
                    {
                        if (!parseex(&args[0]))
                        {
                            error("unexp arg");
                        }
                        xtrab = LEN2;
                        rel = args[0].xvalue;
                        sym = args[0].xname;
                        TRACE1("rel on line %d = %d\n", lineno, rel);
                        TRACE1("0x%04x: ABSREL %s=%d %d\n", dotp->xvalue, sym->name, sym->value, sym->type);
                        /*if (rel > 128 || rel < -127)
                        {
                            error("out of range");
                        }*/
                        /*outb(ins->opcode);
                        outb(rel);*/
                        i = (args[0].xvalue & 0xfff) | (ins->opcode << 12);
                        outrel(&i, LEN2, args[0].xtype, args[0].xname);

                        /*fwrite(&ins->opcode, sizeof(ins->opcode), 1, txtfil);
                        fwrite(&rel, 1, 1, txtfil);*/
                    }
                    break;
                default:
                    error("unexpented");
                    break;
                }
                /*while ((tok = lex()) != TNL)
                {
                    if (tok == TNAME)
                    {
                        ins = (struct instab *)lexval;
                    }
                    else if (tok == TCOMMA)
                    {
                    }
                    else if (tok == TINT)
                    {
                    }
                }*/
                break;
            default:
                expect(TCOLON, ":");
                if (passno == 1)
                {
                    if ((sym->type & XTYPE) != XUNDEF)
                    {
                        TRACE1("type %d value %d\n", sym->type, sym->value);
                        error("%s redef", sym->name);
                    }
                    sym->value = dotp->xvalue;
                    sym->index = dotp - usedot;
                    sym->type = dotp->xtype;
                }
                else
                {
                    if (sym->type & XTYPE == XUNDEF)
                    {
                        sym->type |= XXTRN;
                    }
                }
                sym->type |= dotp->xtype;
                TRACE2("0x%x: LBL %s=%d %d (index: %d)\n", dotp->xvalue, sym->name, sym->value, sym->type, sym->index);
                break;
            }
            expect(TNL, "\\n");
            break;
        default:
            error("unexp tok");
        }
    }
}

/*
 *	Save the relocation information
 */
outrel(pval, reftype, reltype, xsym) int *pval;
int reftype, reltype;
struct symtab *xsym;
{
    /*
     *	reftype: PCREL or not, plus length LEN1, LEN2, LEN4, LEN8
     *	reltype: csect ("segment") number (XTEXT, XDATA, ...) associated with 'val'
     * 	xsym: symbol table pointer
     */
    uint16_t ts;
    char tc;
    uint16_t tl;
    short t;
    if (passno != 2)
    {

        dotp->xvalue += reflen[reftype];
        return;
    }
    /*if (bitoff&07)
        error("Padding error");*/
    reltype &= ~XFORW;
    if (reltype == XUNDEF)
        error("undef");
    if (reltype != XABS || reftype & PCREL)
    {
        /* write the address portion of a relocation datum */
        if (dotp >= &usedot[NLOC])
        {
            hdr.drsize += sizeof(dotp->xvalue) + sizeof(tl) + sizeof(tc);
            tl = dotp->xvalue - datbase;
        }
        else
        {
            hdr.trsize += sizeof(dotp->xvalue) + sizeof(tl) + sizeof(tc);
            tl = dotp->xvalue;
        }
        fwrite(&tl, sizeof(tl), 1, relfil);
        /* write the properties portion of a relocation datum */
        if (reltype == XXTRN + XUNDEF)
        {
            ts = (xsym->index);
            tc = (XXTRN << 3) | (reftype - LEN1);
        }
        else if ((reltype & XTYPE) == XUNDEFO)
        {
            ts = (xsym->index);
            tc = ((XXTRN + 2) << 3) | (reftype - LEN1);
        }
        else
        {
            ts = (reltype);
            tc = (reftype - LEN1);
        }
        fwrite(&ts, sizeof(ts), 1, relfil);
        fwrite(&tc, sizeof(tc), 1, relfil);
    }
    /* write the raw ("unrelocated") value to the text file */
    t = reflen[reftype];
    dotp->xvalue += t;
    if (reftype & PCREL)
        *pval -= dotp->xvalue;

    if (t > 1)
    {
        reltype = *pval >> 8;
        fwrite(&reltype, 1, 1, txtfil);
        fwrite(pval, 1, 1, txtfil);
    }
    else
    {
        fwrite(pval, 1, t, txtfil);
    }
#if TRACE >= 1
    { /* DEBUG*/
        char str[NCPS + 2];
        str[0] = 0;
        if (xsym->name != NULL)
        {
            strcpy(str, xsym->name);
        }
        TRACE1("OUTREL (%s) addr=0x%06x[%d] type=0x%x len=0x%x value=0x%x\n", str, tl, t, ts, tc, *pval);
    }
#endif
}

int sizesymtab()
{
    struct symtab *sp;

/*#define NOUTSYMS (nsyms - nforgotten - (savelabels ? 0 : nlabels))*/
#define NOUTSYMS nsyms

    return (
        (
            /*   NCPS
            + sizeof (sp->ptype)
            + sizeof (sp->other)
            + sizeof (sp->desc)
            + sizeof (sp->value)*/
            sizeof(symtab[0])) *
        NOUTSYMS);
}

symwrite(p, n, f) struct symtab *p;
FILE *f;
{
    while (n--)
    {
        fwrite(p, sizeof(symtab[0]), 1, f);
        p++;
    }
}

#if TRACE >= 1
dumpsymone(sym) struct symtab *sym;
{
    char buf[1024];
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
    TRACE1(buf);
}

dumpsym()
{
    int i;
    TRACE1("symtab size %d\n", hshused);
    lastnam = symtab;
    while (lastnam < symcur)
    {
        dumpsymone(lastnam);
        lastnam++;
    }
    /*
        TRACE1("hashtab size: %d\n", NHASH);
        for(i =0 ; i < NHASH; i++) {
            if(hshtab[i] != NULL) {
                lastnam = hshtab[i];
                TRACE1("hash %d\n", i);
                dumpsymone(lastnam);
            }
        }
    */
}
#else
dumpsym() {}
dumpsymone(sym) struct symtab *sym;
{
}
#endif

main(argc, argv) int argc;
char **argv;
{
    struct instab *ip;
    struct symtab **hp;
    int i;
    argc--;
    argv++;

    while (argc > 1 && **argv == '-')
    {
        switch ((*argv)[1])
        {
        case '\0':
            break;
        case 'o':
            oflg = 1;
            if (argc < 2)
            {
                error("-o what?");
            }
            argc--;
            argv++;
            strcpy(ofilen, *argv);
            TRACE2("==Output File: %s\n", ofilen);
            break;
        }
        argv++;
        argc--;
    }
    if (argc < 1)
    {
        error("no file");
    }

    /* Init hash table
        commented out because we rely
        on BSS initializing it
    for (i = 0; i < NHASH; i++)
    {
        hshtab[i] = 0;
    }
    */

    /* TODO: dynamic alloc */
    /*
    symcur = symtab;
    symend = &symtab[sizeof(symtab)];
    */

    symcur = symtab = sbrk(200 * sizeof(struct symtab));
    symend = sbrk(0);
    nsyms = 0;

    /*
     * Install symbol table
     */
    for (ip = instab; ip->name[0] != 0; ip++)
    {
        char *p1, *p2;
        for (p1 = ip->name, p2 = strval; p2 < strval + NCPS;)
            *p2++ = *p1++;
        *p2++ = 0;
        hp = lookup(0); /* do not install */
        if (*hp == NULL)
        {
            *hp = (struct symtab *)ip;
        }
    }
    seg_number = 0;
    for (i = 0; i < NLOC; i++)
    {
        usedot[i].xtype = XTEXT;
        usedot[i + NLOC].xtype = XDATA;
    }

    /*
     * PASS 0
     */
    puts("pass 0");
    /* temp file for scan result */
    mkfile(0, 's');
    txtfil = fopen(tasfilen, "wb");

    /* this workflow doesn't really work */
    while (argc > 0)
    {
        if (freopen(*argv, "r", stdin) == NULL)
        {
            error("can't open %s\n");
        }
        lineno = 1;
        TRACE1("scanning %s\n", *argv);
        puts(*argv);
        scan();
        i = ftell(txtfil);
        TRACE1("last byte written %d (%04x)\n", i, i);
        argv++;
        argc--;
    }
    fclose(txtfil);

    /*
     * PASS 1
     */
    puts("pass 1");
    lineno = 1;
    passno = 1;
    freopen(tasfilen, "rb", stdin);

    /* init usefile and rusefile arrays */
    for (i = 0; i < NLOC * 2; i++)
    {
        usefile[i] = NULL;
        rusefile[i] = NULL;
    }

    /* default temp files for .text */
    mkfile(0, 't');
    if ((usefile[0] = fopen(tasfilen, "wb")) == NULL)
    {
        error("no temp %s", tasfilen);
    }
    mkfile(0, 'R');
    if ((rusefile[0] = fopen(tasfilen, "wb")) == NULL)
    {
        error("no temp %s", tasfilen);
    }
    txtfil = usefile[0];
    relfil = rusefile[0];

    TRACE1("==============parsing pass 1\n");
    parse();

    /*
     *	~~round and~~ assign text segment origins
     *  yaros: we do not do rounding, as we are using byte addresses
     */
    tsize = 0;
    for (i = 0; i < NLOC; i++)
    {
        tok = usedot[i].xvalue;
        usedot[i].xvalue = tsize;
        tsize += tok;
    }
    /*
     *	round and assign data segment origins
     */
    /* yaros: we do not need pages here */
    /*datbase = round(tsize, PAGRND);*/
    dsize = 0;
    datbase = tsize;
    TRACE1("text size: %d data size: %d data base: %d\n", tsize, dsize, datbase);
    for (i = 0; i < NLOC; i++)
    {
        tok = usedot[NLOC + i].xvalue;
        usedot[NLOC + i].xvalue = datbase + dsize;
        dsize += tok;
    }

    hdr.bsize = dsize;
    /* assign final values to symbols */
    i = 0;
    for (lastnam = symtab; lastnam < symcur; lastnam++)
    {
        /*
        * We could reenable this code if we want all undefined
        * to automatically become external
        if ((lastnam->type&XTYPE)==XUNDEF)
            lastnam->type = XXTRN+XUNDEF;
        else
        */
        if ((lastnam->type & XTYPE) == XDATA)
            lastnam->value += usedot[lastnam->index].xvalue;
        else if ((lastnam->type & XTYPE) == XTEXT)
            lastnam->value += usedot[lastnam->index].xvalue;
        else if ((lastnam->type & XTYPE) == XBSS)
        {
            long bs;
            bs = lastnam->value;
            lastnam->value = hdr.bsize + datbase;
            hdr.bsize += bs;
        }
        lastnam->index = i++;
    }
    hdr.bsize -= dsize;
    hdr.dsize = dsize;
    hdr.tsize = tsize;
    hdr.ssize = sizesymtab();
    dumpsym();
    puts("pass 2");
    /*
     PASS 2
    */
    close(stdin);
    dotp = &usedot[0];
    txtfil = usefile[0];
    relfil = rusefile[0];
    lineno = 1;
    tsize = 0;
    dsize = 0;
    passno = 2;

    mkfile(0, 's');
    freopen(tasfilen, "rb", stdin);
    TRACE1("==============parsing pass 2\n");
    parse();
    fclose(stdin);
    /* delete scan file */
    unlink(tasfilen);

    /* close all out files */
    for (i = 0; i < NLOC * 2; i++)
    {
        if (usefile[i] != NULL)
        {
            fclose(usefile[i]);
        }
        if (rusefile[i] != NULL)
        {
            fclose(rusefile[i]);
        }
    }
    txtfil = fopen(ofilen, "w");
    fwrite(&hdr, sizeof(hdr), 1, txtfil);
    /*
     *	append csect text onto text for csect 0
     */
    for (i = 0; i < NLOC + NLOC; i++)
    {
        char buffer[BUFSIZ];
        if (usefile[i])
        {
            mkfile(i, 't');
            relfil = fopen(tasfilen, "r");
            if (relfil == NULL)
            {
                error("unexp");
                continue;
            }
            while (!feof(relfil))
                fwrite(buffer, 1,
                       fread(buffer, 1, BUFSIZ, relfil),
                       txtfil);
            fclose(relfil);
            unlink(tasfilen);
        }
    }
    /*
     *	append relocation info onto text
     */
    for (i = 0; i < NLOC + NLOC; i++)
    {
        char buffer[BUFSIZ];
        if (rusefile[i])
        {
            mkfile(i, 'R');
            relfil = fopen(tasfilen, "r");
            if (relfil == NULL)
            {
                error("cannot reopen temp");
                continue;
            }
            while (!feof(relfil))
                fwrite(buffer, 1,
                       fread(buffer, 1, BUFSIZ, relfil),
                       txtfil);
            fclose(relfil);
            unlink(tasfilen);
        }
    }

    for (lastnam = symtab; lastnam < symcur; lastnam++)
    {
        lastnam->type &= ~XFORW;
        lastnam->index = 0;
    }
    dumpsym();
    symwrite(symtab, symcur - symtab, txtfil);

    /*
     *	Go back and patch up rsize
     */
    fseek(txtfil, 0L, 0);
    fwrite(&hdr, sizeof(hdr), 1, txtfil);
    fseek(txtfil, 0, SEEK_END);
    fclose(txtfil);

    TRACE1("==============\n");

    return 0;
}
