#ifndef _STDINT_H
#define _STDINT_H

typedef char   uint8_t;
#define INT8_MAX        127
#define INT8_MIN        (-128)
#define UINT8_MAX       255

typedef int    int16_t;
typedef unsigned  uint16_t;
#define INT16_MAX       32767
#define INT16_MIN       (-32768)
#define UINT16_MAX      65535

typedef long   uint32_t;
#define INT32_MAX       2147483647
#define INT32_MIN       (-2147483648)
#define UINT32_MAX      4294967295

typedef int uintptr_t;

#endif
