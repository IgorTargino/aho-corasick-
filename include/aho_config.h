#ifndef AHO_CONFIG_H
#define AHO_CONFIG_H

#define AC_K_ALPHABET_SIZE          26
#define AC_MAX_VERTICES             30
#define AC_MAX_PATTERNS             30  
#define AC_MAX_PATTERNS_PER_VERTEX  2

#ifdef DEBUG_PRINTS
    #include <stdio.h>
    #define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...) ((void)0)
#endif

#endif 