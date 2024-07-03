#include <stdio.h>
#include "as.h"

int16_t nexttok = -2;
int16_t curtok;

uint16_t segsize[4];
int curseg;


bswap(p)
uint16_t *p;
{
    *p = (*p >> 8) | (*p << 8);
}

static peek()
{
    if (nexttok == -2)
    {
        nexttok = getchar();
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
        curtok = getchar();
    }
#if LOG >= 3
    if (curtok >= 32)
        TRACE("advance: %c dec='%d'\n", curtok, curtok);
    else
    {
        TRACE("advance: dec=%d\n", curtok);
    }
#endif
    if (curtok == '\n')
    {
        lineno++;
    }
    return curtok;
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
        *dst++ = getchar();
    }
    TRACE("readn(x, %d) = %d\n", k, *dst);
}

static outins(word_t ins, word_t arg)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        ins = ins | arg;
        bswap(&ins);
        fwrite(&ins, sizeof(ins), 1, segout[curseg]);
        ins = 0;
        fwrite(&ins, sizeof(ins), 1, segout[curseg + RELOFFS]);
    }
    segsize[curseg] += sizeof(ins);
}

static outrel(word_t ins, word_t rel)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        bswap(&ins);
        fwrite(&ins, sizeof(ins), 1, segout[curseg]);
        fwrite(&rel, sizeof(rel), 1, segout[curseg + RELOFFS]);
    }
    segsize[curseg] += sizeof(ins);
}

static outb(word_t b)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        putc(b, segout[curseg]);
        b = 0;
        putc(b, segout[curseg + RELOFFS]);
    }
    segsize[curseg]++;
}

static parseins()
{
    /* segm in this case is representing instruction
     * addressing type */
    word_t ins;
    char is_const = 0;
    word_t idx;
    struct sym *p;
    word_t rel;
    uint8_t mtype = cursym->segm;
    switch (mtype)
    {
    case MNONE:
        outins(cursym->value, 0);
        break;
    case MABS:
    case MIMM:
        ins = cursym->value;
        advance();
        if (curtok == '#')
        {
            advance();
            is_const = 1;
            if (passno == 1)
            {
                if (curtok == TOKINT)
                {
                    symnew();
                    cursym->name[0] = '#';
                    cursym->segm = SEGDATA;
                    cursym->type = SYMCONST | SYMEXPORT;
                    segsize[SEGCONST]++;
                }
                else
                {
                    /* We only need it for size reporting */
                    segsize[SEGCONST]++;
                }
            }
        }
        if (curtok != TOKSYM && curtok != TOKINT)
            error("unexp");
        readn(&idx, sizeof(idx));
        if (is_const)
        {
            if (passno == 1)
            {
                if (curtok == TOKSYM)
                {
                    /*
                    p = &syms[idx];
                    cursym->segm = p->segm;
                    strcpy(cursym->name, p->name);

                    It's actually not relative, but absolute to a
                    const table
                    cursym->type |= SYMREL;

                    I guess we should define it REL if it is defined in current object
                    if ((p->type & SYMTYPE) != SYMUNDEF)
                    {
                        cursym->type |= SYMREL;
                        cursym->value = p->value;
                    }
                    else
                    {
                        //cursym->value = idx - symstart + 1;
                    }
                    */
                    /*
                    After long bashing, I realized that I need to simply reference the symbol
                    and instead add info to relocation indicating it's constant
                    */
                    /*
                    because linking sucks, I do need const symbol, just to have it exported and
                    resolved during the link process
                    */
                    p = &syms[idx];
                    sprintf(strbuf, "#%s", p->name);
                    symfind();
                    if(cursym->name[0] == '\0') {
                        memcpy(cursym->name, strbuf, NAMESZ + 1);
                        cursym->segm = p->segm;
                        cursym->type = SYMCONST;
                        cursym->value = idx - symstart; /* const referenced value */
                        if((p->type & SYMTYPE) != SYMUNDEF)
                        {
                            p->type |= SYMCOEXPORT;
                            /* cursym->type |= SYMREL; */
                        }
                    }
                    rel = cursymn - symstart + 1; /* const symbol */
                    rel |= RELCONST;
                }
                else
                {
                    cursym->type |= SYMABS;
                    sprintf(cursym->name, "#%x", idx);
                    cursym->value = idx;
                    rel = cursymn - symstart + 1;
                    /* rel |= RELCONST; */
                }
            }
            outrel(ins, rel);
        }
        else
        {
            if (curtok == TOKSYM)
            {
                p = &syms[idx];
                if (passno == 1 &&
                    p->segm == UINT8_MAX)
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
                if ((p->type & ~(SYMEXPORT)) == SYMABS)
                {
                    outins(ins, p->value);
                }
                else
                {
                    if ((p->type & SYMTYPE))
                    {
                        /* if defined, just indicate segment */
                        outrel(ins, (p->segm+1) << RELSEGSHIFT);
                    }
                    else
                    {
                        outrel(ins, idx - symstart + 1);
                    }
                }
            }
            else
            {
                outins(ins, idx);
            }
        }
    }
}

static parseset()
{
    expect(TOKSYM);
    readn(&curtok, sizeof(curtok));
    cursym = &syms[curtok];
    expect(',');
    expect(TOKINT);
    if (passno == 0 && cursym->type != SYMUNDEF)
    {
        error("redefined");
    }
    readn(&curtok, sizeof(curtok));
    TRACE(".set %s = %d\n", cursym->name, curtok);
    if (passno == 0)
    {
        cursym->type = SYMABS;
        cursym->value = curtok;
    }
}

static parseexport()
{
    expect(TOKSYM);
    readn(&curtok, sizeof(curtok));
    cursym = &syms[curtok];
    if (passno == 1)
    {
        cursym->segm = curseg;
        cursym->type |= SYMEXPORT;
    }
    TRACE(".export %s = %d\n", cursym->name, curtok);
}

static parseimport()
{
    expect(TOKSYM);
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
    word_t idx;
    advance();
    if (curtok != TOKINT && curtok != TOKSYM)
        error("unexp");
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
        fseek(segout[curseg + RELOFFS], idx, SEEK_CUR);
    }
}

parse()
{
    lineno = 1;
    curseg = 0;
    while (advance() >= 0)
    {
        switch (curtok)
        {
        case '\n':
            TRACE("TOK='\\n'\n");
            break;
        case TOKSYM:
            TRACE("TOKSYM\n");
            readn(&curtok, sizeof(curtok));
            cursym = &syms[curtok];
            switch (cursym->type)
            {
            case STOKID:
                TRACE("STOKID\n");
                expect(':');
                if (passno == 0)
                {
                    if (cursym->type != SYMUNDEF)
                        error("redefined");
                    cursym->segm = curseg;
                    cursym->type = SYMREL;
                    cursym->value = segsize[curseg];
                }
                break;
            case STOKTEXT:
                TRACE("STOKTEXT\n");
                curseg = SEGTEXT;
                break;
            case STOKDATA:
                TRACE("STOKDATA\n");
                curseg = SEGDATA;
                break;
            case STOKBSS:
                TRACE("STOKBSS\n");
                curseg = SEGBSS;
                break;
            case STOKINS:
                TRACE("STOKINS\n");
                parseins();
                break;
            case STOKSET:
                TRACE("STOKSET\n");
                parseset();
                break;
            case STOKEXPORT:
                TRACE("STOKEXPORT\n");
                parseexport();
                break;
            case STOKIMPORT:
                TRACE("STOKIMPORT\n");
                parseimport();
                break;
            case STOKRES:
                TRACE("STOKRES\n");
                parseres();
            case STOKBYTE:
                TRACE("STOKBYTE\n");
                advance();
                if (curtok == TOKINT)
                {
                    TRACE("TOKINT\n");
                    readn(&curtok, sizeof(curtok));
                    if (curtok > UINT8_MAX && curtok < INT8_MIN)
                        error("out of range");
                    outb(curtok);
                    break;
                }
                else if (curtok == TOKSTR)
                {
                    TRACE("TOKSTR\n");
                    while ((curtok = advance()) != 0)
                    {
                        outb(curtok);
                    }
                    break;
                }
                /* fallthrough */
            }
            break;
        default:

            error("unexpected token %d", curtok);

            break;
        }
    }
}