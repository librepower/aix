//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// Endianess handling, swapping 16bit and 32bit.
// Fixed for AIX/POWER big-endian systems
//

#ifndef __I_SWAP__
#define __I_SWAP__

#include <stdint.h>

// Check endianness
#if defined(__BIG_ENDIAN__) || defined(_AIX) || defined(__powerpc__) || defined(__ppc__)
#define SYS_BIG_ENDIAN

static inline int16_t swap16(int16_t x) {
    return (int16_t)(((uint16_t)x >> 8) | ((uint16_t)x << 8));
}

static inline int32_t swap32(int32_t x) {
    uint32_t ux = (uint32_t)x;
    return (int32_t)(((ux >> 24) & 0xff) | ((ux >> 8) & 0xff00) |
                     ((ux << 8) & 0xff0000) | ((ux << 24) & 0xff000000));
}

#define SHORT(x)  swap16(x)
#define LONG(x)   swap32(x)

#else
#define SYS_LITTLE_ENDIAN
#define SHORT(x)  ((signed short) (x))
#define LONG(x)   ((signed int) (x))
#endif

#endif
