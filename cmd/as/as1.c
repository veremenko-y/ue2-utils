#include <stdio.h>
#include "as.h"

int16_t nexttok = -2;
int16_t curtok;

uint16_t segsize[3];
int curseg;

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
    INFO("readn(x, %d) = %d\n", k, *dst);
}

static outins(word_t ins, word_t arg)
{
    if (curseg >= SEGBSS)
        error("bss out");
    if (passno == 1)
    {
        ins = ins | arg;
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
    switch (cursym->segm)
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
                symnew();
                cursym->name[0] = '#';
                cursym->type = SYMCONST;
            }
        }
        if (curtok != TOKSYM && curtok != TOKINT)
            error("unexp");
        readn(&idx, sizeof(idx));
        p = &syms[idx];
        if (is_const)
        {
            if (passno == 1)
            {
                if (curtok == TOKSYM)
                {
                    cursym->segm = p->segm;
                    cursym->type |= SYMREL;
                    cursym->value = 0; /* add expression calculation here */
                }
                else
                {
                    cursym->type |= SYMABS;
                    cursym->value = idx;
                }
            }
            outrel(ins, cursymn);
        }
        else
        {
            if (curtok == TOKSYM)
            {
                if (p->type & ~(SYMEXPORT) == SYMABS)
                {
                    outins(ins, p->value);
                }
                else
                {
                    outrel(ins, idx);
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
    INFO(".set %s = %d\n", cursym->name, curtok);
    if (passno == 0)
    {
        cursym->type = SYMABS;
        cursym->value = curtok;
    }
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

            error("unexpected token");
            break;
        }
    }
}