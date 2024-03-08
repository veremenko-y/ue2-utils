/* DEBUG */
/* Token types */
#ifndef DEBUGH
#define DEBUGH
FILE *debugfiles[8];
#endif

#define DSCAN 0
#define DLEX 1

#define DBGINIT()                                      \
	{                                                  \
		debugfiles[DSCAN] = fopen("dbgscan.tmp", "w"); \
		debugfiles[DLEX] = fopen("dbglex.tmp", "w");   \
	}

#define DBGCLOSE()                 \
	{                              \
		fclose(debugfiles[DSCAN]); \
		fclose(debugfiles[DLEX]);  \
	}

#define DBGTOK(tok) ""
/*(tok >= 0 ? DEBUGSTRINGS[tok] : DEBUGSTRINGS[tok + 256])*/

#define DBGSCAN(...)                             \
	{                                            \
		fprintf(debugfiles[DSCAN], __VA_ARGS__); \
	}

#define DBGLEX(...)                               \
	{                                             \
		fprintf(debugfiles[DLEX], __VA_ARGS__); \
	}

#define TRACE 0

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   /*WINDOWS*/
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          /**NIX*/
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

#define TRACEFUNC() {fprintf(stdout, "[%s] ", __FUNCTION_NAME__);}

#if TRACE >= 1
#define TRACE1(...) {TRACEFUNC(); fprintf(stdout, __VA_ARGS__);}
#else
#define TRACE1(...) 
#endif

#if TRACE >= 2
#define TRACE2(...) {TRACEFUNC(); fprintf(stdout, __VA_ARGS__);}
#else
#define TRACE2(...) 
#endif

#if TRACE >= 3
#define TRACE3(...) {TRACEFUNC(); fprintf(stdout, __VA_ARGS__);}
#else
#define TRACE3(...) 
#endif

void hexdump(data, off, size, width) char *data;
int off;
int size;
int width;
{
    int i;
    int pos = 0;
    int c;

    while (pos < size)
    {
        printf("%04x", pos + off);
        printf(" : ");
        for (i = 0; i < width; i++)
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
        for (i = 0; i < width; i++)
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

        pos += width;
        putchar('\n');
    }
}
