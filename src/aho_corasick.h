#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h> // Para o tipo bool (C99 em diante)
#include <stddef.h>  // Para size_t (geralmente não estritamente necessário aqui, mas bom ter)

// --- Constantes de Configuração ---
// (Ajuste conforme a necessidade antes da compilação)

// Tamanho do alfabeto (ex: 'a'-'z')
#define AC_K_ALPHABET_SIZE 26

// Número máximo de nós/estados na Trie/Autômato.
// Deve ser suficiente para a soma dos comprimentos de todos os padrões
// e suas sobreposições.
#define AC_MAX_VERTICES 40 // 40×213=8520 bytes (aproximadamente 8.3KB) 

// --- Estruturas de Dados ---

// Definindo a estrutura para um vértice/estado do autômato
typedef struct ac_vertex_s {
    int next[AC_K_ALPHABET_SIZE]; // Transições diretas da Trie para filhos
    int go[AC_K_ALPHABET_SIZE];   // Transições do autômato (calculadas)
    int link;                     // Link de falha (suffix link)
    bool output;                  // true se um padrão termina neste nó
} ac_vertex_t;

// --- Protótipos das Funções Públicas ---

/**
 * @brief Inicializa o autômato Aho-Corasick.
 * Deve ser chamada antes de adicionar padrões ou realizar buscas.
 */
void ac_initialize_automaton(void);

/**
 * @brief Adiciona uma string (padrão) ao autômato.
 * @param pattern A string do padrão a ser adicionada (espera-se apenas 'a'-'z').
 */
void ac_add_pattern(const char* pattern);

/**
 * @brief Constrói os links de falha e as transições 'go' do autômato.
 * Deve ser chamada após adicionar todos os padrões e antes de iniciar qualquer busca.
 */
void ac_build_automaton(void);

/**
 * @brief Realiza a busca dos padrões adicionados em um texto.
 * Imprime as ocorrências encontradas na saída padrão (usando printf).
 * @param text O texto onde os padrões serão buscados.
 */
void ac_search(const char* text);

#endif // AHO_CORASICK_H