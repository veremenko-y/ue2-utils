#include <stdio.h>
#include "as.h"

static struct sym ops[] = {
    {"bz", STOKINS, MIMM, 0},
    {"bl", STOKINS, MIMM, 1},
    {"lda", STOKINS, MABS, 2},
    {"ldl", STOKINS, MIMM, 3},
    {"stl", STOKINS, MIMM, 4},
    {"jsr", STOKINS, MIMM, 5},
    {"strh", STOKINS, MIMM, 6},
    {"strl", STOKINS, MIMM, 7},
    {"rsr", STOKINS, MNONE, 8},
    {"scf", STOKINS, MABS, 9},
    {"adc", STOKINS, MIMM, 10},
    {"cmp", STOKINS, MIMM, 11},
    {"ror", STOKINS, MIMM, 12},
    {"nand", STOKINS, MIMM, 13},
    {"ori", STOKINS, MIMM, 14},
    {"ore", STOKINS, MIMM, 15},
    {".align", STOKALIGN, 0, 0},
    {".byte", STOKBYTE, 0, 0},
    {".word", STOKWORD, 0, 0},
    {".code", STOKTEXT, 0, 0},
    {".data", STOKDATA, 0, 0},
    {".bss", STOKBSS, 0, 0},
    {".set", STOKSET, 0, 0},
    {".res", STOKRES, 0, 0},
    {".globl", STOKGLOBL, 0, 0},
    {".macro", STOKGLOBL, 0, 0},
    {".endm", STOKENDM, 0, 0},
    {".local", STOKLOCAL, 0, 0},
};

struct sym *syms;
struct sym *cursym;
uint16_t symscnt;
uint16_t symstart;
uint16_t cursymn;
uint16_t symssize;
char strbuf[NAMESZ + 1];

syminit()
{
    symssize = 100;
    symscnt = symstart = sizeof(ops) / sizeof(ops[0]);
    syms = calloc(symssize, sizeof(struct sym));
    memcpy(syms, ops, sizeof(ops));
}

symfindp(struct sym *p)
{
    cursymn = 0;
    cursym = syms;
    while (cursymn < symscnt)
    {
        if (cursym->name[0] == 0)
            break;
        if (cursym == p)
        {
            return cursymn;
        }
        cursym++;
        cursymn++;
    }
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
    if (cursym->name[0] == 0)
        symnew();
}

symnew()
{
    cursymn = symscnt;
    cursym = &syms[symscnt++];
    cursym->segm = 0;
    memset(cursym->name, 0, NAMESZ + 1);
    if (symscnt >= symssize)
    {
        error("oo syms");
    }
}

symdump()
{
#ifdef TRACEEN
    uint16_t i;
    INFO("Symbols:");
    for (i = 0; i < symscnt; i++)
    {
        INFO("%-10s[%x] ", syms[i].name, i);
        if (i < symstart)
        {
            INFO("BUILTIN ");
        }
        else
        {
            if (syms[i].segm == SEGTEXT)
            {
                INFO("[code] ");
            }
            else if (syms[i].segm == SEGDATA)
            {
                INFO("[DATA] ");
            }
            else if (syms[i].segm == SEGBSS)
            {
                INFO("[BSS] ");
            }
            else
            {
                INFO("[<invalid>] ");
            }
            switch (syms[i].type & ~(SYMEXPORT | SYMCONST))
            {
            case SYMUNDEF:
                INFO("UNDEF ");
                break;
            case SYMIMM:
                INFO("ABS ");
                break;
            case SYMREL:
                INFO("REL ");
                break;
            default:
                INFO("<invalid> ");
                break;
            }
            if (syms[i].type & SYMEXPORT)
            {
                INFO("EXPORT ");
            }
            if (syms[i].type & SYMCONST)
            {
                INFO("CONST ");
            }
        }
        INFO("= 0x%x\n", syms[i].value);
    }
#endif
}

int16_t ch = -2;

static peek()
{
    if (ch == -2)
    {
        ch = getc(fin);
    }
    return ch;
}

static advance()
{
    int16_t c;
    if (ch != -2)
    {
        c = ch;
        ch = -2;
        return c;
    }
    return getc(fin);
}

is_alpha(c)
{
    return c == '.' || c == '_' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

is_digit(c)
{
    return c >= '0' && c <= '9';
}

is_hex(c)
{
    return is_digit(c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F';
}

is_alpha_num(c)
{
    return is_alpha(c) || is_digit(c);
}

scan()
{
    int16_t t;
    int16_t c;
    char *p;
    struct sym *s;
    while ((c = advance()) >= 0)
    {
        switch (c)
        {
        case '\r':
        case ' ':
            continue;
        case '\n':
            putc(c, fout);
            break;
        case '/':
            if (peek() == '/')
                goto comment;
        case '#':
            if (peek() != ' ')
                goto notcomment;
        comment:
            while (peek() != '\n' && ch != -1)
            {
                advance();
            }
            break;
        case '"':
            putc(TOKSTR, fout);
            while (peek() != '"' && ch != -1)
            {
                putc(advance(), fout);
            }
            advance();
            putc('\0', fout);
            break;
        default:
        notcomment:
            if (is_alpha(c))
            {
                p = strbuf;
                *p++ = c;
                while (is_alpha_num(peek()))
                {
                    if (p < (strbuf + NAMESZ))
                    {
                        *p++ = advance();
                    }
                    else
                    {
                        advance();
                    }
                }
                *p = '\0';
                symfind();
                if (cursym->name[0] == 0)
                {
                    memcpy(cursym->name, strbuf, NAMESZ);
                    cursym->type = STOKID;
                }
                putc(TOKSYM, fout);
                putc(cursym->type, fout);
                fwrite(&cursymn, sizeof(cursymn), 1, fout);
            }
            else if (is_digit(c))
            {
                p = strbuf;
                *p++ = c;
                c = peek();
                t = 10;
                if (c == 'x')
                {
                    advance();
                    p = strbuf;
                    t = 16;
                    while (is_hex(peek()))
                    {
                        *p++ = advance();
                    }
                }
                else
                    while (is_digit(peek()))
                    {
                        *p++ = advance();
                    }
                *p = '\0';
                t = (int)strtol(strbuf, &p, t);
                putc(TOKINT, fout);
                fwrite(&t, sizeof(t), 1, fout);
            }
            else
            {
                putc(c, fout);
            }
            break;
        }
    }
    putc('\n', fout);
}
