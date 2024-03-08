/*
**	print symbol tables for
**	object or archive files
**
**	nm [-goprun] [name ...]
*/



/*#include	<ar.h>
#include	<a.out.h>*/
#include "ar.h"
#include "obj.h"
#include	<stdio.h>
#include <stdlib.h>
#include	<ctype.h>
#define	MAGIC	exp.magic
#define	BADMAG	MAGIC!=A_MAGIC1 && MAGIC!=A_MAGIC2  \
		&& MAGIC!=A_MAGIC3 && MAGIC!=A_MAGIC4
#define	SELECT	arch_flg ? arp.ar_name : *argv
int	numsort_flg;
int	undef_flg;
int	revsort_flg = 1;
int	globl_flg;
int	nosort_flg;
int	arch_flg;
int	prep_flg;
struct	ar_hdr	arp;
struct	hdr	exp;
FILE	*fi;
long	off;
/*long	ftell();
char	*malloc();
char	*realloc();*/

main(argc, argv)
char **argv;
{
	int narg;
	int  compare();

	if (--argc>0 && argv[1][0]=='-' && argv[1][1]!=0) {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'n':		/* sort numerically */
			numsort_flg++;
			continue;

		case 'g':		/* globl symbols only */
			globl_flg++;
			continue;

		case 'u':		/* undefined symbols only */
			undef_flg++;
			continue;

		case 'r':		/* sort in reverse order */
			revsort_flg = -1;
			continue;

		case 'p':		/* don't sort -- symbol table order */
			nosort_flg++;
			continue;

		case 'o':		/* prepend a name to each line */
			prep_flg++;
			continue;

		default:		/* oops */
			fprintf(stderr, "nm: invalid argument -%c\n", *argv[0]);
			exit(1);
		}
		argc--;
	}
	if (argc == 0) {
		argc = 1;
		argv[1] = "a.out";
	}
	narg = argc;
	while(argc--) {
		fi = fopen(*++argv,"r");
		if (fi == NULL) {
			fprintf(stderr, "nm: cannot open %s\n", *argv);
			continue;
		}
		off = sizeof(exp.magic);
		fread((char *)&exp, 1, sizeof(MAGIC), fi);	/* get magic no. */
		if (MAGIC == ARMAG)
			arch_flg++;
		else if (BADMAG) {
			fprintf(stderr, "nm: %s-- bad format\n", *argv);
			continue;
		}
		fseek(fi, 0L, 0);
		if (arch_flg) {
			nextel(fi);
			if (narg > 1)
				printf("\n%s:\n", *argv);
		}
		do {
			long o;
			register i, n, c;
			struct symtab *symp = NULL;
			struct symtab sym;

			fread(&exp, sizeof(struct hdr), 1, fi);
			if (BADMAG)		/* archive element not in  */
				continue;	/* proper format - skip it */
			o = exp.tsize + exp.dsize;
			o += exp.trsize + exp.drsize;
			fseek(fi, o, SEEK_CUR);
			n = exp.ssize / sizeof(struct symtab);
			if (n == 0) {
				fprintf(stderr, "nm: %s-- no name list\n", SELECT);
				continue;
			}
			i = 0;
			while (--n >= 0) {
				fread((char *)&sym, 1, sizeof(sym), fi);
				if (globl_flg && (sym.type&XXTRN)==0)
					continue;
				switch (sym.type&XTYPE) {

				case XUNDEF:
					c = 'u';
					if (sym.value)
						c = 'c';
					break;

				default:
				case XABS:
					c = 'a';
					break;

				case XTEXT:
					c = 't';
					break;

				case XDATA:
					c = 'd';
					break;

				case XBSS:
					c = 'b';
					break;
				}
				if (undef_flg && c!='u')
					continue;
				if (sym.type&XXTRN)
					c = toupper(c);
				sym.type = c;
				if (symp==NULL)
					symp = (struct symtab *)malloc(sizeof(struct symtab));
				else {
					symp = (struct symtab *)realloc(symp, (i+1)*sizeof(struct symtab));
				}
				if (symp == NULL) {
					fprintf(stderr, "nm: out of memory on %s\n", *argv);
					exit(2);
				}
				symp[i++] = sym;
			}
			if (nosort_flg==0)
				qsort(symp, i, sizeof(struct symtab), compare);
			if ((arch_flg || narg>1) && prep_flg==0)
				printf("\n%s:\n", SELECT);
			for (n=0; n<i; n++) {
				if (prep_flg) {
					if (arch_flg)
						printf("%s:", *argv);
					printf("%s:", SELECT);
				}
				c = symp[n].type;
				if (!undef_flg) {
					if (c=='u' || c=='U')
						printf("      ");
					else
						printf("%.6x", symp[n].value);
					printf(" %c ", c);
				}
				printf("%.8s", symp[n].name);

				/* yaros: todo add flag 
				printf(" [index: %d value: %d type: %d]", symp[n].index, symp[n].value, symp[n].type);
				*/
				putchar('\n');
			}
			if (symp)
				free((char *)symp);
		} while(arch_flg && nextel(fi));
		fclose(fi);
	}
	exit(0);
}

compare(p1, p2)
struct symtab *p1, *p2;
{
	register i;

	if (numsort_flg) {
		if (p1->value > p2->value)
			return(revsort_flg);
		if (p1->value < p2->value)
			return(-revsort_flg);
	}
	for(i=0; i<sizeof(p1->name); i++)
		if (p1->name[i] != p2->name[i]) {
			if (p1->name[i] > p2->name[i])
				return(revsort_flg);
			else
				return(-revsort_flg);
		}
	return(0);
}

nextel(af)
FILE *af;
{
	register r;

	fseek(af, off, 0);
	r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af);  /* read archive header */
	if (r <= 0)
		return(0);
	if (arp.ar_size & 1)
		++arp.ar_size;
	off = ftell(af) + arp.ar_size;	/* offset to next element */
	return(1);
}
