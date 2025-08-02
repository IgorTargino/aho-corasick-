#ifndef AHO_APP_CONFIG_H
#define AHO_APP_CONFIG_H

#include "aho_corasick.h"

// Test scenario definitions
typedef enum {
    SCENARIO_MINIMAL,      // Minimal patterns for very constrained systems
    SCENARIO_EMBEDDED,     // Balanced for STM32F0 (8KB RAM)
    SCENARIO_DESKTOP,      // More patterns for PC testing
    SCENARIO_STRESS        // Maximum patterns for stress testing
} test_scenario_t;

// Configuration structure for different scenarios
typedef struct {
    test_scenario_t scenario;
    uint8_t pattern_count;
    uint16_t max_text_length;
    uint8_t max_pattern_length;
    const char* description;
    const char* patterns[AC_MAX_PATTERNS];
} scenario_config_t;

// Predefined test scenarios
extern const scenario_config_t g_test_scenarios[];
extern const int g_scenario_count;

// Get configuration for specific scenario
const scenario_config_t* get_scenario_config(test_scenario_t scenario);

// Get test text for specific scenario
const char* get_test_text_for_scenario(test_scenario_t scenario);

// Validation and estimation functions
bool validate_scenario_for_platform(test_scenario_t scenario, bool is_embedded);
size_t estimate_memory_usage(test_scenario_t scenario);

// Test patterns for different categories
extern const char* g_common_patterns[];
extern const char* g_embedded_patterns[];
extern const char* g_security_patterns[];
extern const char* g_protocol_patterns[];

// Test texts for different scenarios
extern const char* g_minimal_test_text;
extern const char* g_embedded_test_text;
extern const char* g_desktop_test_text;
extern const char* g_stress_test_text;

#endif // AHO_APP_CONFIG_H
