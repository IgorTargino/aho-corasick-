#ifndef AHO_APP_H
#define AHO_APP_H

#include <stdint.h>
#include <stddef.h>

// Platform-specific definitions
#ifdef STM32F0
    #define PLATFORM_NAME "STM32F0"
    #define IS_EMBEDDED 1
    #define TARGET_RAM_SIZE (8 * 1024)     // 8KB
    #define MAX_TEXT_BUFFER 256
    #define MAX_PATTERNS_EMBEDDED 10
#else
    #define PLATFORM_NAME "PC"
    #define IS_EMBEDDED 0
    #define TARGET_RAM_SIZE (1024 * 1024)  // 1MB (arbitrary for PC)
    #define MAX_TEXT_BUFFER 4096
    #define MAX_PATTERNS_PC 25
#endif

// Memory measurement structure
typedef struct {
    size_t total_ram;
    size_t available_ram;
    size_t automaton_size;
    size_t stack_usage;
    size_t heap_usage;
} memory_stats_t;

// Performance measurement structure
typedef struct {
    uint32_t build_time_us;
    uint32_t search_time_us;
    uint32_t total_time_us;
    uint32_t matches_found;
    uint32_t text_length;
} performance_stats_t;

// Application configuration structure
typedef struct {
    uint8_t max_patterns;
    uint16_t max_text_length;
    size_t max_pattern_length;
    const char* test_patterns[40]; // Max possible patterns
    uint8_t pattern_count;
} app_config_t;

// Public function declarations
int aho_app_run(void);
void on_match_found(const char* pattern, int position);

// Platform-specific timer functions
uint32_t get_timestamp_us(void);
void init_platform_timer(void);
size_t get_available_memory(void);

// Configuration and measurement functions
app_config_t get_platform_config(void);
void measure_memory_before(memory_stats_t* stats);
void measure_memory_after(memory_stats_t* stats, const void* ac);
char* generate_test_text(const app_config_t* config);

#endif // AHO_APP_H
