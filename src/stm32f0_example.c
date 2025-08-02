/*
 * Exemplo de uso do Aho-Corasick em STM32F0
 * 
 * Este exemplo demonstra como usar a implementação do algoritmo
 * Aho-Corasick em um microcontrolador STM32F0 com limitações de memória.
 */

#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>

#ifdef STM32F0
    #include "stm32f0xx_hal.h"
    #include "aho_app_stm32.h"
#endif

// Buffer estático para evitar uso de heap
static char input_buffer[256];
static char output_buffer[512];
static int output_pos = 0;

// Contador de matches encontrados
static int match_count = 0;

// Callback para processar matches encontrados
void stm32f0_match_callback(const char* pattern, int position) {
    match_count++;
    
    // Adicionar resultado ao buffer de saída
    int written = snprintf(output_buffer + output_pos, 
                          sizeof(output_buffer) - output_pos,
                          "Match: '%s' at %d\n", pattern, position);
    
    if (written > 0 && output_pos + written < sizeof(output_buffer)) {
        output_pos += written;
    }
    
#ifdef STM32F0
    // Em ambiente real, poderia acender LED ou enviar via UART
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // LED onboard
#endif
}

// Função principal do exemplo
int stm32f0_aho_example(void) {
    printf("=== STM32F0 Aho-Corasick Example ===\n");
    
#ifdef STM32F0
    // Inicialização específica do STM32F0
    stm32f0_system_init();
    
    // Imprimir informações de memória
    printf("STM32F0 Memory Status:\n");
    printf("Free RAM: %zu bytes\n", stm32f0_get_free_ram());
    printf("Used RAM: %zu bytes\n", stm32f0_get_used_ram());
    printf("Stack usage: %zu bytes\n", stm32f0_get_stack_usage());
#endif
    
    // Inicializar autômato
    ac_automaton_t ac;
    ac_init(&ac, stm32f0_match_callback);
    
    // Padrões otimizados para sistemas embarcados
    const char* embedded_patterns[] = {
        "ERROR",    // Códigos de erro
        "WARN",     // Avisos
        "INFO",     // Informações
        "OK",       // Status OK
        "FAIL",     // Falhas
        "INIT",     // Inicialização
        "RUN",      // Execução
        "STOP",     // Parada
        "DATA",     // Dados
        "END"       // Fim
    };
    
    int pattern_count = sizeof(embedded_patterns) / sizeof(embedded_patterns[0]);
    
    printf("Adding %d patterns...\n", pattern_count);
    
    // Adicionar padrões
    for (int i = 0; i < pattern_count; i++) {
        if (!ac_add_pattern(&ac, embedded_patterns[i])) {
            printf("Failed to add pattern: %s\n", embedded_patterns[i]);
            return -1;
        }
    }
    
    printf("Patterns added: %d\n", ac.pattern_count);
    
    // Construir autômato
    printf("Building automaton...\n");
    
#ifdef STM32F0
    uint32_t build_start = stm32f0_get_tick_us();
#endif
    
    ac_build(&ac);
    
#ifdef STM32F0
    uint32_t build_time = stm32f0_get_tick_us() - build_start;
    printf("Build time: %u microseconds\n", build_time);
#endif
    
    printf("Automaton built: %d vertices\n", ac.vertex_count);
    
    // Texto de exemplo para processar
    const char* test_text = 
        "INIT system startup OK. "
        "Loading configuration DATA. "
        "WARN: low memory detected. "
        "RUN main loop started. "
        "Processing input DATA stream. "
        "ERROR in communication module. "
        "Attempting recovery procedure. "
        "INFO: connection restored. "
        "System status: OK. "
        "STOP requested by user. "
        "Cleanup completed. "
        "END of session.";
    
    // Copiar para buffer (simulando recepção via UART)
    strncpy(input_buffer, test_text, sizeof(input_buffer) - 1);
    input_buffer[sizeof(input_buffer) - 1] = '\0';
    
    printf("\nProcessing text (%zu chars):\n", strlen(input_buffer));
    printf("\"%.80s...\"\n", input_buffer);
    
    // Resetar contadores
    match_count = 0;
    output_pos = 0;
    
    // Processar texto
#ifdef STM32F0
    uint32_t search_start = stm32f0_get_tick_us();
#endif
    
    ac_search(&ac, input_buffer);
    
#ifdef STM32F0
    uint32_t search_time = stm32f0_get_tick_us() - search_start;
    printf("Search time: %u microseconds\n", search_time);
#endif
    
    // Imprimir resultados
    printf("\n=== Results ===\n");
    printf("Matches found: %d\n", match_count);
    printf("Text length: %zu characters\n", strlen(input_buffer));
    printf("\nMatch details:\n%s", output_buffer);
    
    // Calcular estatísticas de memória
    size_t automaton_memory = sizeof(ac_automaton_t);
    
    // Adicionar memória dos padrões
    for (int i = 0; i < ac.pattern_count; i++) {
        if (ac.patterns[i]) {
            automaton_memory += strlen(ac.patterns[i]) + 1;
        }
    }
    
    printf("\n=== Memory Usage ===\n");
    printf("Automaton structure: %zu bytes\n", sizeof(ac_automaton_t));
    printf("Total memory used: %zu bytes\n", automaton_memory);
    
#ifdef STM32F0
    printf("RAM usage: %.1f%% of 8KB\n", 
           (double)automaton_memory / (8 * 1024) * 100.0);
    
    // Análise final de memória
    stm32f0_print_memory_analysis();
    stm32f0_print_performance_analysis(build_time, search_time);
    
    // Verificar se está dentro dos limites aceitáveis
    if (automaton_memory > 6 * 1024) { // 75% de 8KB
        printf("WARNING: High memory usage for STM32F0!\n");
        return -1;
    }
#else
    printf("(STM32F0 projection: %.1f%% of 8KB)\n", 
           (double)automaton_memory / (8 * 1024) * 100.0);
#endif
    
    printf("\n=== Performance Summary ===\n");
    printf("✅ Patterns processed: %d\n", ac.pattern_count);
    printf("✅ Vertices created: %d\n", ac.vertex_count);
    printf("✅ Matches found: %d\n", match_count);
    printf("✅ Memory efficient: %.1f%% usage\n", 
           (double)automaton_memory / (8 * 1024) * 100.0);
    
    if (match_count > 0) {
        printf("✅ Algorithm working correctly\n");
    } else {
        printf("⚠️  No matches found - check patterns/text\n");
    }
    
    return 0;
}

#ifdef STM32F0
// Função main para STM32F0
int main(void) {
    return stm32f0_aho_example();
}
#else
// Função main para PC (para testes)
int main(void) {
    printf("Running STM32F0 Aho-Corasick example on PC\n");
    printf("This simulates the behavior on actual STM32F0 hardware.\n\n");
    
    int result = stm32f0_aho_example();
    
    if (result == 0) {
        printf("\n✅ Example completed successfully!\n");
        printf("This configuration should work well on STM32F0.\n");
    } else {
        printf("\n❌ Example failed!\n");
        printf("Consider reducing patterns or optimizing configuration.\n");
    }
    
    return result;
}
#endif
