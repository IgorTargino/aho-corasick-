#include "aho_corasick.h"
#include "aho_app.h"
#include "aho_app_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef STM32F0
    #define _POSIX_C_SOURCE 199309L
    #include <time.h>
#else
    #include "aho_app_stm32.h"
#endif

// Platform detection
#ifdef STM32F0
    #include "stm32f0xx.h"
    #define PLATFORM_NAME "STM32F0"
    #define IS_EMBEDDED 1
#else
    #define PLATFORM_NAME "PC"
    #define IS_EMBEDDED 0
#endif

// Global variables for measurements
static memory_stats_t g_memory_stats = {0};
static performance_stats_t g_perf_stats = {0};
static uint32_t g_match_count = 0;

// Platform-specific timer functions
#ifdef STM32F0
uint32_t get_timestamp_us(void) {
    // Use SysTick or TIM for STM32F0
    return HAL_GetTick() * 1000; // Convert ms to us (simplified)
}

void init_platform_timer(void) {
    // Initialize timer if needed
}

size_t get_available_memory(void) {
    // For STM32F0, calculate available RAM
    extern uint32_t _end;
    extern uint32_t _estack;
    return (size_t)(&_estack - &_end);
}

#else // PC Platform
uint32_t get_timestamp_us(void) {
    // Simple implementation using clock()
    return (uint32_t)(clock() * 1000000 / CLOCKS_PER_SEC);
}

void init_platform_timer(void) {
    // No initialization needed for PC
}

size_t get_available_memory(void) {
    // For PC, return a large value (simplified)
    return 1024 * 1024; // 1MB
}
#endif

// Configuration for different platforms
app_config_t get_platform_config(void) {
    app_config_t config = {0};
    
    if (IS_EMBEDDED) {
        // STM32F0 configuration - Use embedded scenario
        const scenario_config_t* scenario = get_scenario_config(SCENARIO_EMBEDDED);
        if (scenario) {
            config.max_patterns = scenario->pattern_count;
            config.max_text_length = scenario->max_text_length;
            config.max_pattern_length = scenario->max_pattern_length;
            config.pattern_count = scenario->pattern_count;
            
            // Copy patterns
            for (int i = 0; i < scenario->pattern_count && i < 40; i++) {
                config.test_patterns[i] = scenario->patterns[i];
            }
        }
    } else {
        // PC configuration - Use desktop scenario
        const scenario_config_t* scenario = get_scenario_config(SCENARIO_DESKTOP);
        if (scenario) {
            config.max_patterns = scenario->pattern_count;
            config.max_text_length = scenario->max_text_length;
            config.max_pattern_length = scenario->max_pattern_length;
            config.pattern_count = scenario->pattern_count;
            
            // Copy patterns
            for (int i = 0; i < scenario->pattern_count && i < 40; i++) {
                config.test_patterns[i] = scenario->patterns[i];
            }
        }
    }
    
    return config;
}

// Callback function for pattern matches
void on_match_found(const char* pattern, int position) {
    g_match_count++;
    printf("Match: '%s' at position %d\n", pattern, position);
}

// Memory measurement functions
void measure_memory_before(memory_stats_t* stats) {
    stats->available_ram = get_available_memory();
    
    if (IS_EMBEDDED) {
        stats->total_ram = 8 * 1024; // 8KB for STM32F0
    } else {
        stats->total_ram = stats->available_ram;
    }
}

void measure_memory_after(memory_stats_t* stats, const void* ac_ptr) {
    const ac_automaton_t* ac = (const ac_automaton_t*)ac_ptr;
    // Calculate automaton memory usage
    stats->automaton_size = sizeof(ac_automaton_t);
    
    // Add pattern storage (simplified calculation)
    for (int i = 0; i < ac->pattern_count; i++) {
        if (ac->patterns[i]) {
            stats->automaton_size += strlen(ac->patterns[i]) + 1;
        }
    }
    
    stats->heap_usage = stats->automaton_size;
    
    printf("\n=== Memory Analysis ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    printf("Total RAM: %zu bytes\n", stats->total_ram);
    printf("Available RAM: %zu bytes\n", stats->available_ram);
    printf("Automaton size: %zu bytes\n", stats->automaton_size);
    printf("Memory usage: %.2f%%\n", 
           (double)stats->automaton_size / stats->total_ram * 100.0);
    
    if (IS_EMBEDDED) {
        if (stats->automaton_size > stats->total_ram * 0.8) {
            printf("WARNING: High memory usage for embedded system!\n");
        }
    }
}

// Performance measurement functions
static void measure_build_performance(ac_automaton_t* ac, const app_config_t* config) {
    uint32_t start_time = get_timestamp_us();
    
    // Add patterns
    for (int i = 0; i < config->pattern_count; i++) {
        if (!ac_add_pattern(ac, config->test_patterns[i])) {
            printf("Failed to add pattern: %s\n", config->test_patterns[i]);
            break;
        }
    }
    
    // Build automaton
    ac_build(ac);
    
    uint32_t end_time = get_timestamp_us();
    g_perf_stats.build_time_us = end_time - start_time;
    
    printf("\n=== Build Performance ===\n");
    printf("Patterns added: %d\n", ac->pattern_count);
    printf("Vertices created: %d\n", ac->vertex_count);
    printf("Build time: %u microseconds\n", g_perf_stats.build_time_us);
}

static void measure_search_performance(ac_automaton_t* ac, const char* text) {
    g_match_count = 0;
    g_perf_stats.text_length = strlen(text);
    
    uint32_t start_time = get_timestamp_us();
    ac_search(ac, text);
    uint32_t end_time = get_timestamp_us();
    
    g_perf_stats.search_time_us = end_time - start_time;
    g_perf_stats.matches_found = g_match_count;
    
    printf("\n=== Search Performance ===\n");
    printf("Text length: %u characters\n", g_perf_stats.text_length);
    printf("Matches found: %u\n", g_perf_stats.matches_found);
    printf("Search time: %u microseconds\n", g_perf_stats.search_time_us);
    
    if (g_perf_stats.text_length > 0) {
        printf("Throughput: %.2f chars/ms\n", 
               (double)g_perf_stats.text_length / (g_perf_stats.search_time_us / 1000.0));
    }
}

// Generate test text based on platform
char* generate_test_text(const app_config_t* config) {
    static char text_buffer[4096]; // Static buffer to avoid heap allocation
    
    const char* source_text;
    
    if (IS_EMBEDDED) {
        source_text = get_test_text_for_scenario(SCENARIO_EMBEDDED);
    } else {
        source_text = get_test_text_for_scenario(SCENARIO_DESKTOP);
    }
    
    // Copy text, respecting buffer limits
    size_t max_len = (config->max_text_length < sizeof(text_buffer) - 1) ? 
                     config->max_text_length : sizeof(text_buffer) - 1;
    
    strncpy(text_buffer, source_text, max_len);
    text_buffer[max_len] = '\0';
    
    return text_buffer;
}

// Main application function
int aho_app_run(void) {
    printf("\n=== Aho-Corasick Application ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    
    // Initialize platform-specific components
    init_platform_timer();
    
    // Get platform configuration
    app_config_t config = get_platform_config();
    
    // Initialize automaton
    ac_automaton_t ac;
    ac_init(&ac, on_match_found);
    
    // Measure memory before
    measure_memory_before(&g_memory_stats);
    
    // Measure build performance
    measure_build_performance(&ac, &config);
    
    // Measure memory after build
    measure_memory_after(&g_memory_stats, &ac);
    
    // Generate test text
    char* test_text = generate_test_text(&config);
    printf("\nTest text: \"%.100s%s\"\n", test_text, 
           strlen(test_text) > 100 ? "..." : "");
    
    // Measure search performance
    measure_search_performance(&ac, test_text);
    
    // Calculate total time
    g_perf_stats.total_time_us = g_perf_stats.build_time_us + g_perf_stats.search_time_us;
    
    // Print final summary
    printf("\n=== Final Summary ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    printf("Memory efficiency: %.2f%% of total RAM\n", 
           (double)g_memory_stats.automaton_size / g_memory_stats.total_ram * 100.0);
    printf("Total execution time: %u microseconds\n", g_perf_stats.total_time_us);
    printf("Patterns/Memory ratio: %.2f patterns/KB\n",
           (double)ac.pattern_count / (g_memory_stats.automaton_size / 1024.0));
    
    // Platform-specific recommendations
    if (IS_EMBEDDED) {
        printf("\n=== STM32F0 Recommendations ===\n");
        if (g_memory_stats.automaton_size > g_memory_stats.total_ram * 0.5) {
            printf("- Consider reducing pattern count or length\n");
            printf("- Current usage is high for embedded system\n");
        } else {
            printf("- Memory usage is acceptable for STM32F0\n");
            printf("- Could potentially add %d more small patterns\n", 
                   (int)((g_memory_stats.total_ram * 0.8 - g_memory_stats.automaton_size) / 20));
        }
    } else {
        printf("\n=== PC Performance Notes ===\n");
        printf("- Excellent performance for desktop application\n");
        printf("- Could handle significantly more patterns\n");
    }
    
    return 0;
}

// Main function
int main(void) {
    #ifdef STM32F0
    // STM32F0 specific initialization would go here
    // HAL_Init();
    // SystemClock_Config();
    #endif
    
    return aho_app_run();
}
