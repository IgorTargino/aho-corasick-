#ifndef AHO_CONFIG_H
#define AHO_CONFIG_H

#include <stdint.h>

// Configuration optimized for STM32F0 (8KB RAM)
// This configuration ensures the automaton fits in ~2-3KB
#define AC_MAX_VERTICES 50          // Sufficient for ~10-12 patterns
#define AC_MAX_PATTERNS 15          // Maximum recommended for STM32F0
#define AC_MAX_PATTERNS_PER_VERTEX 2
#define AC_MAX_TRANSITIONS_PER_VERTEX 26
#define INVALID_VERTEX_U8 255

// Memory estimation:
// - ac_vertex_t ≈ 30 bytes × 50 = 1.5KB
// - Base structures ≈ 300 bytes  
// - Pattern storage ≈ 15 × 8 = 120 bytes
// Total ≈ 2KB (25% of 8KB - acceptable)

// Debug prints (disable for production STM32)
#ifdef DEBUG_PRINTS
    #include <stdio.h>
    #define DEBUG_PRINTF(format, ...) printf("[DEBUG] " format, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTF(format, ...) ((void)0)
#endif

#endif // AHO_CONFIG_H
