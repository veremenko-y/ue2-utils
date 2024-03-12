#include <stdio.h>

hexdump(data, off, size) char *data;
int off;
int size;
{
    int i;
    int pos = 0;
    int c;

    while (pos < size)
    {
        printf("%04x", pos + off);
        printf(" : ");
        for (i = 0; i < 16; i++)
        {
            if ((pos + i) < size)
            {
                c = ((char *)data)[i + pos];
                c = c & 0xff;
                printf("%02x", c);
            }
            else
            {
                putchar(' ');
                putchar(' ');
            }
            putchar(' ');
        }

        printf(" : ");
        for (i = 0; i < 16; i++)
        {
            if ((pos + i) < size)
            {
                c = ((char *)data)[i + pos];
                if ((c >= 32) && (c <= 126))
                    putchar(c);
                else
                    putchar('.');
            }
            else
                putchar(' ');
        }

        pos += 16;
        putchar('\n');
    }
}

char buf[1024];

main(argc, argv) char **argv;
{
    FILE *f;
    int r;
    long skip = 0;
    int take = -1;
    int off;
    while (--argc > 0)
    {
        argv++;
        if ((*argv)[0] == '-')
        {
            switch ((*argv)[1])
            {
            case 's':
                argc--;
                argv++;
                skip = atoi(*argv);
                break;
            case 't':
                argc--;
                argv++;
                take = atoi(*argv);
                break;
            }
        }
        else
        {
            f = fopen(*argv, "r");
            if (f == NULL)
            {
                printf("file err\n");
                return;
            }
            fseek(f, skip, 0);
            off = skip;
            while ((r = fread(buf, 1, 1024, f)) > 0)
            {
                if (take == 0)
                {
                    break;
                }
                else if (take > 0)
                {
                    if (r > take)
                    {
                        r = take;
                        take = 0;
                    }
                    else
                    {
                        take -= r;
                    }
                }
                hexdump(buf, off, r);
                off += r;
            }
            fclose(f);
        }
    }
}
