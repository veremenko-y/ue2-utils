#include <stdio.h>
#include "as.h"

static struct sym ops[] = {
    {"bz", STOKINS, MABS, 0 << 4},
    {"bl", STOKINS, MABS, 1 << 4},
    {"lda", STOKINS, MIMM, 2 << 4},
    {"ldl", STOKINS, MABS, 3 << 4},
    {"stl", STOKINS, MABS, 4 << 4},
    {"jsr", STOKINS, MABS, 5 << 4},
    {"strh", STOKINS, MABS, 6 << 4},
    {"strl", STOKINS, MABS, 7 << 4},
    {"rsr", STOKINS, MNONE, 8 << 4},
    {"scf", STOKINS, MIMM, 9 << 4},
    {"adc", STOKINS, MABS, 10 << 4},
    {"cmp", STOKINS, MABS, 11 << 4},
    {"ror", STOKINS, MABS, 12 << 4},
    {"nand", STOKINS, MABS, 13 << 4},
    {"ori", STOKINS, MABS, 14 << 4},
    {"ore", STOKINS, MABS, 15 << 4},
    {".byte", STOKBYTE, 0, 0},
    {".word", STOKWORD, 0, 0},
    {".code", STOKTEXT, 0, 0},
    {".data", STOKDATA, 0, 0},
    {".bss", STOKBSS, 0, 0},
    {".set", STOKSET, 0, 0},
    {".res", STOKRES, 0, 0},
    {".export", STOKEXPORT, 0, 0},
    {".import", STOKIMPORT, 0, 0},
};

struct sym *syms;
struct sym *cursym;
uint16_t symscnt;
uint16_t symstart;
uint16_t cursymn;
uint16_t symssize;
uint16_t symuser;
char strbuf[NAMESZ + 1];

syminit()
{
    symssize = 100;
    symscnt = symstart = sizeof(ops) / sizeof(ops[0]);
    symuser = symscnt;
    syms = malloc(symssize * sizeof(struct sym));
    memcpy(syms, ops, sizeof(ops));
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
    if (symscnt >= symssize)
    {
        error("oo syms");
    }
}

symdump()
{
#ifdef INFOEN
    uint16_t i;
    INFO("Symbols:\n");
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
            case SYMABS:
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
        ch = getchar();
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
    return getchar();
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
        case ';':
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
            if (is_alpha(c))
            {
                p = strbuf;
                *p++ = c;
                while (is_alpha_num(peek()))
                {
                    *p++ = advance();
                }
                *p = '\0';
                symfind();
                if (cursym->name[0] == 0)
                {
                    memcpy(cursym->name, strbuf, sizeof(cursym->name));
                    cursym->type = STOKID;
                }
                putc(TOKSYM, fout);
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
}
