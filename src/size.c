#include <stdio.h>
/*#include 	<a.out.h>*/
#include "obj.h"

/*
	size -- determine object size
*/

int a_magic[] = {A_MAGIC1, A_MAGIC2, A_MAGIC3, A_MAGIC4, 0};


print2(name, size)
uint8_t *name;
uint16_t size;
{
	printf("%s: %d (0x%04x)\n", name, size, size);
}

main(argc, argv) char **argv;
{
	struct hdr buf;
	int sum;
	int gorp, i;
	int symn;
	FILE *f;

	if (argc == 1)
	{
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	while (--argc)
	{
		++argv;
		if ((f = fopen(*argv, "r")) == NULL)
		{
			printf("size: %s not found\n", *argv);
			continue;
		}
		fread((char *)&buf, sizeof(buf), 1, f);
		for (i = 0; a_magic[i]; i++)
			if (a_magic[i] == buf.magic)
				break;
		if (a_magic[i] == 0)
		{
			printf("size: %s not an object file\n", *argv);
			fclose(f);
			continue;
		}
		if (gorp > 2)
			printf("\n%s: \n", *argv);
		puts("===segments===");
		printf("text: %u (0x%04x)\ndata: %u (0x%04x)\nbss: %u (0x%04x)\n",
			buf.tsize, buf.tsize,
			buf.dsize, buf.dsize,
			buf.bsize, buf.bsize);
		sum = buf.tsize + buf.dsize + buf.bsize;
		printf("total: %u (0x%04x)\n\n", sum);

		puts("===aux===");
		if (buf.trsize > 0 || buf.drsize > 0)
		{
			print2("header", sizeof(struct hdr));
			print2("rtext size", buf.trsize);
			print2("rdata size", buf.drsize);
			print2("sym size", buf.ssize);
		}

		printf("entry: 0x%04x\n", buf.entry);
		if (buf.ssize)
		{
			symn = buf.ssize / sizeof(struct symtab);
			printf("sym size: %u, 0x%04x (%u entries)\n",
				   buf.ssize, buf.ssize, symn);
		}

		sum = buf.tsize +
			  buf.dsize +
			  buf.bsize +
			  buf.ssize +
			  buf.trsize +
			  buf.drsize + sizeof(struct hdr);

		printf("\nfile size: %u (0x%04x)\n", sum, sum);

		printf("\n===offsets==\n");

		print2("text off", T_OFFSET(buf));
		print2("data off", D_OFFSET(buf));
		print2("rtext off", TREL_OFFSET(buf));
		print2("rdata off", DREL_OFFSET(buf));
		print2("sym off", SYM_OFFSET(buf));

		fseek(f, 0, SEEK_END);
		i = ftell(f);
		if (i != sum)
		{
			fprintf(stderr, "!!! file size mismatch !!!\n");
			fprintf(stderr, "expected %d got %d\n", sum, i);
		}
		fclose(f);
	}
}
