/* DBG */
#ifndef DEBUG_H
#define DEBUG_H

#define DBG_NONE 0
#define DBG_INFO 1
#define DBG_DEBUG 2
#define DBG_TRACE 3

#ifndef LOG
#define LOG DBG_NONE
#endif


#if LOG >= DBG_INFO
#define INFO warnx
#define VINFO printf
#define INFOEN
#else
/* #define INFO nothing */
#define INFO(...)
#define VINFO(...)
#endif

#if LOG >= DBG_DEBUG
#define DBG warnx
#define DBGEN
#else
/* #define DBG nothing */
#define DBG(...)
#endif

#if LOG >= DBG_TRACE
#define TRACE warnx
#define VTRACE printf
#define TRACEEN
#else
/* #define TRACE nothing */
#define TRACE(...)
#define VTRACE(...)
#endif


#endif
