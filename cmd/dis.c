#include <stdio.h>
#include <stdlib.h>
#include <obj.h>
#include <ar.h>

#ifdef __GNUC__
#include <sys/types.h>
#endif

struct header hdr;
FILE *fi;
FILE *fir;
off_t off;
uint16_t symscnt;
struct sym *syms;


symprint(int i)
{
    if (i >= symscnt)
    {
        printf("%d nout found", i);
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
        printf("CONST ");
    if (p->type & SYMEXPORT)
        printf("EXPORT ");
    printf("= 0x%x", p->value);
}

main(argc, argv) char **argv;
{
    int narg;

    if (argc == 0)
    {
        argc = 1;
        argv[1] = "a.out";
    }
    narg = argc;
    argc--;
    while (argc--)
    {
        fi = fopen(*++argv, "r");
        fir = fopen(*argv, "r");
        if (fi == NULL)
        {
            fprintf(stderr, "dis: cannot open %s\n", *argv);
            continue;
        }
        fread(&hdr, sizeof(MAGIC_OBJ), 1, fi); /* get magic no. */
        if (IS_MAGIC_VALID(hdr.magic) == 0)
        {
            fprintf(stderr, "dis: %s-- bad format\n", *argv);
            continue;
        }
        fseek(fi, 0L, 0);

        off_t o;
        register i, n, c;
        char is_const;
        struct sym sym;

        fread(&hdr, sizeof(struct header), 1, fi);
        if (IS_MAGIC_VALID(hdr.magic) == 0) /* archive element not in  */
            continue;           /* proper format - skip it */

        symscnt = n = hdr.symsize;
        syms = calloc(n, sizeof(struct sym));
        fseek(fi, SYMOFFSET(hdr), SEEK_SET);
        printf("read %d sym\n", n);
        fread(syms, sizeof(struct sym), n, fi);
        fseek(fi, CODEOFFSET(hdr), SEEK_SET);
        fseek(fir, RELCODEOFFSET(hdr), SEEK_SET);
        uint16_t ins;
        uint16_t arg;
        uint16_t rel;
        uint16_t addr = 0;
        int count = hdr.textsize;
        puts(".code");
        while (count > 0)
        {
            fread(&ins, 1, 2, fi);
            ins = ins >> 8 | ins << 8;
            arg = ins & 0x0fff;
            fread(&rel, 1, 2, fir);
            printf("; %04x: %04x ", (int)addr, (int)ins);
            if(rel & RELCONST)
            {
                printf("RELCONST ");
            }
            if(rel & RELSEG)
            {
                printf("RELSEG: ");
                if((rel & RELSEG) == RELTEXT)
                    printf("TEXT ");
                if((rel & RELSEG) == RELDATA)
                    printf("DATA ");
                if((rel & RELSEG) == RELBSS)
                    printf("BSS ");
            }
            printf("rel: %04x mask %04x value %04x\n", rel, (addr_t)~(RELTYPE), (addr_t)(rel & ~(RELTYPE)));
            rel = rel & ~(RELTYPE);
            if(rel != 0) {
                printf("%d ", rel); 
                symprint(rel-1);
            }
            printf("%04x\n\t\t");
            switch (ins >> 12)
            {
            case 0:
                printf("bz");
                break;
            case 1:
                printf("bl");
                break;
            case 2:
                printf("lda");
                break;
            case 3:
                printf("ldl");
                break;
            case 4:
                printf("stl");
                break;
            case 5:
                printf("jsr");
                break;
            case 6:
                printf("strh");
                break;
            case 7:
                printf("strl");
                break;
            case 8:
                printf("rsr");
                break;
            case 9:
                printf("scf");
                break;
            case 10:
                printf("adc");
                break;
            case 11:
                printf("cmp");
                break;
            case 12:
                printf("ror");
                break;
            case 13:
                printf("nand");
                break;
            case 14:
                printf("ori");
                break;
            case 15:
                printf("ore");
                break;
            }
            printf("\t");
            if (rel != 0)
            {
                printf("; ");
                if (rel & RELCONST)
                {
                    rel &= ~(RELCONST);
                }
                if (rel & RELSEG)
                {
                    int seg = ((rel & RELSEG) >> RELSEGSHIFT) -1;
                    for (i = 0; i < symscnt; i++)
                    {
                        if (syms[i].segm == seg && syms[i].value == arg)
                        {
                            printf("XXX");
                            symprint(i);
                            break;
                        }
                    }
                }
                else if(rel > 0)
                {
                    symprint(rel - 1);
                }
                puts("");
            }
            else
            {
                printf("\n");
            }
            addr += sizeof(ins);
            count -= sizeof(ins);
        }

        free((char *)syms);
        fclose(fi);
        fclose(fir);
    }
    exit(0);
}
