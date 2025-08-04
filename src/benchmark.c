#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "aho_corasick.h"

// Estrutura para medições de performance focadas
typedef struct {
    struct timespec start_time;
    struct timespec end_time;
    double elapsed_microseconds;
    size_t automaton_memory_bytes;
    uint16_t vertices_used;
    uint16_t transitions_total;
    uint16_t patterns_loaded;
} ac_performance_t;

// Variáveis globais para medições
static ac_performance_t g_perf;
static uint16_t g_match_count;

// Callback silencioso para medições puras
void performance_callback(const char* pattern, int position) {
    (void)pattern;
    (void)position;
    g_match_count++;
}

// Função para calcular uso de memória do autômato
size_t calculate_automaton_memory(const ac_automaton_t *ac) {
    size_t total = sizeof(ac_automaton_t);
    
    // Calcula memória adicional baseada no uso real
    size_t vertices_mem = ac->vertex_count * sizeof(ac_vertex_t);
    size_t patterns_mem = ac->pattern_count * sizeof(char*);
    
    printf("   Estrutura base: %zu bytes\n", sizeof(ac_automaton_t));
    printf("   Vértices usados: %u/%u (%zu bytes)\n", 
           ac->vertex_count, AC_MAX_VERTICES, vertices_mem);
    printf("   Padrões carregados: %u/%u (%zu bytes)\n", 
           ac->pattern_count, AC_MAX_PATTERNS, patterns_mem);
    
    return total;
}

// Função para contar transições totais
uint16_t count_total_transitions(const ac_automaton_t *ac) {
    uint16_t total = 0;
    for (uint8_t i = 0; i < ac->vertex_count; i++) {
        total += ac->vertices[i].num_transitions;
    }
    return total;
}

// Função para iniciar medição de tempo
void start_performance_timing(void) {
    clock_gettime(CLOCK_MONOTONIC, &g_perf.start_time);
}

// Função para finalizar medição de tempo
void end_performance_timing(void) {
    clock_gettime(CLOCK_MONOTONIC, &g_perf.end_time);
    
    long seconds = g_perf.end_time.tv_sec - g_perf.start_time.tv_sec;
    long nanoseconds = g_perf.end_time.tv_nsec - g_perf.start_time.tv_nsec;
    
    g_perf.elapsed_microseconds = (seconds * 1000000.0) + (nanoseconds / 1000.0);
}

// Função para medição completa da construção
void measure_construction(ac_automaton_t *automaton) {
    printf("\n=== MEDIÇÃO: CONSTRUÇÃO DO AUTÔMATO ===\n");
    
    // Estatísticas antes da construção
    printf("Estado antes da construção:\n");
    g_perf.vertices_used = automaton->vertex_count;
    g_perf.patterns_loaded = automaton->pattern_count;
    g_perf.transitions_total = count_total_transitions(automaton);
    
    printf("   Vértices: %u\n", g_perf.vertices_used);
    printf("   Padrões: %u\n", g_perf.patterns_loaded);
    printf("   Transições: %u\n", g_perf.transitions_total);
    
    // Medição de tempo
    start_performance_timing();
    ac_build(automaton);
    end_performance_timing();
    
    // Estatísticas após construção
    printf("\nEstado após a construção:\n");
    g_perf.transitions_total = count_total_transitions(automaton);
    g_perf.automaton_memory_bytes = calculate_automaton_memory(automaton);
    
    printf("   Transições finais: %u\n", g_perf.transitions_total);
    
    printf("\n📊 Resultados da Construção:\n");
    printf("   ⏱️  Tempo: %.3f µs (%.6f ms)\n", 
           g_perf.elapsed_microseconds, g_perf.elapsed_microseconds / 1000.0);
    printf("   💾 Memória total: %zu bytes (%.2f KB)\n", 
           g_perf.automaton_memory_bytes, g_perf.automaton_memory_bytes / 1024.0);
    
    if (g_perf.automaton_memory_bytes <= 8192) {
        printf("   ✅ Dentro do limite de 8KB\n");
    } else {
        printf("   ❌ Excede o limite de 8KB\n");
    }
}

// Função para medição de busca
void measure_search(ac_automaton_t *automaton, const char* text, const char* description) {
    printf("\n=== MEDIÇÃO: BUSCA DE PADRÕES ===\n");
    printf("Texto: %s\n", description);
    printf("Tamanho: %zu caracteres\n", strlen(text));
    
    g_match_count = 0;
    
    // Medição de tempo
    start_performance_timing();
    ac_search(automaton, text);
    end_performance_timing();
    
    printf("\n📊 Resultados da Busca:\n");
    printf("   ⏱️  Tempo: %.3f µs (%.6f ms)\n", 
           g_perf.elapsed_microseconds, g_perf.elapsed_microseconds / 1000.0);
    printf("   🎯 Matches: %u\n", g_match_count);
    printf("   🚀 Taxa: %.2f chars/µs\n", 
           strlen(text) / g_perf.elapsed_microseconds);
}

// Lista de palavras para teste
static const char* test_patterns[] = {
    "spam", "hack", "virus", "malware", "phishing",
    "scam", "fraud", "illegal", "crack", "trojan"
};

int main(void) {
    printf("=== MEDIÇÕES PRECISAS AHO-CORASICK ===\n");
    printf("Foco: Construção do grafo + Busca de padrões\n");
    printf("Objetivo: Manter uso < 8KB de RAM\n\n");
    
    ac_automaton_t automaton;
    
    // Inicialização
    printf("🚀 Inicializando autômato...\n");
    ac_init(&automaton, performance_callback);
    
    printf("📋 Tamanho da estrutura: %zu bytes\n", sizeof(ac_automaton_t));
    
    // Carregamento de padrões
    printf("\n📝 Carregando padrões de teste...\n");
    uint8_t num_patterns = sizeof(test_patterns) / sizeof(test_patterns[0]);
    uint8_t loaded = 0;
    
    for (uint8_t i = 0; i < num_patterns && i < AC_MAX_PATTERNS; i++) {
        if (ac_add_pattern(&automaton, test_patterns[i])) {
            printf("   ✓ '%s'\n", test_patterns[i]);
            loaded++;
        } else {
            printf("   ✗ Falha: '%s'\n", test_patterns[i]);
        }
    }
    
    printf("\nPadrões carregados: %u/%u\n", loaded, num_patterns);
    
    // MEDIÇÃO 1: Construção do autômato
    measure_construction(&automaton);
    
    // Textos de teste
    const char* test_texts[] = {
        "texto limpo sem problemas",
        "cuidado com spam aqui",
        "este malware é um trojan",
        "software com crack illegal",
        "texto longo para performance: este texto contém várias palavras e deve testar a velocidade do algoritmo de busca em textos maiores com mais conteúdo"
    };
    
    // MEDIÇÕES 2-6: Diferentes tipos de busca
    uint8_t num_texts = sizeof(test_texts) / sizeof(test_texts[0]);
    for (uint8_t i = 0; i < num_texts; i++) {
        char desc[100];
        snprintf(desc, sizeof(desc), "Teste %u", i + 1);
        measure_search(&automaton, test_texts[i], desc);
    }
    
    printf("\n=== RESUMO FINAL ===\n");
    printf("Todas as medições focaram apenas no processamento\n");
    printf("do algoritmo Aho-Corasick, excluindo I/O.\n");
    printf("Estrutura do autômato: %zu bytes\n", sizeof(ac_automaton_t));
    printf("Limite de 8KB: %s\n", 
           sizeof(ac_automaton_t) <= 8192 ? "✅ RESPEITADO" : "❌ EXCEDIDO");
    
    return 0;
}
