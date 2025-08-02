#include "aho_app_stm32.h"

#ifdef STM32F0

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// Global allocation tracking
alloc_info_t g_alloc_table[MAX_ALLOCS] = {0};
int g_alloc_count = 0;

// STM32F0 System initialization
void stm32f0_system_init(void) {
    // Initialize HAL
    HAL_Init();
    
    // Configure system clock (this would be project-specific)
    // SystemClock_Config();
    
    // Initialize DWT for cycle counting
    stm32f0_dwt_enable();
    
    // Initialize UART for debugging (optional)
    // stm32f0_uart_init();
}

// Timer initialization using SysTick
void stm32f0_timer_init(void) {
    // SysTick is usually initialized by HAL_Init()
    // Additional timer setup if needed
}

// Get timestamp in microseconds
uint32_t stm32f0_get_tick_us(void) {
    // Use DWT cycle counter for higher precision
    uint32_t cycles = stm32f0_dwt_get_cycles();
    return stm32f0_cycles_to_us(cycles);
}

// Memory measurement functions
size_t stm32f0_get_free_ram(void) {
    // Calculate free RAM based on heap and stack pointers
    extern uint32_t _end;
    extern uint32_t _estack;
    
    uint32_t stack_ptr;
    __asm volatile ("mov %0, sp" : "=r" (stack_ptr));
    
    uint32_t heap_end = (uint32_t)&_end;
    
    if (stack_ptr > heap_end) {
        return stack_ptr - heap_end;
    } else {
        return 0; // Stack overflow condition
    }
}

size_t stm32f0_get_used_ram(void) {
    return STM32F0_RAM_SIZE - stm32f0_get_free_ram();
}

size_t stm32f0_get_stack_usage(void) {
    extern uint32_t _estack;
    uint32_t stack_ptr;
    __asm volatile ("mov %0, sp" : "=r" (stack_ptr));
    
    return (uint32_t)&_estack - stack_ptr;
}

// Custom malloc with tracking
void* stm32f0_malloc_tracked(size_t size, const char* file, int line) {
    if (g_alloc_count >= MAX_ALLOCS) {
        return NULL; // Too many allocations
    }
    
    // In a real embedded system, you might use a static memory pool
    // For now, we'll simulate with a static buffer
    static uint8_t memory_pool[2048]; // 2KB pool
    static size_t pool_offset = 0;
    
    if (pool_offset + size > sizeof(memory_pool)) {
        return NULL; // Out of memory
    }
    
    void* ptr = &memory_pool[pool_offset];
    pool_offset += size;
    
    // Track allocation
    g_alloc_table[g_alloc_count].ptr = ptr;
    g_alloc_table[g_alloc_count].size = size;
    g_alloc_table[g_alloc_count].file = file;
    g_alloc_table[g_alloc_count].line = line;
    g_alloc_count++;
    
    return ptr;
}

void stm32f0_free_tracked(void* ptr) {
    // Find and remove from tracking table
    for (int i = 0; i < g_alloc_count; i++) {
        if (g_alloc_table[i].ptr == ptr) {
            // Shift remaining entries down
            for (int j = i; j < g_alloc_count - 1; j++) {
                g_alloc_table[j] = g_alloc_table[j + 1];
            }
            g_alloc_count--;
            break;
        }
    }
    
    // In a real system with a memory pool, you'd return the memory here
    // For this simulation, we don't actually free the memory
}

// UART functions (placeholder - would need actual UART configuration)
void stm32f0_uart_init(void) {
    // Initialize UART peripheral
    // This would be project-specific based on which UART is used
}

void stm32f0_uart_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Send via UART
    // HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    
    // For debugging, you might also use ITM (if available)
    // ITM_SendString(buffer);
}

// Memory analysis functions specific to embedded systems
void stm32f0_print_memory_analysis(void) {
    size_t free_ram = stm32f0_get_free_ram();
    size_t used_ram = stm32f0_get_used_ram();
    size_t stack_usage = stm32f0_get_stack_usage();
    
    printf("\n=== STM32F0 Memory Analysis ===\n");
    printf("Total RAM: %d bytes\n", STM32F0_RAM_SIZE);
    printf("Used RAM: %zu bytes (%.1f%%)\n", used_ram, 
           (double)used_ram / STM32F0_RAM_SIZE * 100.0);
    printf("Free RAM: %zu bytes (%.1f%%)\n", free_ram,
           (double)free_ram / STM32F0_RAM_SIZE * 100.0);
    printf("Stack usage: %zu bytes\n", stack_usage);
    
    printf("\nAllocation tracking:\n");
    printf("Active allocations: %d\n", g_alloc_count);
    for (int i = 0; i < g_alloc_count; i++) {
        printf("  [%d] %zu bytes at %s:%d\n", 
               i, g_alloc_table[i].size, 
               g_alloc_table[i].file, g_alloc_table[i].line);
    }
    
    // Memory safety warnings
    if (used_ram > STM32F0_RAM_SIZE * 0.8) {
        printf("WARNING: High memory usage (>80%%)!\n");
    }
    
    if (free_ram < 512) {
        printf("CRITICAL: Very low free memory (<512 bytes)!\n");
    }
    
    if (stack_usage > STM32F0_STACK_SIZE * 0.8) {
        printf("WARNING: High stack usage!\n");
    }
}

// Performance analysis for embedded systems
void stm32f0_print_performance_analysis(uint32_t build_time_us, uint32_t search_time_us) {
    printf("\n=== STM32F0 Performance Analysis ===\n");
    printf("Build time: %u us\n", build_time_us);
    printf("Search time: %u us\n", search_time_us);
    
    // Check against thresholds
    if (build_time_us > STM32F0_MAX_BUILD_TIME_US) {
        printf("WARNING: Build time exceeds threshold (%u us)!\n", STM32F0_MAX_BUILD_TIME_US);
    }
    
    if (search_time_us > STM32F0_MAX_SEARCH_TIME_US) {
        printf("WARNING: Search time exceeds threshold (%u us)!\n", STM32F0_MAX_SEARCH_TIME_US);
    }
    
    // Calculate throughput
    uint32_t total_time = build_time_us + search_time_us;
    printf("Total time: %u us\n", total_time);
    
    // Real-time performance metrics
    printf("CPU cycles used: %u\n", total_time * 48); // Assuming 48MHz
    printf("Power efficiency: %s\n", 
           total_time < 100000 ? "Good" : "Consider optimization");
}

#endif // STM32F0
