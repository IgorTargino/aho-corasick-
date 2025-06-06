#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "aho_corasick.h"

// --- Configuration for this application ---
#define INPUT_TEXT_MAX_LEN 512
#define OTHER_APP_BUFFER_SIZE 256 // Example other memory usage

// --- Global flag for filter detection ---
// In a real embedded app, this might be more sophisticated (e.g., part of a task's state)
volatile bool g_forbidden_pattern_found = false;

// --- Match Callback Implementation ---
// This function is called by ac_search when a forbidden pattern is found.
void ac_set_match_callback(const char* pattern_found, int text_position) {
    g_forbidden_pattern_found = true;
    // For debugging or more detailed logging (enabled only in debug builds):
    #ifdef AC_DEBUG_PRINTS
    printf("[FILTER] Detected: '%s' at position %d\n", pattern_found, text_position);
    #endif
}

// --- Function to check a message ---
bool is_message_forbidden(ac_automaton_t* filter_automaton, const char* message) {
    g_forbidden_pattern_found = false; // Reset flag for each new message
    
    // Note: The current implementation expects lowercase input since char_to_index 
    // handles both uppercase and lowercase by converting to same indices.
    // For consistency, we'll assume patterns and input are already properly cased.
    
    ac_search(filter_automaton, message);
    
    return g_forbidden_pattern_found;
}

// --- Helper function to display memory usage ---
static void display_memory_usage(const ac_automaton_t* filter) {
    printf("--- Estimativa de Uso de Memoria ---\n");
    printf("Configuracao do Automato:\n");
    printf("  AC_MAX_VERTICES: %d\n", AC_MAX_VERTICES);
    printf("  AC_MAX_PATTERNS: %d\n", AC_MAX_PATTERNS);
    printf("  AC_MAX_PATTERNS_PER_VERTEX: %d\n", AC_MAX_PATTERNS_PER_VERTEX);
    printf("  AC_K_ALPHABET_SIZE: %d\n", AC_K_ALPHABET_SIZE);
    
    printf("\nEstatisticas do Automato Construido:\n");
    printf("  Vertices utilizados: %d/%d (%.1f%%)\n", 
           ac_get_vertex_count(filter), AC_MAX_VERTICES,
           (100.0 * ac_get_vertex_count(filter)) / AC_MAX_VERTICES);
    printf("  Padroes carregados: %d/%d (%.1f%%)\n",
           ac_get_pattern_count(filter), AC_MAX_PATTERNS,
           (100.0 * ac_get_pattern_count(filter)) / AC_MAX_PATTERNS);
    
    printf("\nMemoria Estatica Estimada:\n");
    size_t automaton_size = sizeof(ac_automaton_t);
    size_t input_buffer_size = INPUT_TEXT_MAX_LEN;
    size_t other_buffer_size = OTHER_APP_BUFFER_SIZE;
    size_t total_static = automaton_size + input_buffer_size + other_buffer_size;
    
    printf("  Automato (ac_automaton_t): %zu bytes\n", automaton_size);
    printf("  Buffer de entrada: %zu bytes\n", input_buffer_size);
    printf("  Outros buffers: %zu bytes\n", other_buffer_size);
    printf("  Total estimado: %zu bytes (%.2f KB)\n", total_static, total_static / 1024.0);
    
    // Check against memory budget
    const size_t MEMORY_BUDGET = 8192; // 8KB limit
    printf("  Orcamento disponivel: %zu bytes (%.2f KB)\n", MEMORY_BUDGET, MEMORY_BUDGET / 1024.0);
    printf("  Margem restante: %zu bytes (%.2f KB)\n", 
           MEMORY_BUDGET - total_static, (MEMORY_BUDGET - total_static) / 1024.0);
    
    if (total_static > MEMORY_BUDGET) {
        printf("  ⚠️  ALERTA: Orcamento de memoria excedido!\n");
    } else {
        printf("  ✅ Dentro do orcamento de memoria\n");
    }
    printf("----------------------------------------\n\n");
}

// --- Function to validate automaton state ---
static bool validate_automaton_state(const ac_automaton_t* filter) {
    if (!ac_is_built(filter)) {
        printf("❌ Erro: Automato nao foi construido corretamente!\n");
        return false;
    }
    
    if (ac_get_vertex_count(filter) == 0) {
        printf("❌ Erro: Nenhum vertice foi alocado!\n");
        return false;
    }
    
    if (ac_get_pattern_count(filter) == 0) {
        printf("❌ Erro: Nenhum padrao foi adicionado!\n");
        return false;
    }
    
    printf("✅ Automato validado com sucesso\n");
    return true;
}

// --- Main Application ---
int main() {
    printf("=== Filtro de Conteudo - Aho-Corasick ===\n\n");
    
    volatile char other_buffer[OTHER_APP_BUFFER_SIZE];
    for(int i = 0; i < OTHER_APP_BUFFER_SIZE; ++i) {
        other_buffer[i] = (char)(i & 0xFF);
    }

    // Initialize the content filter automaton
    ac_automaton_t content_filter;
    ac_initialize_automaton(&content_filter);

    // Define forbidden words (ensure consistent casing)
    const char* forbidden_words[] = {
        "spam", 
        "virus", 
        "hack", 
        "malware",
        "phishing",
        "erro",
        "trojan",
        "worm"
        // Add more, up to AC_MAX_PATTERNS
    };
    int num_forbidden = sizeof(forbidden_words) / sizeof(forbidden_words[0]);

    // Validate we don't exceed pattern limit
    if (num_forbidden > AC_MAX_PATTERNS) {
        printf("⚠️  Aviso: Numero de padroes (%d) excede limite (%d)\n", 
               num_forbidden, AC_MAX_PATTERNS);
        num_forbidden = AC_MAX_PATTERNS;
    }

    printf("Configurando filtro de conteudo...\n");
    int patterns_added = 0;
    
    for (int i = 0; i < num_forbidden; ++i) {
        if (ac_add_pattern(&content_filter, forbidden_words[i])) {
            patterns_added++;
            printf("  ✅ Padrao adicionado: \"%s\"\n", forbidden_words[i]);
        } else {
            printf("  ❌ Falha ao adicionar padrao: \"%s\"\n", forbidden_words[i]);
        }
    }
    
    printf("\nResumo da configuracao:\n");
    printf("  Padroes solicitados: %d\n", num_forbidden);
    printf("  Padroes adicionados: %d\n", patterns_added);
    printf("  Vertices utilizados: %d\n", ac_get_vertex_count(&content_filter));

    if (patterns_added == 0) {
        printf("❌ Erro critico: Nenhum padrao foi adicionado!\n");
        return 1;
    }

    printf("\nConstruindo automato do filtro...\n");
    ac_build_automaton(&content_filter);
    
    // Validate automaton was built correctly
    if (!validate_automaton_state(&content_filter)) {
        printf("❌ Erro critico: Falha na construcao do automato!\n");
        return 1;
    }
    
    printf("✅ Automato construido com sucesso!\n\n");
    
    // Display memory usage information
    display_memory_usage(&content_filter);

    // Test messages
    const char* test_messages[] = {
        "esta e uma mensagem normal e segura",
        "alerta um virus foi detectado no seu email",     // "virus"
        "cuidado com links de phishing",                  // "phishing"
        "o sistema reportou um erro grave",               // "erro"
        "texto completamente limpo e aprovado",
        "nova tentativa de hack bloqueada",               // "hack"
        "promocao imperdivel nao e spam",                 // "spam"
        "detectado malware no sistema",                   // "malware"
        "mensagem sem palavras proibidas",
        "trojan encontrado no arquivo"                    // "trojan"
    };
    int num_messages = sizeof(test_messages) / sizeof(test_messages[0]);

    printf("=== Teste do Filtro de Conteudo ===\n");
    int blocked_count = 0;
    int allowed_count = 0;

    for (int i = 0; i < num_messages; ++i) {
        printf("\n[%d] Analisando: \"%s\"\n", i + 1, test_messages[i]);
        
        if (is_message_forbidden(&content_filter, test_messages[i])) {
            printf("    🚫 Resultado: MENSAGEM BLOQUEADA!\n");
            blocked_count++;
        } else {
            printf("    ✅ Resultado: Mensagem Permitida\n");
            allowed_count++;
        }
    }

    // Summary
    printf("\n=== Resumo dos Testes ===\n");
    printf("Total de mensagens testadas: %d\n", num_messages);
    printf("Mensagens bloqueadas: %d\n", blocked_count);
    printf("Mensagens permitidas: %d\n", allowed_count);
    printf("Taxa de deteccao: %.1f%%\n", (100.0 * blocked_count) / num_messages);
    
    printf("\n=== Fim da Execucao ===\n");
    
    // Prevent compiler optimization of unused buffer
    (void)other_buffer;
    
    return 0;
}