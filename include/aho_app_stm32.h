#ifndef AHO_APP_STM32_H
#define AHO_APP_STM32_H

#ifdef STM32F0

#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"

// STM32F0-specific memory layout
#define STM32F0_RAM_START    0x20000000
#define STM32F0_RAM_SIZE     (8 * 1024)    // 8KB
#define STM32F0_STACK_SIZE   (1 * 1024)    // 1KB for stack

// Symbols from linker script
extern uint32_t _end;      // End of used RAM
extern uint32_t _estack;   // End of RAM

// STM32F0-specific timer setup
void stm32f0_timer_init(void);
uint32_t stm32f0_get_tick_us(void);

// Memory measurement functions for STM32F0
size_t stm32f0_get_free_ram(void);
size_t stm32f0_get_used_ram(void);
size_t stm32f0_get_stack_usage(void);

// UART functions for output (if needed)
void stm32f0_uart_init(void);
void stm32f0_uart_printf(const char* format, ...);

// System initialization for STM32F0
void stm32f0_system_init(void);

// Performance counter using DWT (Data Watchpoint and Trace)
static inline void stm32f0_dwt_enable(void) {
    // Enable DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Reset cycle counter
    DWT->CYCCNT = 0;
    // Enable cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t stm32f0_dwt_get_cycles(void) {
    return DWT->CYCCNT;
}

static inline uint32_t stm32f0_cycles_to_us(uint32_t cycles) {
    // Assuming 48MHz system clock for STM32F0
    return cycles / 48;
}

// Memory allocation tracking for embedded systems
typedef struct {
    void* ptr;
    size_t size;
    const char* file;
    int line;
} alloc_info_t;

#define MAX_ALLOCS 16
extern alloc_info_t g_alloc_table[MAX_ALLOCS];
extern int g_alloc_count;

// Custom malloc/free with tracking (for debugging)
void* stm32f0_malloc_tracked(size_t size, const char* file, int line);
void stm32f0_free_tracked(void* ptr);

#define TRACKED_MALLOC(size) stm32f0_malloc_tracked(size, __FILE__, __LINE__)
#define TRACKED_FREE(ptr) stm32f0_free_tracked(ptr)

// Memory constraints for STM32F0
#define STM32F0_MAX_PATTERNS 8
#define STM32F0_MAX_TEXT_LENGTH 128
#define STM32F0_MAX_PATTERN_LENGTH 12

// Performance benchmarks thresholds
#define STM32F0_MAX_BUILD_TIME_US 50000    // 50ms
#define STM32F0_MAX_SEARCH_TIME_US 10000   // 10ms per 100 chars

#endif // STM32F0

#endif // AHO_APP_STM32_H
