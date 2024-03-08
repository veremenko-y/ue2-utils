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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define MAGIC exp.magic
#define BADMAG MAGIC != A_MAGIC1 &&MAGIC != A_MAGIC2 &&MAGIC != A_MAGIC3 &&MAGIC != A_MAGIC4
#define SELECT arch_flg ? arp.ar_name : *argv
int arch_flg;
struct ar_hdr arp;
struct hdr exp;
FILE *fi;
long off;
/*long	ftell();
char	*malloc();
char	*realloc();*/

int reflen[] = {0, 0, 1, 1, 2, 2, 4, 4, 8, 8}; /* length of relocation value */

main(argc, argv) char **argv;
{
	int narg;
	argc--;
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
			fprintf(stderr, "rel: cannot open %s\n", *argv);
			continue;
		}
		off = sizeof(exp.magic);
		fread((char *)&exp, 1, sizeof(MAGIC), fi); /* get magic no. */
		if (MAGIC == ARMAG)
			arch_flg++;
		else if (BADMAG)
		{
			fprintf(stderr, "rel: %s-- bad format\n", *argv);
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
			int o;
			int i, n, c;
			struct symtab *symp = NULL;
			struct symtab sym;
			uint16_t raddr, rts;
			uint8_t rtc;

			fread(&exp, sizeof(struct hdr), 1, fi);
			if (BADMAG)	  /* archive element not in  */
				continue; /* proper format - skip it */
			o = exp.tsize + exp.dsize;
			fseek(fi, o, SEEK_CUR);
			i = exp.trsize;
			while (i > 0)
			{
				fread(&raddr, sizeof(raddr), 1, fi);
				fread(&rts, sizeof(rts), 1, fi);
				fread(&rtc, sizeof(rtc), 1, fi);
				printf("0x%06x: sym=%x type=%x\n", raddr, rts, rtc);

				i -= sizeof(raddr) + 3 + sizeof(rtc);

				n = reflen[(rtc & 0x7) + 2];
				c = 0;
				/*fread(&c, n, 1, fi);*/
			}

		} while (arch_flg && nextel(fi));
		fclose(fi);
	}
	exit(0);
}


nextel(af)
	FILE *af;
{
	register r;

	fseek(af, off, 0);
	r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af); /* read archive header */
	if (r <= 0)
		return (0);
	if (arp.ar_size & 1)
		++arp.ar_size;
	off = ftell(af) + arp.ar_size; /* offset to next element */
	return (1);
}
