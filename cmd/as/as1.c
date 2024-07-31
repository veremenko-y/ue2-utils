#include <stdio.h>
#include <assert.h>
#include "as.h"

int16_t nexttok = -2;
int16_t curtok;

uint16_t segsize[6];
int curseg;
int last_globlbl;

bswap16(uint16_t *s)
{
    *s = *s >> 8 | *s << 8;
}

bswap24(uint8_t *s)
{
    *s = (uint32_t)(s[0] << 16 | s[1] << 8 | s[2]);
}

bswap32(uint8_t *s)
{
    *s = (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
}

#ifdef BITS12
#define bswapaddr bswap16
#define bswapir bswap16
#else
#define bswapaddr bswap16
#define bswapir bswap24
#endif


static peek()
{
    if (nexttok == -2)
    {
        nexttok = getc(fin);
    }
    return nexttok;
}

static advance()
{
    if (nexttok != -2)
    {
        curtok = nexttok;
        nexttok = -2;
    }
    else
    {
        curtok = getc(fin);
    }
#if 0
if(curtok >= 0)
{
    if (curtok > 1)
    {
        VINFO("%c", curtok);
        if(curtok == ':') {
            putchar(' ');
        }
    }
    else
    {
        addr_t idx;
        int tell = ftell(fin);
        VINFO("[%d]", tell);
        fread(&idx, sizeof(idx), 1, fin);
        fseek(fin, -sizeof(idx), SEEK_CUR);
        if(curtok == TOKINT) {
            VINFO(" %04x", idx);
        }else {
            VINFO(" %s", syms[idx].name);
        }
    }
}
#endif
    if (curtok == '\n')
    {
        lineno++;
        charno = 1;
    }
    else
    {
        charno++;
    }
    return curtok;
}

static err_unexp(tok)
{
    if (tok == TOKINT)
    {
        error("unexp int");
    }
    else if (tok == TOKSYM)
    {
        error("unexp symbol");
    }
    else
    {
        error("unexp '%c'", tok);
    }
}

static expect(c) char c;
{
    if (advance() != c)
    {
        if (c == TOKINT)
        {
            error("expected int");
        }
        else if (c == TOKSYM)
        {
            error("expected symbol");
        }
        else
        {
            error("expected '%c'", c);
        }
    }
}

static readn(char *dst, int16_t n)
{
    int k = n;
    if (nexttok != -2)
    {
        *dst++ = nexttok;
        nexttok = -2;
        n--;
    }
    while (n-- > 0)
    {
        *dst++ = getc(fin);
    }
    TRACE("readn(x, %d) = %d\n", k, *dst);
}

static outins(addr_t ins, addr_t arg)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        int i;
        char c;
        irword_t ir = (ins << IRSHIFT) | arg;
        bswapir(&ir);
        fwrite(&ir, IRBSIZE, 1, segout[curseg]);
    }
    segsize[curseg] += IRBSIZE;
    /* putc(0, segout[curseg + RELOFFS]);
    segsize[curseg + SIZERELOFF] += 1; */
}

static outrel(addr_t ins, addr_t arg, irword_t rel)
{
    addr_t addr = segsize[curseg];
    outins(ins,arg);
    if(passno == 1)
    {
        bswapaddr(&addr);
        fwrite(&addr, ADDRBSIZE, 1, segout[curseg + RELOFFS]);
        segsize[curseg + SIZERELOFF] += ADDRBSIZE;
        bswapir(&rel);
        fwrite(&rel, IRBSIZE, 1, segout[curseg + RELOFFS]);
        segsize[curseg + SIZERELOFF] += IRBSIZE;
    }
 /*    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
 int i;
        char c;
        irword_t ir = (ins << IRSHIFT) | arg;
        for(i = 0; i < IRSIZE; i++)
        {
            c = ((char*)&ir)[IRSIZE - i - IROFFSET];
            putc(c, segout[curseg]);
            c = ((char*)&rel)[IRSIZE - i - IROFFSET];
            putc(c, segout[curseg + RELOFFS]);
        }
    }
    segsize[curseg] += IRSIZE;
    segsize[curseg+SIZERELOFF] += IRSIZE; */
}

static outb(uint8_t b)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        putc(b, segout[curseg]);
        /* b = 0;
        putc(b, segout[curseg + RELOFFS]); */
    }
    segsize[curseg]++;
    /* segsize[curseg+SIZERELOFF]++; */
}

struct expr expr[EXPRSIZE];
int exprcnt;

static printexpr(struct expr *e)
{
    if (e == NULL || e->type == EXPNON)
    {
        return;
    }
    putchar('(');
    if (e->op)
    {
        putchar(e->op);
        putchar(' ');
    }
    if (e->type == EXPINT)
    {
        printf("i:%d", e->l.val);
    }
    else if (e->type == EXPSYM)
    {
        printf("s:%s", e->l.sym->name);
    }
    else if (e->type == EXPEXP)
    {
        printexpr(e->l.expr);
    }
    if (e->r != NULL)
    {
        putchar(' ');
        printexpr(e->r);
    }
    putchar(')');
}

static dumpexpr(struct expr *e)
{
    printexpr(e);
    putchar('\n');
}

static resetexpr()
{
    int i;
    for (i = 0; i < EXPRSIZE; i++)
    {
        expr[i].type = EXPNON;
    }
    exprcnt = 0;
}

static freeexpr(struct expr *e)
{
    int i;
    e->type = EXPNON;
    for (i = 0; i < EXPRSIZE; i++)
    {
        if (e == &expr[i])
        {
            exprcnt--;
        }
    }
}

static struct expr *allocexpr()
{
    struct expr *e;
    int i;
    if (exprcnt >= EXPRSIZE)
    {
        error("out of expr");
    }
    for (i = 0; i < EXPRSIZE; i++)
    {
        if (expr[i].type == EXPNON)
        {
            e = &expr[i];
            exprcnt++;
            break;
        }
    }
    e->type = EXPNON;
    e->op = 0;
    e->r = NULL;
    return e;
}

static parsexpr();

static exprprimary()
{
    addr_t val;
    struct expr *e;
    addr_t tok = advance();
    if (tok == TOKINT || tok == TOKSYM)
    {
        e = allocexpr();
        if (tok == TOKSYM)
        {
            getc(fin);
            readn(&val, sizeof(val));
            e->l.sym = &syms[val];
        }
        else
        {
            readn(&val, sizeof(val));
            e->l.val = val;
        }
        e->type = tok;
    }
    else if (tok == '(')
    {
        e = parsexpr();
        expect(')');
    }
    else
    {
        err_unexp(tok);
    }
    return e;
}

/* static join(uint8_t op, struct expr *l, struct expr *r)
{
    struct expr *e = allocexpr();
} */

static resolvevalue(struct expr *e);

static combine(uint8_t op, struct expr *l, struct expr *r)
{
    struct expr *e;
    addr_t val;
    switch (op)
    {
    case '+':
    case '-':
        l = resolvevalue(l);
        if (l->type == EXPINT && r->type == EXPINT)
        {
            e = l;
            e->l.val = op == '+'
                           ? l->l.val + r->l.val
                           : l->l.val - r->l.val;
            freeexpr(r);
            return e;
        }
        goto join;
        break;
    case '<':
    case '>':
        if (l->type == EXPINT)
        {
            val = l->l.val;
        }
        else if (l->type == EXPSYM && l->l.sym->type == SYMABS)
        {
            val = l->l.sym->value;
        }
        else
        {
            /* Hoping linker can figure it out */
            e = allocexpr();
            e->op = op;
            e->l.expr = l;
            e->type = EXPEXP;
            return e;
        }
        if (op == '<')
        {
            val &= 0xff;
        }
        else
        {
            val >>= 8;
        }
        l->l.val = val;
        l->type = EXPINT;
        l->op = 0;
        return l;
    default:
        error("reloc3");
        break;
    }
join:
    e = allocexpr();
    e->type = EXPEXP;
    e->op = op;
    e->l.expr = l;
    e->r = r;
    return e;
}

static exprunary()
{
    struct expr *l;
    int tok = peek();
    if (tok == '<' || tok == '>')
    {
        advance();
    }
    else
    {
        tok = 0;
    }
    l = exprprimary();
    if (tok)
    {
        l = combine(tok, l, (struct expr *)NULL);
    }
    return l;
}

static parsexpr()
{
    struct expr *l = exprunary();
    int op;
    while (peek() >= 0 && (nexttok == '+' || nexttok == '-'))
    {
        op = advance();
        l = combine(op, l, exprunary());
    }
    return l;
}

static resolvevalue(struct expr *e)
{
    if (e == NULL)
    {
        return NULL;
    }
    if (!e->op)
    {
        if (e->type == EXPSYM)
        {
            if (e->l.sym->type == SYMABS)
            {
                e->type = EXPINT;
                e->l.val = e->l.sym->value;
                return e;
            }
            else
            {
                return e;
            }
        }
        else if (e->type == EXPINT)
        {
            return e;
        }
    }
    else if (e->op == '<' || e->op == '>')
    {
        printexpr(e);
        error("why");
    }
    error("reloc1");
}

static resolveexpr(struct expr **out)
{
    struct expr *e;
    struct expr *l;
    struct expr *r;
    resetexpr();
    e = parsexpr();
    /* printexpr(e);
    puts(""); */
    /* Some expressions, like .set MUST resolve in pass 0 */
    if (!e->op)
    {
        l = resolvevalue(e);
        if (l->type == EXPINT)
        {
            *out = NULL;
            return l->l.val;
        }
        else if (l->type == EXPSYM)
        {
            *out = l;
            return 0;
        }
        else
        {
            printexpr(e);
            error("reloc4");
        }
    }
    else
    {
        l = resolvevalue(e->l.expr);
        r = resolvevalue(e->r);
        if (r && r->type == EXPSYM)
        {
            e = l;
            l = r;
            r = e;
        }
        if (r)
        {
            if (r->type == EXPSYM)
                error("reloc2");
        }
        *out = l;
        return r ? r->l.val : 0;
    }
}

static parseins()
{
    addr_t ins;
    char is_const = 0;
    addr_t idx;
    addr_t arg = 0;
    struct sym *p = NULL;
    irword_t rel;
    struct expr *e;
    /* segm in this case is representing instruction
     * addressing type */
    uint8_t mtype = cursym->segm;
    switch (mtype)
    {
    case MNONE:
        outins(cursym->value, 0);
        break;
    case MABS:
    case MIMM:
        ins = cursym->value;
        if (peek() == '#')
        {
            advance();
            is_const = 1;
        }
        idx = resolveexpr(&e);
        if (passno == 0)
        {
            outins(ins, 0);
            return;
        }

        if (e)
        {
            if (e->type == EXPINT)
            {
                idx = e->l.val;
            }
            else if (e->type == EXPSYM)
            {
                p = e->l.sym;
            }
            else if (e->type == EXPEXP)
            {
                if (e->op == '<' || e->op == '>')
                {
                    /* p = e->l.sym; */
                }
            }
        }
        if (is_const)
        {
            arg = idx;
            if (p != NULL)
            {
                /*
                because linking sucks, I do need const symbol, just to have it exported and
                resolved during the link process
                */
                sprintf(strbuf, "#%s", p->name);
                symfind();
                if (cursym->name[0] == '\0')
                {
                    segsize[SEGCONST]++; /* We only need it for size reporting */
                    memcpy(cursym->name, strbuf, NAMESZ + 1);
                    cursym->segm = p->segm;
                    cursym->type = SYMCONST;
                    cursym->value = SYMID(idx); /* const referenced value */
                    if ((p->type & SYMTYPE) != SYMUNDEF)
                    {
                        p->type |= SYMCOEXPORT;
                        /* cursym->type |= SYMREL; */
                    }
                }
                rel = SYMID(cursymn) + 1; /* const symbol */
                rel |= RELCONST;
            }
            else
            {
                symnew();
                cursym->name[0] = '#';
                cursym->type = SYMCONST | SYMEXPORT;
                segsize[SEGCONST]++;
                cursym->type |= SYMABS;
                sprintf(cursym->name, "#%x", idx);
                cursym->value = idx;
                /* Because this is not undefined symbol, just mark as const and emmit value to be
                looked up later */
                rel |= RELCONST;
            }
            outrel(ins, arg, rel);
        }
        else
        {
            if (p != NULL)
            {
                if (p->segm == UINT8_MAX)
                {
                    if (mtype == MIMM)
                    {
                        /* requires to be imported from the
                        correct segment*/
                        error("undefined");
                    }
                    else
                    {
                        /* absolute number, segment doesn't matter*/
                        p->segm = 0;
                    }
                }
                if ((p->type & SYMTYPE) == SYMABS)
                {
                    outins(ins, p->value);
                }
                else
                {
                    if ((p->type & SYMTYPE))
                    {
                        /* if defined, just indicate segment */
                        outrel(ins, p->value + idx, (p->segm + 1) << RELSEGSHIFT);
                    }
                    else
                    {
                        /* if not, relocate undefined external */
                        outrel(ins, idx, SYMID(symfindp(p)) + 1);
                    }
                }
            }
            else
            {
                /* Output absolute */
                outins(ins, idx);
            }
        }
    }
}

static parseset()
{
    addr_t idx;
    struct expr *e;

    expect(TOKSYM);
    getc(fin);
    readn(&curtok, sizeof(curtok));
    cursym = &syms[curtok];
    expect(',');
    if (passno == 0 && cursym->type != SYMUNDEF)
    {
        error("redefined");
    }
    idx = resolveexpr(&e);
    if (e != NULL)
    {
        printexpr(e);
        error("must be const");
    }
    TRACE(".set %s = %d\n", cursym->name, idx);
    if (passno == 0)
    {
        cursym->type = SYMABS;
        cursym->value = idx;
        cursym->segm = curseg;
    }
}

static parseglobl()
{
    do
    {
        expect(TOKSYM);
        getc(fin);
        readn(&curtok, sizeof(curtok));
        cursym = &syms[curtok];
        if (passno == 1)
        {
            cursym->segm = curseg;
            cursym->type |= SYMEXPORT;
        }
    } while (peek() == ',' && advance());
    TRACE(".export %s = %d\n", cursym->name, curtok);
}

static parseimport()
{
    expect(TOKSYM);
    getc(fin);
    readn(&curtok, sizeof(curtok));
    if (passno == 1)
    {
        cursym = &syms[curtok];
        cursym->segm = curseg;
    }
    TRACE(".export %s = %d\n", cursym->name, curtok);
}

static parseres()
{
    addr_t idx;
    advance();
    if (curtok != TOKINT && curtok != TOKSYM)
        err_unexp(curtok);
    readn(&idx, sizeof(idx));

    if (curtok == TOKSYM)
    {
        cursym = &syms[idx];
        if (cursym->type != SYMABS)
        {
            symdump();
            error("not abs");
        }
        idx = cursym->value;
    }
    segsize[curseg] += idx;

    if (passno == 0)
        return;

    if (curseg != SEGBSS)
    {
        fseek(segout[curseg], idx, SEEK_CUR);
        /* fseek(segout[curseg + RELOFFS], idx, SEEK_CUR); */
    }
}

uint8_t inmacro;

parse()
{
    lineno = 1;
    charno = 1;
    curseg = 0;
    int symtype;
    addr_t idx;
    struct expr *e;
    while (advance() >= 0)
    {
        switch (curtok)
        {
        case '\n':
            TRACE("TOK='\\n'");
            break;
        case ':': /* consume statement */
            break;
        case TOKSYM:
            TRACE("TOKSYM");
            symtype = getc(fin);
            readn(&idx, sizeof(idx));
            cursym = &syms[idx];
            switch (symtype)
            {
            case STOKID:
                INFO("STOKID");
                expect(':');
                if (cursym->name[0] != '.') /* not local label*/
                {
                    last_globlbl = idx;
                }
                if (passno == 0)
                {
                    if (cursym->type != SYMUNDEF)
                    {
                        /*
                        .local: / address 8
                        label: // address 10
                        .local: // address 12
                        if last global has address greater than local
                        it means we can redefine it
                        */
                        if ((syms[last_globlbl].segm == cursym->segm && syms[last_globlbl].value <= cursym->value) || cursym->name[0] != '.')
                        {
                            error("redefined");
                        }
                    }
                }
                cursym->segm = curseg;
                cursym->type = SYMREL;
                cursym->value = segsize[curseg];
                break;
            case STOKALIGN:
                if (curseg < SEGBSS && segsize[curseg] & 1)
                {
                    outb(0);
                }
            case STOKTEXT:
                TRACE("STOKTEXT");
                curseg = SEGTEXT;
                break;
            case STOKDATA:
                TRACE("STOKDATA");
                curseg = SEGDATA;
                break;
            case STOKBSS:
                TRACE("STOKBSS");
                curseg = SEGBSS;
                break;
            case STOKINS:
                TRACE("STOKINS");
                parseins();
                break;
            case STOKSET:
                TRACE("STOKSET");
                parseset();
                break;
            case STOKGLOBL:
                TRACE("STOKGLOBL");
                parseglobl();
                break;
            case STOKRES:
                TRACE("STOKRES");
                parseres();
                break;
            case STOKMACRO:
                if(inmacro) {
                    error("already in macro");
                }
                inmacro = 1;
                while(peek() == TOKSYM)
                {
                    
                }
                break;
            case STOKENDM:
                if(!inmacro) {
                    error("not in macro");
                }
                break;
            case STOKBYTE:
                TRACE("STOKBYTE");
                if (peek() == TOKSTR)
                {
                    advance();
                    TRACE("TOKSTR");
                    while ((curtok = advance()) != 0)
                    {
                        outb(curtok);
                    }
                    break;
                }
                else
                {
                    TRACE("TOKINT");
                    idx = resolveexpr(&e);
                    if (e != NULL)
                    {
                        error("not const");
                    }
                    if (idx > UINT8_MAX)
                        error("out of range");
                    outb(idx);
                    break;
                }
                /* fallthrough */
            }
            break;
        default:
            err_unexp(curtok);
            break;
        }
    }
}