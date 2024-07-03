/* DBG */
#ifndef DEBUG_H
#define DEBUG_H

#define DBG_NONE 0
#define DBG_INFO 1
#define DBG_DEBUG 2
#define DBG_TRACE 3

#ifndef LOG
#define LOG DBG_INFO
#endif


#if LOG >= DBG_INFO
#define INFO printf
#define INFOEN
#else
/* #define INFO nothing */
#define INFO(...)
#endif

#if LOG >= DBG_DEBUG
#define DBG printf
#define DBGEN
#else
/* #define DBG nothing */
#define DBG(...)
#endif

#if LOG >= DBG_TRACE
#define TRACE printf
#define TRACEEN
#else
/* #define TRACE nothing */
#define TRACE(...)
#endif


#endif
