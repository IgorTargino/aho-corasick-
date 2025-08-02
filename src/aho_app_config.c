#include "aho_app_config.h"
#include <string.h>
#include <stdbool.h>

// Common patterns for general use
const char* g_common_patterns[] = {
    "error", "warn", "info", "debug", "fatal",
    "ok", "fail", "pass", "test", "data"
};

// Embedded-specific patterns (shorter, more relevant)
const char* g_embedded_patterns[] = {
    "err", "ok", "init", "run", "stop",
    "tx", "rx", "cmd", "ack", "nak"
};

// Security-related patterns
const char* g_security_patterns[] = {
    "attack", "virus", "malware", "breach", "hack",
    "exploit", "threat", "secure", "auth", "login"
};

// Protocol-related patterns
const char* g_protocol_patterns[] = {
    "http", "tcp", "udp", "ip", "dns",
    "get", "post", "put", "delete", "head"
};

// Test scenarios configuration
const scenario_config_t g_test_scenarios[] = {
    // SCENARIO_MINIMAL - For very constrained systems
    {
        .scenario = SCENARIO_MINIMAL,
        .pattern_count = 5,
        .max_text_length = 64,
        .max_pattern_length = 4,
        .description = "Minimal scenario for highly constrained systems",
        .patterns = {"err", "ok", "run", "end", "tx"}
    },
    
    // SCENARIO_EMBEDDED - Balanced for STM32F0
    {
        .scenario = SCENARIO_EMBEDDED,
        .pattern_count = 10,
        .max_text_length = 256,
        .max_pattern_length = 8,
        .description = "Embedded scenario optimized for STM32F0 (8KB RAM)",
        .patterns = {
            "error", "warn", "info", "ok", "fail",
            "init", "run", "stop", "data", "end"
        }
    },
    
    // SCENARIO_DESKTOP - More patterns for PC
    {
        .scenario = SCENARIO_DESKTOP,
        .pattern_count = 20,
        .max_text_length = 1024,
        .max_pattern_length = 16,
        .description = "Desktop scenario with moderate pattern set",
        .patterns = {
            "algorithm", "pattern", "search", "matching", "automaton",
            "corasick", "finite", "state", "machine", "text",
            "string", "processing", "performance", "memory", "optimization",
            "embedded", "microcontroller", "stm32", "real", "time"
        }
    },
    
    // SCENARIO_STRESS - Maximum patterns for stress testing
    {
        .scenario = SCENARIO_STRESS,
        .pattern_count = 30,
        .max_text_length = 2048,
        .max_pattern_length = 32,
        .description = "Stress test scenario with maximum patterns",
        .patterns = {
            "aho", "corasick", "algorithm", "pattern", "matching",
            "string", "search", "automaton", "finite", "state",
            "machine", "trie", "suffix", "link", "failure",
            "transition", "vertex", "edge", "node", "tree",
            "performance", "optimization", "memory", "efficient", "fast",
            "embedded", "microcontroller", "stm32", "cortex", "arm"
        }
    }
};

const int g_scenario_count = sizeof(g_test_scenarios) / sizeof(g_test_scenarios[0]);

// Test texts for different scenarios
const char* g_minimal_test_text = 
    "err in run. ok end tx data.";

const char* g_embedded_test_text = 
    "System init ok. Starting main run loop. "
    "Processing data packets. warn: low memory. "
    "error in communication module. "
    "Attempting recovery. info: system stable. "
    "Data processing complete. end of cycle.";

const char* g_desktop_test_text = 
    "The Aho-Corasick algorithm is a powerful string matching algorithm "
    "that efficiently searches for multiple patterns simultaneously. "
    "It constructs a finite state machine that resembles a trie with "
    "additional failure links for fast transitions. This automaton "
    "enables real-time text processing in embedded microcontroller "
    "systems like the STM32 family. The algorithm's performance "
    "characteristics make it suitable for pattern matching applications "
    "requiring optimal memory usage and processing speed.";

const char* g_stress_test_text = 
    "Advanced Aho-Corasick algorithm implementation for embedded systems. "
    "This comprehensive string matching solution utilizes a sophisticated "
    "finite state machine architecture with optimized trie construction "
    "and efficient failure link computation. The automaton processes "
    "multiple patterns simultaneously, enabling high-performance text "
    "analysis in resource-constrained environments. Microcontroller "
    "platforms such as STM32 and other ARM Cortex-M devices benefit "
    "from the algorithm's memory-efficient design and fast execution "
    "characteristics. Pattern matching applications in embedded systems "
    "require careful optimization of both memory usage and processing "
    "time to achieve real-time performance goals. The implementation "
    "balances algorithmic sophistication with practical constraints "
    "imposed by limited RAM and processing capabilities of target "
    "hardware platforms.";

// Function to get configuration for specific scenario
const scenario_config_t* get_scenario_config(test_scenario_t scenario) {
    for (int i = 0; i < g_scenario_count; i++) {
        if (g_test_scenarios[i].scenario == scenario) {
            return &g_test_scenarios[i];
        }
    }
    return NULL; // Scenario not found
}

// Helper function to get test text for scenario
const char* get_test_text_for_scenario(test_scenario_t scenario) {
    switch (scenario) {
        case SCENARIO_MINIMAL:
            return g_minimal_test_text;
        case SCENARIO_EMBEDDED:
            return g_embedded_test_text;
        case SCENARIO_DESKTOP:
            return g_desktop_test_text;
        case SCENARIO_STRESS:
            return g_stress_test_text;
        default:
            return g_embedded_test_text;
    }
}

// Function to validate scenario against platform constraints
bool validate_scenario_for_platform(test_scenario_t scenario, bool is_embedded) {
    const scenario_config_t* config = get_scenario_config(scenario);
    if (!config) return false;
    
    if (is_embedded) {
        // STM32F0 constraints
        if (config->pattern_count > 15) return false;
        if (config->max_text_length > 512) return false;
        if (config->max_pattern_length > 16) return false;
    }
    
    return true;
}

// Function to estimate memory usage for scenario
size_t estimate_memory_usage(test_scenario_t scenario) {
    const scenario_config_t* config = get_scenario_config(scenario);
    if (!config) return 0;
    
    size_t estimated_size = 0;
    
    // Base automaton structure
    estimated_size += sizeof(ac_automaton_t);
    
    // Estimate vertex memory (rough calculation)
    // Assume average 3 characters per pattern creates vertices
    estimated_size += config->pattern_count * 3 * sizeof(ac_vertex_t);
    
    // Pattern storage
    for (int i = 0; i < config->pattern_count; i++) {
        if (config->patterns[i]) {
            estimated_size += strlen(config->patterns[i]) + 1;
        }
    }
    
    return estimated_size;
}
