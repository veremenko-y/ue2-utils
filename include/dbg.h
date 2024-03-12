/* DEBUG */
#ifndef DEBUG_H
#define DEBUG_H

#ifndef TRACE
#define TRACE 0
#endif

#if TRACE >= 1
#define TRACE1 printf
#else
#define TRACE1 nothing
#endif

#if TRACE >= 2
#define TRACE2 printf
#else
#define TRACE2 nothing
#endif

#if TRACE >= 3
#define TRACE3 printf
#else
#define TRACE3 nothing
#endif

nothing() {}

#endif
