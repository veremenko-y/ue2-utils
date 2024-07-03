/*
**      print symbol tables for
**      object or archive files
**
**      nm [-goprun] [name ...]
*/

#include <stdio.h>
#include <stdlib.h>
#include <obj.h>
#include <ar.h>

#ifdef __GNUC__
#include <sys/types.h>
#endif

/* #define	SELECT	arch_flg ? arp.ar_name : *argv */
#define SELECT *argv

int numsort_flg;
int undef_flg;
int revsort_flg = 1;
int globl_flg;
int nosort_flg;
int arch_flg;
int prep_flg;
/* struct	ar_hdr	arp; */
struct header hdr;
FILE *fi;
off_t off;

main(argc, argv) char **argv;
{
    int narg;
    int compare();

    if (--argc > 0 && argv[1][0] == '-' && argv[1][1] != 0)
    {
        argv++;
        while (*++*argv)
            switch (**argv)
            {
            case 'n': /* sort numerically */
                numsort_flg++;
                continue;

            case 'g': /* globl symbols only */
                globl_flg++;
                continue;

            case 'u': /* undefined symbols only */
                undef_flg++;
                continue;

            case 'r': /* sort in reverse order */
                revsort_flg = -1;
                continue;

            case 'p': /* don't sort -- symbol table order */
                nosort_flg++;
                continue;

            case 'o': /* prepend a name to each line */
                prep_flg++;
                continue;

            default: /* oops */
                fprintf(stderr, "nm: invalid argument -%c\n", *argv[0]);
                exit(1);
            }
        argc--;
    }
    if (argc == 0)
    {
        argc = 1;
        argv[1] = "a.out";
    }
    narg = argc;
    while (argc--)
    {
        fi = fopen(*++argv, "r");
        if (fi == NULL)
        {
            fprintf(stderr, "nm: cannot open %s\n", *argv);
            continue;
        }
        fread((char *)&hdr, 1, sizeof(MAGIC), fi); /* get magic no. */
        if (hdr.magic == ARMAG)
            arch_flg++;
        else if (hdr.magic != MAGIC)
        {
            fprintf(stderr, "nm: %s-- bad format\n", *argv);
            continue;
        }
        fseek(fi, 0L, 0);
        if (arch_flg)
        {
            nextel(fi);
            if (narg > 1)
                printf("\n%s:\n", *argv);
        }
        do
        {
            off_t o;
            register i, n, c;
            char is_const;
            char is_conexport;
            struct sym *symp = NULL;
            struct sym sym;

            fread(&hdr, sizeof(struct header), 1, fi);
            if (hdr.magic != MAGIC) /* archive element not in  */
                continue;           /* proper format - skip it */
            o = hdr.textsize + hdr.datasize;
            if (hdr.hasrel)
            {
                o <<= 1;
            }
            fseek(fi, SYMOFFSET(hdr), SEEK_SET);
            n = hdr.symsize;
            if (n == 0)
            {
                fprintf(stderr, "nm: %s-- no name list\n", SELECT);
                continue;
            }
            i = 0;
            while (--n >= 0)
            {
                fread(&sym, 1, sizeof(sym), fi);
                if (globl_flg && (sym.type & SYMEXPORT) == 0)
                    continue;
                c = 0;
                switch (sym.type & SYMTYPE & ~(SYMCONST))
                {
                case SYMUNDEF:
                    c = 'u';
                    break;
                case SYMABS:
                    c = 'a';
                    break;
                case SYMREL:
                    c = 'r';
                    break;
                default:
                    c = '@';
                    break;
                }
                if (undef_flg && c != 'u')
                    continue;
                if ((sym.type & SYMEXPORT) )/* || (sym.type && SYMCOEXPORT)) */
                    c = toupper(c);
                is_const = sym.type & SYMCONST;
                is_conexport = sym.type & SYMCOEXPORT;
                sym.type = c;
                switch (sym.segm)
                {
                case SEGTEXT:
                    c = 't';
                    break;
                case SEGDATA:
                    c = 'd';
                    break;
                case SEGBSS:
                    c = 'b';
                    break;
                default:
                    c = '@';
                    break;
                }
                if(is_const)
                {
                    c = 'c';
                    sym.name[0] = '#';
                }
                if(is_conexport) {
                    c = toupper(c);
                }
                sym.segm = c;
                if (symp == NULL)
                    symp = (struct sym *)malloc(sizeof(struct sym));
                else
                {
                    symp = (struct sym *)realloc(symp, (i + 1) * sizeof(struct sym));
                }
                if (symp == NULL)
                {
                    fprintf(stderr, "nm: out of memory on %s\n", *argv);
                    exit(2);
                }
                symp[i++] = sym;
            }
            if (nosort_flg == 0)
                qsort(symp, i, sizeof(struct sym), compare);
            if ((arch_flg || narg > 1) && prep_flg == 0)
                printf("\n%s:\n", SELECT);
            for (n = 0; n < i; n++)
            {
                if (prep_flg)
                {
                    if (arch_flg)
                        printf("%s:", *argv);
                    printf("%s:", SELECT);
                }
                c = symp[n].type;
                if (!undef_flg)
                {
                    if ((c == 'u' || c == 'U') && symp[n].value == 0)
                        printf("      ");
                    else
                        printf("%.6x", symp[n].value);
                    printf(" %c %c ", c, symp[n].segm);
                    if(nosort_flg) {
                        printf("[$%04x] ", n);
                    }
                }
                printf("%.8s", symp[n].name);

                /* yaros: todo add flag
                printf(" [index: %d value: %d type: %d]", symp[n].index, symp[n].value, symp[n].type);
                */
                putchar('\n');
            }
            if (symp)
                free((char *)symp);
        } while (arch_flg && nextel(fi));
        fclose(fi);
    }
    exit(0);
}

compare(p1, p2) struct sym *p1, *p2;
{
    register i;

    if (numsort_flg)
    {
        if (p1->value > p2->value)
            return (revsort_flg);
        if (p1->value < p2->value)
            return (-revsort_flg);
    }
    for (i = 0; i < sizeof(p1->name); i++)
        if (p1->name[i] != p2->name[i])
        {
            if (p1->name[i] > p2->name[i])
                return (revsort_flg);
            else
                return (-revsort_flg);
        }
    return (0);
}

nextel(af)
    FILE *af;
{
    return 0;
#if 0
    register r;

    fseek(af, off, 0);
    r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af); /* read archive header */
    if (r <= 0)
        return (0);
    if (arp.ar_size & 1)
        ++arp.ar_size;
    off = ftell(af) + arp.ar_size; /* offset to next element */
    return (1);
#endif
}
