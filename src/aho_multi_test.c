#include "aho_corasick.h"
#include "aho_app.h"
#include "aho_app_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Multi-scenario test results structure
typedef struct {
    test_scenario_t scenario;
    uint32_t build_time_us;
    uint32_t search_time_us;
    uint32_t total_time_us;
    size_t memory_usage;
    uint32_t matches_found;
    uint32_t pattern_count;
    uint32_t vertex_count;
    bool success;
} scenario_result_t;

// Global results storage
static scenario_result_t g_results[4]; // Max 4 scenarios
static int g_result_count = 0;
static uint32_t g_match_count = 0;

// Callback for counting matches
void multi_test_match_callback(const char* pattern, int position) {
    g_match_count++;
    // Don't print individual matches in multi-test mode
}

// Simple timer implementation
static uint32_t get_time_us(void) {
    return (uint32_t)(clock() * 1000000 / CLOCKS_PER_SEC);
}

// Run a single scenario test
static bool run_scenario_test(test_scenario_t scenario) {
    const scenario_config_t* config = get_scenario_config(scenario);
    if (!config) {
        printf("ERROR: Invalid scenario %d\n", scenario);
        return false;
    }
    
    printf("\n=== Testing Scenario: %s ===\n", config->description);
    printf("Patterns: %d, Max text: %d chars\n", 
           config->pattern_count, config->max_text_length);
    
    // Initialize automaton
    ac_automaton_t ac;
    ac_init(&ac, multi_test_match_callback);
    
    // Measure build time
    g_match_count = 0;
    uint32_t build_start = get_time_us();
    
    // Add patterns
    for (int i = 0; i < config->pattern_count; i++) {
        if (!ac_add_pattern(&ac, config->patterns[i])) {
            printf("ERROR: Failed to add pattern '%s'\n", config->patterns[i]);
            return false;
        }
    }
    
    // Build automaton
    ac_build(&ac);
    uint32_t build_end = get_time_us();
    
    // Get test text
    const char* test_text = get_test_text_for_scenario(scenario);
    if (!test_text) {
        printf("ERROR: No test text for scenario\n");
        return false;
    }
    
    // Truncate text if needed
    static char text_buffer[2048];
    size_t text_len = strlen(test_text);
    if (text_len > config->max_text_length) {
        strncpy(text_buffer, test_text, config->max_text_length);
        text_buffer[config->max_text_length] = '\0';
        test_text = text_buffer;
        text_len = config->max_text_length;
    }
    
    // Measure search time
    uint32_t search_start = get_time_us();
    ac_search(&ac, test_text);
    uint32_t search_end = get_time_us();
    
    // Calculate memory usage (simplified)
    size_t memory_usage = sizeof(ac_automaton_t);
    for (int i = 0; i < ac.pattern_count; i++) {
        if (ac.patterns[i]) {
            memory_usage += strlen(ac.patterns[i]) + 1;
        }
    }
    
    // Store results
    scenario_result_t* result = &g_results[g_result_count++];
    result->scenario = scenario;
    result->build_time_us = build_end - build_start;
    result->search_time_us = search_end - search_start;
    result->total_time_us = result->build_time_us + result->search_time_us;
    result->memory_usage = memory_usage;
    result->matches_found = g_match_count;
    result->pattern_count = ac.pattern_count;
    result->vertex_count = ac.vertex_count;
    result->success = true;
    
    // Print immediate results
    printf("Results:\n");
    printf("  Build time: %u us\n", result->build_time_us);
    printf("  Search time: %u us (text: %zu chars)\n", result->search_time_us, text_len);
    printf("  Memory usage: %zu bytes\n", result->memory_usage);
    printf("  Patterns: %d, Vertices: %d\n", result->pattern_count, result->vertex_count);
    printf("  Matches found: %d\n", result->matches_found);
    
    return true;
}

// Print comparison table
static void print_comparison_table(void) {
    printf("\n================================================================================\n");
    printf("SCENARIO COMPARISON TABLE\n");
    printf("================================================================================\n");
    
    printf("%-12s %8s %10s %10s %10s %8s %8s %8s\n",
           "Scenario", "Patterns", "Build(us)", "Search(us)", "Total(us)", 
           "Memory", "Vertices", "Matches");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < g_result_count; i++) {
        scenario_result_t* r = &g_results[i];
        const char* scenario_name;
        
        switch (r->scenario) {
            case SCENARIO_MINIMAL: scenario_name = "MINIMAL"; break;
            case SCENARIO_EMBEDDED: scenario_name = "EMBEDDED"; break;
            case SCENARIO_DESKTOP: scenario_name = "DESKTOP"; break;
            case SCENARIO_STRESS: scenario_name = "STRESS"; break;
            default: scenario_name = "UNKNOWN"; break;
        }
        
        printf("%-12s %8d %10u %10u %10u %7zuB %8d %8d\n",
               scenario_name, r->pattern_count, r->build_time_us, 
               r->search_time_us, r->total_time_us, r->memory_usage,
               r->vertex_count, r->matches_found);
    }
    
    printf("================================================================================\n");
}

// Analyze results for STM32F0 suitability
static void analyze_stm32_suitability(void) {
    printf("\n=== STM32F0 SUITABILITY ANALYSIS ===\n");
    
    const size_t STM32F0_RAM = 8 * 1024; // 8KB
    const uint32_t STM32F0_MAX_BUILD_TIME = 50000; // 50ms at 48MHz
    const uint32_t STM32F0_MAX_SEARCH_TIME = 10000; // 10ms per search
    
    printf("STM32F0 Constraints:\n");
    printf("  RAM: %zu bytes\n", STM32F0_RAM);
    printf("  Max build time: %u us\n", STM32F0_MAX_BUILD_TIME);
    printf("  Max search time: %u us\n", STM32F0_MAX_SEARCH_TIME);
    printf("\n");
    
    printf("Scenario Recommendations:\n");
    printf("%-12s %-10s %-15s %-15s %s\n", 
           "Scenario", "Memory", "Build Time", "Search Time", "Recommendation");
    printf("----------------------------------------------------------------------\n");
    
    for (int i = 0; i < g_result_count; i++) {
        scenario_result_t* r = &g_results[i];
        const char* scenario_name;
        
        switch (r->scenario) {
            case SCENARIO_MINIMAL: scenario_name = "MINIMAL"; break;
            case SCENARIO_EMBEDDED: scenario_name = "EMBEDDED"; break;
            case SCENARIO_DESKTOP: scenario_name = "DESKTOP"; break;
            case SCENARIO_STRESS: scenario_name = "STRESS"; break;
            default: scenario_name = "UNKNOWN"; break;
        }
        
        bool memory_ok = r->memory_usage <= STM32F0_RAM * 0.6; // 60% threshold
        bool build_ok = r->build_time_us <= STM32F0_MAX_BUILD_TIME;
        bool search_ok = r->search_time_us <= STM32F0_MAX_SEARCH_TIME;
        
        const char* memory_status = memory_ok ? "✅ OK" : "❌ HIGH";
        const char* build_status = build_ok ? "✅ OK" : "❌ SLOW";
        const char* search_status = search_ok ? "✅ OK" : "❌ SLOW";
        
        const char* recommendation;
        if (memory_ok && build_ok && search_ok) {
            recommendation = "✅ RECOMMENDED";
        } else if (memory_ok && (build_ok || search_ok)) {
            recommendation = "⚠️ CONSIDER";
        } else {
            recommendation = "❌ NOT SUITABLE";
        }
        
        printf("%-12s %-10s %-15s %-15s %s\n",
               scenario_name, memory_status, build_status, search_status, recommendation);
    }
    
    printf("\n");
    
    // Find best scenario for STM32F0
    int best_scenario = -1;
    for (int i = 0; i < g_result_count; i++) {
        scenario_result_t* r = &g_results[i];
        if (r->memory_usage <= STM32F0_RAM * 0.6 && 
            r->build_time_us <= STM32F0_MAX_BUILD_TIME &&
            r->search_time_us <= STM32F0_MAX_SEARCH_TIME) {
            if (best_scenario == -1 || r->pattern_count > g_results[best_scenario].pattern_count) {
                best_scenario = i;
            }
        }
    }
    
    if (best_scenario >= 0) {
        scenario_result_t* best = &g_results[best_scenario];
        printf("RECOMMENDED CONFIGURATION FOR STM32F0:\n");
        printf("  Scenario: %s\n", 
               best->scenario == SCENARIO_MINIMAL ? "MINIMAL" :
               best->scenario == SCENARIO_EMBEDDED ? "EMBEDDED" : "OTHER");
        printf("  Patterns: %d\n", best->pattern_count);
        printf("  Memory usage: %zu bytes (%.1f%% of RAM)\n", 
               best->memory_usage, (double)best->memory_usage / STM32F0_RAM * 100.0);
        printf("  Expected performance: Build %u us, Search %u us\n",
               best->build_time_us, best->search_time_us);
    } else {
        printf("WARNING: No scenario fully suitable for STM32F0.\n");
        printf("Consider reducing pattern count or optimizing algorithm.\n");
    }
}

// Main multi-scenario test function
int main(void) {
    printf("Aho-Corasick Multi-Scenario Performance Test\n");
    printf("Platform: PC (Linux)\n");
    printf("Purpose: Analyze suitability for STM32F0 (8KB RAM)\n");
    
    // Test scenarios in order of complexity
    test_scenario_t scenarios[] = {
        SCENARIO_MINIMAL,
        SCENARIO_EMBEDDED,
        SCENARIO_DESKTOP,
        SCENARIO_STRESS
    };
    
    int scenario_count = sizeof(scenarios) / sizeof(scenarios[0]);
    
    // Run all scenarios
    for (int i = 0; i < scenario_count; i++) {
        if (!run_scenario_test(scenarios[i])) {
            printf("WARNING: Scenario test failed - stopping here\n");
            break;
        }
        
        // Stop if we exceed reasonable limits for comparison
        if (g_results[g_result_count - 1].memory_usage > 16 * 1024) {
            printf("NOTE: Stopping tests - memory usage too high for embedded\n");
            break;
        }
    }
    
    // Print results
    print_comparison_table();
    analyze_stm32_suitability();
    
    printf("\nTest completed successfully!\n");
    printf("Results show performance characteristics for embedded deployment.\n");
    
    return 0;
}
