#ifndef AHO_CONFIG_H
#define AHO_CONFIG_H

#include <stdint.h>

// Configurações de capacidade para sistemas embarcados (8KB)
// Usando uint8_t (0-255), podemos ter até 255 vértices
#define AC_MAX_VERTICES 50
#define AC_MAX_PATTERNS 20
#define AC_MAX_PATTERNS_PER_VERTEX 2
#define AC_MAX_TRANSITIONS_PER_VERTEX 26
#define AC_K_ALPHABET_SIZE 26

// Constantes para valores inválidos
#define INVALID_VERTEX_U8 255                 // Valor inválido para uint8_t

// Debug prints (desabilitado em produção)
#ifdef DEBUG_PRINTS
    #include <stdio.h>
    #define DEBUG_PRINTF(format, ...) printf("[DEBUG] " format, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTF(format, ...) ((void)0)
#endif

#endif