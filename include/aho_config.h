#ifndef AHO_CONFIG_H
#define AHO_CONFIG_H

/**
 * @file aho_config.h
 * @brief Configurações globais para o autômato de Aho-Corasick.
 *
 * Este arquivo define os limites e parâmetros principais usados em todo o projeto,
 * como o tamanho máximo do trie, do alfabeto e dos padrões.
 */

/**
 * @def AC_MAX_VERTICES
 * @brief Número máximo de vértices (nós) permitidos no trie/autômato.
 */
#define AC_MAX_VERTICES 40

/**
 * @def AC_K_ALPHABET_SIZE
 * @brief Tamanho do alfabeto utilizado (por padrão, 26 para letras minúsculas).
 */
#define AC_K_ALPHABET_SIZE 26

/**
 * @def AC_MAX_PATTERNS
 * @brief Número máximo de padrões que podem ser adicionados ao autômato.
 */
#define AC_MAX_PATTERNS 10 

/**
 * @def AC_MAX_PATTERNS_PER_NODE
 * @brief Número máximo de padrões que podem terminar em um mesmo nó do trie.
 */
#define AC_MAX_PATTERNS_PER_NODE 2

#endif