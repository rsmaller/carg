#pragma once

#define MEM_DEBUG_H

#ifdef MEM_DEBUG_DISABLE
    #define _malloc malloc
    #define _calloc calloc
    #define _realloc realloc
#else
    #define _malloc malloc_db
    #define _calloc calloc_db
    #define _realloc realloc_db
#endif

#ifndef _INC_STDDEF
    #include <stddef.h>
#endif

inline void *malloc_db(size_t bytes) {
    return (void *)0;
}

inline void *calloc_db(size_t count, size_t bytes) {
    return (void *)0;
}

inline void *realloc_db(void *old, size_t newSize) {
    return (void *)0;
}