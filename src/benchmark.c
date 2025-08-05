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
    
    // Para medições futuras, podemos calcular memória adicional se necessário
    (void)ac; // Suprime warning de parâmetro não usado
    
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
    // Medição de tempo
    start_performance_timing();
    ac_build(automaton);
    end_performance_timing();
    
    // Coleta estatísticas após construção
    g_perf.vertices_used = automaton->vertex_count;
    g_perf.patterns_loaded = automaton->pattern_count;
    g_perf.transitions_total = count_total_transitions(automaton);
    g_perf.automaton_memory_bytes = calculate_automaton_memory(automaton);
}

// Função para medição de busca
void measure_search(ac_automaton_t *automaton, const char* text) {
    g_match_count = 0;
    
    // Medição de tempo
    start_performance_timing();
    ac_search(automaton, text);
    end_performance_timing();
}

// Lista de palavras para teste
static const char* test_patterns[] = {
    "spam", "hack", "virus", "malware", "phishing",
    "scam", "fraud", "illegal", "crack", "trojan"
};

int main(void) {
    ac_automaton_t automaton;
    
    // Inicialização
    ac_init(&automaton, performance_callback);
    
    // Carregamento de padrões
    uint8_t num_patterns = sizeof(test_patterns) / sizeof(test_patterns[0]);
    
    for (uint8_t i = 0; i < num_patterns && i < AC_MAX_PATTERNS; i++) {
        ac_add_pattern(&automaton, test_patterns[i]);
    }
    
    // Medição da construção do autômato
    measure_construction(&automaton);
    
    // Texto grande para teste de performance
    const char* large_text = 
        "Este é um texto extenso para testar a performance do algoritmo Aho-Corasick. "
        "O texto contém várias palavras suspeitas como spam, malware, virus e trojan. "
        "Também inclui outras palavras como hack, phishing, scam, fraud, illegal e crack. "
        "O objetivo é medir o tempo de execução e uso de memória em um texto significativo. "
        "Este texto deve ser longo o suficiente para proporcionar medições confiáveis de performance. "
        "Repetindo algumas palavras: spam aparece novamente, assim como malware e virus. "
        "O algoritmo deve encontrar todos os padrões de forma eficiente. "
        "Texto adicional para aumentar o tamanho: esta seção contém mais conteúdo para "
        "garantir que temos dados suficientes para uma medição precisa de performance. "
        "Palavras adicionais incluem trojan, hack, phishing e outras variações. "
        "O sistema deve processar tudo rapidamente mantendo baixo uso de memória. "
        "Final do texto de teste com mais algumas ocorrências: scam, fraud, illegal, crack.";
    
    // Medição da busca
    measure_search(&automaton, large_text);
    
    // Relatório final de performance
    printf("=== RELATÓRIO DE PERFORMANCE ===\n");
    printf("Construção do autômato:\n");
    printf("  Tempo: %.3f µs (%.6f ms)\n", 
           g_perf.elapsed_microseconds, g_perf.elapsed_microseconds / 1000.0);
    printf("  Memória: %zu bytes (%.2f KB)\n", 
           g_perf.automaton_memory_bytes, g_perf.automaton_memory_bytes / 1024.0);
    printf("  Vértices: %u\n", g_perf.vertices_used);
    printf("  Padrões: %u\n", g_perf.patterns_loaded);
    printf("  Transições: %u\n", g_perf.transitions_total);
    
    printf("\nBusca no texto:\n");
    printf("  Tempo: %.3f µs (%.6f ms)\n", 
           g_perf.elapsed_microseconds, g_perf.elapsed_microseconds / 1000.0);
    printf("  Texto: %zu caracteres\n", strlen(large_text));
    printf("  Matches: %u\n", g_match_count);
    printf("  Taxa: %.2f chars/µs\n", 
           strlen(large_text) / g_perf.elapsed_microseconds);
    
    printf("\nLimite de memória 8KB: %s\n", 
           g_perf.automaton_memory_bytes <= 8192 ? "✅ RESPEITADO" : "❌ EXCEDIDO");
    
    return 0;
}
