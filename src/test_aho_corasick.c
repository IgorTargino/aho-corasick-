#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// --- Test Framework ---
static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

// Test result tracking
typedef struct {
    const char* pattern;
    int position;
    bool found;
} test_match_t;

static test_match_t test_matches[100];
static int match_count = 0;

// --- Test Utilities ---
#define TEST_START(name) \
    do { \
        test_count++; \
        match_count = 0; \
        memset(test_matches, 0, sizeof(test_matches)); \
        printf("\n[TEST %d] %s\n", test_count, name); \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✅ PASS: %s\n", message); \
            test_passed++; \
        } else { \
            printf("  ❌ FAIL: %s\n", message); \
            test_failed++; \
        } \
    } while(0)

#define TEST_SUMMARY() \
    do { \
        printf("\n=== RESUMO DOS TESTES ===\n"); \
        printf("Total de testes: %d\n", test_count); \
        printf("Testes passaram: %d\n", test_passed); \
        printf("Testes falharam: %d\n", test_failed); \
        printf("Taxa de sucesso: %.1f%%\n", (100.0 * test_passed) / (test_passed + test_failed)); \
        printf("Status final: %s\n", test_failed == 0 ? "✅ TODOS OS TESTES PASSARAM" : "❌ ALGUNS TESTES FALHARAM"); \
    } while(0)

// --- Mock Callback Implementation ---
void ac_set_match_callback(const char* pattern, int position) {
    if (match_count < 100) {
        test_matches[match_count].pattern = pattern;
        test_matches[match_count].position = position;
        test_matches[match_count].found = true;
        match_count++;
    }
}

// --- Helper Functions ---
static bool has_match(const char* expected_pattern) {
    for (int i = 0; i < match_count; i++) {
        if (strcmp(test_matches[i].pattern, expected_pattern) == 0) {
            return true;
        }
    }
    return false;
}

static bool has_match_at_position(const char* expected_pattern, int expected_position) {
    for (int i = 0; i < match_count; i++) {
        if (strcmp(test_matches[i].pattern, expected_pattern) == 0 && 
            test_matches[i].position == expected_position) {
            return true;
        }
    }
    return false;
}

// --- Test Cases ---

void test_initialization() {
    TEST_START("Inicializacao do Automato");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    TEST_ASSERT(ac_get_vertex_count(&ac) == 1, "Root vertex deve ser criado");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 0, "Nenhum padrao inicial");
    TEST_ASSERT(!ac_is_built(&ac), "Automato nao deve estar construido sem padroes");
}

void test_pattern_addition() {
    TEST_START("Adicao de Padroes");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Test valid pattern addition
    TEST_ASSERT(ac_add_pattern(&ac, "test"), "Deve adicionar padrao valido");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 1, "Contador de padroes deve incrementar");
    
    // Test empty pattern
    TEST_ASSERT(!ac_add_pattern(&ac, ""), "Deve rejeitar padrao vazio");
    TEST_ASSERT(!ac_add_pattern(&ac, NULL), "Deve rejeitar padrao NULL");
    
    // Test case insensitive handling
    TEST_ASSERT(ac_add_pattern(&ac, "Test"), "Deve aceitar letras maiusculas");
    TEST_ASSERT(ac_add_pattern(&ac, "TEST"), "Deve aceitar todas maiusculas");
    
    // Test invalid characters
    TEST_ASSERT(ac_add_pattern(&ac, "test123"), "Deve aceitar padrao com numeros (ignorando numeros)");
    
    TEST_ASSERT(ac_get_pattern_count(&ac) == 4, "Deve ter 4 padroes validos");
}

void test_pattern_limit() {
    TEST_START("Limite de Padroes");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Add patterns up to limit
    char pattern[10];
    int added = 0;
    for (int i = 0; i < AC_MAX_PATTERNS + 5; i++) {
        sprintf(pattern, "pat%d", i);
        if (ac_add_pattern(&ac, pattern)) {
            added++;
        }
    }
    
    TEST_ASSERT(added == AC_MAX_PATTERNS, "Deve adicionar exatamente AC_MAX_PATTERNS padroes");
    TEST_ASSERT(ac_get_pattern_count(&ac) == AC_MAX_PATTERNS, "Contador deve refletir limite");
}

void test_vertex_limit() {
    TEST_START("Limite de Vertices");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Try to add very long patterns to exhaust vertices
    char long_pattern[100];
    for (int i = 0; i < 100; i++) {
        long_pattern[i] = 'a';
    }
    long_pattern[99] = '\0';
    
    // This should eventually fail due to vertex limit
    bool limit_reached = false;
    for (int i = 0; i < 10; i++) {
        if (!ac_add_pattern(&ac, long_pattern)) {
            limit_reached = true;
            break;
        }
        // Make pattern unique for next iteration
        long_pattern[0] = 'a' + i + 1;
    }
    
    TEST_ASSERT(ac_get_vertex_count(&ac) <= AC_MAX_VERTICES, "Nao deve exceder limite de vertices");
}

void test_automaton_construction() {
    TEST_START("Construcao do Automato");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Add some patterns
    ac_add_pattern(&ac, "he");
    ac_add_pattern(&ac, "she");
    ac_add_pattern(&ac, "his");
    ac_add_pattern(&ac, "hers");
    
    // Build automaton
    ac_build_automaton(&ac);
    
    TEST_ASSERT(ac_is_built(&ac), "Automato deve estar construido");
    TEST_ASSERT(ac_get_vertex_count(&ac) > 1, "Deve ter mais que o root vertex");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 4, "Deve manter todos os padroes");
}

void test_empty_automaton_construction() {
    TEST_START("Construcao de Automato Vazio");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Try to build without patterns
    ac_build_automaton(&ac);
    
    TEST_ASSERT(ac_get_vertex_count(&ac) == 1, "Deve ter apenas root vertex");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 0, "Nenhum padrao");
}

void test_single_pattern_search() {
    TEST_START("Busca com Padrao Unico");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    ac_add_pattern(&ac, "test");
    ac_build_automaton(&ac);
    
    // Test exact match
    ac_search(&ac, "test");
    TEST_ASSERT(match_count == 1, "Deve encontrar uma ocorrencia");
    TEST_ASSERT(has_match("test"), "Deve encontrar padrao 'test'");
    
    // Reset and test within text
    match_count = 0;
    ac_search(&ac, "this is a test string");
    TEST_ASSERT(match_count == 1, "Deve encontrar uma ocorrencia no texto");
    TEST_ASSERT(has_match_at_position("test", 13), "Deve encontrar 'test' na posicao 13");
    
    // Reset and test no match
    match_count = 0;
    ac_search(&ac, "nothing here");
    TEST_ASSERT(match_count == 0, "Nao deve encontrar ocorrencias");
}

void test_multiple_patterns_search() {
    TEST_START("Busca com Multiplos Padroes");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    ac_add_pattern(&ac, "he");
    ac_add_pattern(&ac, "she");
    ac_add_pattern(&ac, "his");
    ac_add_pattern(&ac, "hers");
    ac_build_automaton(&ac);
    
    // Test overlapping matches
    ac_search(&ac, "ushers");
    TEST_ASSERT(match_count >= 1, "Deve encontrar pelo menos uma ocorrencia");
    TEST_ASSERT(has_match("she"), "Deve encontrar 'she' em 'ushers'");
    TEST_ASSERT(has_match("he"), "Deve encontrar 'he' em 'ushers'");
    TEST_ASSERT(has_match("hers"), "Deve encontrar 'hers' em 'ushers'");
}

void test_case_insensitive_search() {
    TEST_START("Busca Case-Insensitive");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    ac_add_pattern(&ac, "test");
    ac_build_automaton(&ac);
    
    // Test different cases
    ac_search(&ac, "TEST");
    TEST_ASSERT(match_count == 1, "Deve encontrar 'test' em 'TEST'");
    
    match_count = 0;
    ac_search(&ac, "Test");
    TEST_ASSERT(match_count == 1, "Deve encontrar 'test' em 'Test'");
    
    match_count = 0;
    ac_search(&ac, "tEsT");
    TEST_ASSERT(match_count == 1, "Deve encontrar 'test' em 'tEsT'");
}

void test_repeated_patterns() {
    TEST_START("Padroes Repetidos no Texto");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    ac_add_pattern(&ac, "ab");
    ac_build_automaton(&ac);
    
    ac_search(&ac, "ababab");
    TEST_ASSERT(match_count == 3, "Deve encontrar 3 ocorrencias de 'ab' em 'ababab'");
}

void test_invalid_characters() {
    TEST_START("Caracteres Invalidos");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    ac_add_pattern(&ac, "test");
    ac_build_automaton(&ac);
    
    // Test with numbers and symbols
    ac_search(&ac, "test123!@#");
    TEST_ASSERT(match_count == 1, "Deve encontrar 'test' ignorando caracteres invalidos");
    
    match_count = 0;
    ac_search(&ac, "123test456");
    TEST_ASSERT(match_count == 1, "Deve encontrar 'test' entre caracteres invalidos");
}

void test_edge_cases() {
    TEST_START("Casos Extremos");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Test with single character pattern
    ac_add_pattern(&ac, "a");
    ac_build_automaton(&ac);
    
    ac_search(&ac, "aaa");
    TEST_ASSERT(match_count == 3, "Deve encontrar 3 ocorrencias de 'a' em 'aaa'");
    
    // Test empty text
    match_count = 0;
    ac_search(&ac, "");
    TEST_ASSERT(match_count == 0, "Nao deve encontrar nada em texto vazio");
    
    // Test NULL text
    match_count = 0;
    ac_search(&ac, NULL);
    TEST_ASSERT(match_count == 0, "Deve lidar com texto NULL graciosamente");
}

void test_performance_patterns() {
    TEST_START("Padroes de Performance");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Add patterns that might cause performance issues
    ac_add_pattern(&ac, "a");
    ac_add_pattern(&ac, "aa");
    ac_add_pattern(&ac, "aaa");
    ac_add_pattern(&ac, "aaaa");
    ac_build_automaton(&ac);
    
    // Test with string that should trigger many matches
    ac_search(&ac, "aaaaa");
    TEST_ASSERT(match_count > 0, "Deve encontrar multiplas ocorrencias");
    
    // Verify overlapping patterns are handled correctly
    bool found_a = has_match("a");
    bool found_aa = has_match("aa");
    bool found_aaa = has_match("aaa");
    bool found_aaaa = has_match("aaaa");
    
    TEST_ASSERT(found_a, "Deve encontrar padrao 'a'");
    TEST_ASSERT(found_aa, "Deve encontrar padrao 'aa'");
    TEST_ASSERT(found_aaa, "Deve encontrar padrao 'aaa'");
    TEST_ASSERT(found_aaaa, "Deve encontrar padrao 'aaaa'");
}

void test_memory_usage() {
    TEST_START("Uso de Memoria");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    size_t initial_size = sizeof(ac_automaton_t);
    printf("  Tamanho da estrutura: %zu bytes\n", initial_size);
    
    // Add patterns and check memory usage
    for (int i = 0; i < 5; i++) {
        char pattern[10];
        sprintf(pattern, "test%d", i);
        ac_add_pattern(&ac, pattern);
    }
    
    ac_build_automaton(&ac);
    
    printf("  Vertices utilizados: %d/%d\n", ac_get_vertex_count(&ac), AC_MAX_VERTICES);
    printf("  Padroes utilizados: %d/%d\n", ac_get_pattern_count(&ac), AC_MAX_PATTERNS);
    
    float vertex_usage = (100.0 * ac_get_vertex_count(&ac)) / AC_MAX_VERTICES;
    float pattern_usage = (100.0 * ac_get_pattern_count(&ac)) / AC_MAX_PATTERNS;
    
    TEST_ASSERT(vertex_usage < 50.0, "Uso de vertices deve ser eficiente");
    TEST_ASSERT(pattern_usage < 50.0, "Uso de padroes deve ser eficiente");
    TEST_ASSERT(initial_size <= 8192, "Estrutura deve caber no orcamento de memoria");
}

// --- Main Test Runner ---
int main() {
    printf("=== SUITE DE TESTES - AHO-CORASICK ===\n");
    printf("Configuracao de Teste:\n");
    printf("  AC_MAX_VERTICES: %d\n", AC_MAX_VERTICES);
    printf("  AC_MAX_PATTERNS: %d\n", AC_MAX_PATTERNS);
    printf("  AC_MAX_PATTERNS_PER_VERTEX: %d\n", AC_MAX_PATTERNS_PER_VERTEX);
    printf("  AC_K_ALPHABET_SIZE: %d\n", AC_K_ALPHABET_SIZE);
    
    // Run all test cases
    test_initialization();
    test_pattern_addition();
    test_pattern_limit();
    test_vertex_limit();
    test_automaton_construction();
    test_empty_automaton_construction();
    test_single_pattern_search();
    test_multiple_patterns_search();
    test_case_insensitive_search();
    test_repeated_patterns();
    test_invalid_characters();
    test_edge_cases();
    test_performance_patterns();
    test_memory_usage();
    
    // Print summary
    TEST_SUMMARY();
    
    return test_failed == 0 ? 0 : 1;
}