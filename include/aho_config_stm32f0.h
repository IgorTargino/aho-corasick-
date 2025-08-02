#ifndef AHO_CONFIG_STM32F0_H
#define AHO_CONFIG_STM32F0_H

#include <stdint.h>

// STM32F0 Optimized Configuration
// Total structure size should be < 4KB for 8KB system

#define AC_MAX_VERTICES 60          
#define AC_MAX_PATTERNS 15          
#define AC_MAX_PATTERNS_PER_VERTEX 2
#define AC_MAX_TRANSITIONS_PER_VERTEX 26
#define INVALID_VERTEX_U8 255

// Memory calculation:
// ac_vertex_t = ~30 bytes * 60 = ~1.8KB
// ac_automaton_t base = ~200 bytes
// Queue = ~60 bytes
// Patterns storage = ~15 * 8 = ~120 bytes
// Total ≈ 2.2KB (acceptable for 8KB system)

// Debug prints (disable for production)
#ifdef DEBUG_PRINTS
    #include <stdio.h>
    #define DEBUG_PRINTF(format, ...) printf("[DEBUG] " format, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTF(format, ...) ((void)0)
#endif

// STM32F0 specific optimizations
#ifdef STM32F0
    // Reduce memory footprint further if needed
    #undef AC_MAX_VERTICES
    #undef AC_MAX_PATTERNS
    #define AC_MAX_VERTICES 40
    #define AC_MAX_PATTERNS 10
#endif

#endif // AHO_CONFIG_STM32F0_H
