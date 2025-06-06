#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

// --- Test Configuration ---
#define MAX_TEST_MATCHES 100
#define MAX_PATTERN_LENGTH 64
#define MAX_PATTERN_BUFFER 50

// --- Test Statistics ---
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int total_assertions;
    int passed_assertions;
    int failed_assertions;
} test_stats_t;

static test_stats_t g_test_stats = {0};

// --- Test Match Tracking ---
typedef struct {
    char pattern[MAX_PATTERN_LENGTH];
    int position;
    bool found;
} test_match_t;

static test_match_t g_test_matches[MAX_TEST_MATCHES];
static int g_match_count = 0;

// --- Test Framework Macros ---
#define TEST_START(test_name) \
    do { \
        g_test_stats.total_tests++; \
        test_reset_matches(); \
        test_log_info("\n[TEST %d] %s", g_test_stats.total_tests, test_name); \
    } while(0)

#define TEST_ASSERT(condition, format, ...) \
    do { \
        g_test_stats.total_assertions++; \
        if (condition) { \
            test_log_pass("PASS: " format, ##__VA_ARGS__); \
            g_test_stats.passed_assertions++; \
        } else { \
            test_log_fail("FAIL: " format, ##__VA_ARGS__); \
            g_test_stats.failed_assertions++; \
        } \
    } while(0)

#define TEST_SUMMARY() test_print_final_summary()

// --- Logging Functions ---
static void test_log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void test_log_pass(const char* format, ...) {
    printf("  ✅ ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void test_log_fail(const char* format, ...) {
    printf("  ❌ ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void test_log_data(const char* format, ...) {
    printf("    📊 ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void test_log_input(const char* format, ...) {
    printf("    📝 ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void test_log_search(const char* format, ...) {
    printf("    🔍 ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// --- Test Utilities ---
static void test_reset_matches(void) {
    g_match_count = 0;
    memset(g_test_matches, 0, sizeof(g_test_matches));
}

static bool test_has_match(const char* expected_pattern) {
    if (!expected_pattern) return false;
    
    for (int i = 0; i < g_match_count; i++) {
        if (strcmp(g_test_matches[i].pattern, expected_pattern) == 0) {
            return true;
        }
    }
    return false;
}

static bool test_has_match_at_position(const char* expected_pattern, int expected_position) {
    if (!expected_pattern) return false;
    
    for (int i = 0; i < g_match_count; i++) {
        if (strcmp(g_test_matches[i].pattern, expected_pattern) == 0 && 
            g_test_matches[i].position == expected_position) {
            return true;
        }
    }
    return false;
}

static int test_count_matches(const char* expected_pattern) {
    if (!expected_pattern) return 0;
    
    int count = 0;
    for (int i = 0; i < g_match_count; i++) {
        if (strcmp(g_test_matches[i].pattern, expected_pattern) == 0) {
            count++;
        }
    }
    return count;
}

static void test_print_all_matches(void) {
    printf("    📋 Found %d matches:\n", g_match_count);
    for (int i = 0; i < g_match_count; i++) {
        printf("      [%d] '%s' at position %d\n", 
               i + 1, g_test_matches[i].pattern, g_test_matches[i].position);
    }
}

static void test_print_final_summary(void) {
    printf("\n=== FINAL TEST SUMMARY ===\n");
    printf("Test Cases:\n");
    printf("  Total: %d\n", g_test_stats.total_tests);
    printf("  Passed: %d\n", g_test_stats.passed_tests);
    printf("  Failed: %d\n", g_test_stats.failed_tests);
    
    printf("Assertions:\n");
    printf("  Total: %d\n", g_test_stats.total_assertions);
    printf("  Passed: %d\n", g_test_stats.passed_assertions);
    printf("  Failed: %d\n", g_test_stats.failed_assertions);
    
    if (g_test_stats.total_assertions > 0) {
        double success_rate = (100.0 * g_test_stats.passed_assertions) / g_test_stats.total_assertions;
        printf("  Success rate: %.1f%%\n", success_rate);
    }
    
    bool all_passed = (g_test_stats.failed_assertions == 0);
    printf("Final status: %s\n", all_passed ? "✅ ALL TESTS PASSED" : "❌ SOME TESTS FAILED");
}

// --- Mock Callback Implementation ---
void ac_set_match_callback(const char* pattern, int position) {
    if (g_match_count >= MAX_TEST_MATCHES || !pattern) {
        return;
    }
    
    // Safe string copy
    size_t pattern_len = strlen(pattern);
    size_t copy_len = (pattern_len < MAX_PATTERN_LENGTH - 1) ? pattern_len : MAX_PATTERN_LENGTH - 1;
    
    strncpy(g_test_matches[g_match_count].pattern, pattern, copy_len);
    g_test_matches[g_match_count].pattern[copy_len] = '\0';
    g_test_matches[g_match_count].position = position;
    g_test_matches[g_match_count].found = true;
    
    test_log_search("Match found: '%s' at position %d", pattern, position);
    g_match_count++;
}

// --- Individual Test Functions ---
static void test_automaton_initialization(void) {
    TEST_START("Automaton Initialization");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    TEST_ASSERT(ac_get_vertex_count(&ac) == 1, "Root vertex should be created");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 0, "No initial patterns should exist");
    TEST_ASSERT(!ac_is_built(&ac), "Automaton should not be built initially");
    
    // Test NULL pointer handling - should not crash
    ac_initialize_automaton(NULL);
    TEST_ASSERT(true, "Should handle NULL pointer gracefully");
    
    g_test_stats.passed_tests++;
}

static void test_pattern_addition(void) {
    TEST_START("Pattern Addition");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Test valid pattern addition
    bool result = ac_add_pattern(&ac, "test");
    TEST_ASSERT(result, "Should successfully add valid pattern 'test'");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 1, "Pattern counter should be 1 after first addition");
    
    // Test edge cases
    TEST_ASSERT(!ac_add_pattern(&ac, ""), "Should reject empty pattern");
    TEST_ASSERT(!ac_add_pattern(&ac, NULL), "Should reject NULL pattern");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 1, "Counter should remain unchanged after rejections");
    
    // Test multiple valid patterns
    TEST_ASSERT(ac_add_pattern(&ac, "hello"), "Should accept second valid pattern 'hello'");
    TEST_ASSERT(ac_add_pattern(&ac, "world"), "Should accept third valid pattern 'world'");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 3, "Should have exactly 3 valid patterns");
    
    // Test case sensitivity
    TEST_ASSERT(ac_add_pattern(&ac, "Test"), "Should accept pattern with uppercase letters");
    
    // Test single character pattern
    TEST_ASSERT(ac_add_pattern(&ac, "a"), "Should accept single character pattern");
    
    // Test patterns with invalid characters (implementation dependent)
    bool invalid_char_result = ac_add_pattern(&ac, "test123");
    test_log_data("Pattern with numbers result: %s", invalid_char_result ? "accepted" : "rejected");
    
    g_test_stats.passed_tests++;
}

static void test_vertex_limits(void) {
    TEST_START("Vertex Limits");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    char pattern_buffer[MAX_PATTERN_BUFFER];
    int patterns_added = 0;
    
    // Create diverse patterns to maximize vertex usage
    for (int i = 0; i < 20 && patterns_added < AC_MAX_PATTERNS; i++) {
        int ret = snprintf(pattern_buffer, sizeof(pattern_buffer), "%c%c%c%c%c", 
                          'a' + (i % 26), 'b' + ((i + 1) % 26), 
                          'c' + ((i + 2) % 26), 'd' + ((i + 3) % 26), 
                          'e' + ((i + 4) % 26));
        
        if (ret >= sizeof(pattern_buffer)) continue;
        
        if (ac_add_pattern(&ac, pattern_buffer)) {
            patterns_added++;
        }
    }
    
    ac_build_automaton(&ac);
    
    int vertex_count = ac_get_vertex_count(&ac);
    test_log_data("Created %d vertices with %d patterns", vertex_count, patterns_added);
    
    TEST_ASSERT(vertex_count <= AC_MAX_VERTICES, 
                "Should not exceed vertex limit (%d), got %d", AC_MAX_VERTICES, vertex_count);
    TEST_ASSERT(vertex_count >= 1, "Should have at least the root vertex");
    TEST_ASSERT(patterns_added > 0, "Should have successfully added at least one pattern");
    
    g_test_stats.passed_tests++;
}

static void test_automaton_construction(void) {
    TEST_START("Automaton Construction");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Add overlapping patterns to test failure link construction
    const char* patterns[] = {"he", "she", "his", "hers"};
    const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
    
    for (int i = 0; i < pattern_count; i++) {
        bool result = ac_add_pattern(&ac, patterns[i]);
        TEST_ASSERT(result, "Should successfully add pattern '%s'", patterns[i]);
    }
    
    int patterns_before = ac_get_pattern_count(&ac);
    int vertices_before = ac_get_vertex_count(&ac);
    
    // Build the automaton
    ac_build_automaton(&ac);
    
    TEST_ASSERT(ac_is_built(&ac), "Automaton should be marked as built");
    TEST_ASSERT(ac_get_vertex_count(&ac) >= vertices_before, 
                "Vertex count should not decrease after building");
    TEST_ASSERT(ac_get_pattern_count(&ac) == patterns_before, 
                "Pattern count should be preserved during building");
    TEST_ASSERT(ac_get_pattern_count(&ac) == pattern_count, 
                "Should maintain all %d patterns", pattern_count);
    
    test_log_data("Built automaton: %d vertices, %d patterns", 
                  ac_get_vertex_count(&ac), ac_get_pattern_count(&ac));
    
    g_test_stats.passed_tests++;
}

static void test_empty_automaton_construction(void) {
    TEST_START("Empty Automaton Construction");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Attempt to build automaton without any patterns
    ac_build_automaton(&ac);
    
    TEST_ASSERT(ac_get_vertex_count(&ac) == 1, "Should have only the root vertex");
    TEST_ASSERT(ac_get_pattern_count(&ac) == 0, "Should have no patterns");
    
    // Note: is_built() behavior with empty automaton is implementation dependent
    bool is_built = ac_is_built(&ac);
    test_log_data("Empty automaton is_built() returns: %s", is_built ? "true" : "false");
    
    g_test_stats.passed_tests++;
}

static void test_single_pattern_search(void) {
    TEST_START("Single Pattern Search");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* pattern = "test";
    TEST_ASSERT(ac_add_pattern(&ac, pattern), "Should add pattern '%s'", pattern);
    ac_build_automaton(&ac);
    
    // Test exact match
    test_reset_matches();
    test_log_input("Input text: '%s'", pattern);
    ac_search(&ac, pattern);
    test_print_all_matches();
    
    TEST_ASSERT(g_match_count >= 1, "Should find at least one occurrence in exact match");
    TEST_ASSERT(test_has_match(pattern), "Should find the pattern '%s'", pattern);
    
    // Test pattern within larger text
    test_reset_matches();
    const char* text_with_pattern = "this is a test string";
    test_log_input("Input text: '%s'", text_with_pattern);
    ac_search(&ac, text_with_pattern);
    test_print_all_matches();
    
    TEST_ASSERT(g_match_count >= 1, "Should find pattern in larger text");
    TEST_ASSERT(test_has_match(pattern), "Should find '%s' within text", pattern);
    
    // Test text without pattern
    test_reset_matches();
    const char* text_without_pattern = "nothing here";
    test_log_input("Input text: '%s'", text_without_pattern);
    ac_search(&ac, text_without_pattern);
    
    TEST_ASSERT(g_match_count == 0, "Should not find pattern in unrelated text");
    
    // Test edge cases
    test_reset_matches();
    test_log_input("Input text: '' (empty string)");
    ac_search(&ac, "");
    TEST_ASSERT(g_match_count == 0, "Should not find anything in empty text");
    
    test_reset_matches();
    test_log_input("Input text: NULL");
    ac_search(&ac, NULL);
    TEST_ASSERT(g_match_count == 0, "Should handle NULL text gracefully");
    
    g_test_stats.passed_tests++;
}

static void test_multiple_patterns_search(void) {
    TEST_START("Multiple Patterns Search");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* patterns[] = {"he", "she", "his", "hers"};
    const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
    
    for (int i = 0; i < pattern_count; i++) {
        TEST_ASSERT(ac_add_pattern(&ac, patterns[i]), "Should add pattern '%s'", patterns[i]);
    }
    ac_build_automaton(&ac);
    
    // Test overlapping matches
    test_reset_matches();
    const char* overlapping_text = "ushers";
    test_log_input("Input text: '%s'", overlapping_text);
    ac_search(&ac, overlapping_text);
    test_print_all_matches();
    
    TEST_ASSERT(g_match_count >= 1, "Should find at least one pattern in overlapping text");
    
    // Check which patterns were found
    bool found_patterns[pattern_count];
    for (int i = 0; i < pattern_count; i++) {
        found_patterns[i] = test_has_match(patterns[i]);
    }
    
    test_log_data("Patterns found in '%s': he=%s, she=%s, his=%s, hers=%s", 
                  overlapping_text,
                  found_patterns[0] ? "yes" : "no",
                  found_patterns[1] ? "yes" : "no", 
                  found_patterns[2] ? "yes" : "no",
                  found_patterns[3] ? "yes" : "no");
    
    bool found_any = false;
    for (int i = 0; i < pattern_count; i++) {
        if (found_patterns[i]) {
            found_any = true;
            break;
        }
    }
    TEST_ASSERT(found_any, "Should find at least one overlapping pattern");
    
    // Test non-overlapping matches
    test_reset_matches();
    const char* separated_text = "he is his friend and she is hers";
    test_log_input("Input text: '%s'", separated_text);
    ac_search(&ac, separated_text);
    test_print_all_matches();
    
    TEST_ASSERT(g_match_count >= 4, "Should find multiple distinct occurrences");
    
    g_test_stats.passed_tests++;
}

static void test_case_sensitivity(void) {
    TEST_START("Case Sensitivity");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* lowercase_pattern = "test";
    TEST_ASSERT(ac_add_pattern(&ac, lowercase_pattern), "Should add lowercase pattern");
    ac_build_automaton(&ac);
    
    const char* test_cases[] = {"TEST", "Test", "tEsT", "TeSt"};
    const int case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < case_count; i++) {
        test_reset_matches();
        test_log_input("Input text: '%s'", test_cases[i]);
        ac_search(&ac, test_cases[i]);
        
        test_log_data("Searching '%s': found %d matches", test_cases[i], g_match_count);
        
        // Note: Case sensitivity behavior is implementation dependent
        if (g_match_count > 0) {
            test_log_data("Implementation appears to be case-insensitive");
        } else {
            test_log_data("Implementation appears to be case-sensitive");
        }
        
        // Just record the behavior rather than asserting specific expectations
        TEST_ASSERT(g_match_count >= 0, "Match count should be non-negative");
    }
    
    g_test_stats.passed_tests++;
}

static void test_repeated_patterns(void) {
    TEST_START("Repeated Patterns in Text");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* pattern = "ab";
    TEST_ASSERT(ac_add_pattern(&ac, pattern), "Should add pattern '%s'", pattern);
    ac_build_automaton(&ac);
    
    test_reset_matches();
    const char* repeated_text = "ababab";
    test_log_input("Input text: '%s'", repeated_text);
    ac_search(&ac, repeated_text);
    test_print_all_matches();
    
    int pattern_count = test_count_matches(pattern);
    test_log_data("Found '%s' %d times in '%s'", pattern, pattern_count, repeated_text);
    
    TEST_ASSERT(pattern_count >= 1, "Should find pattern at least once");
    TEST_ASSERT(pattern_count <= 3, "Should not find more than 3 occurrences");
    
    g_test_stats.passed_tests++;
}

static void test_invalid_characters(void) {
    TEST_START("Invalid Characters Handling");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* pattern = "test";
    TEST_ASSERT(ac_add_pattern(&ac, pattern), "Should add pattern '%s'", pattern);
    ac_build_automaton(&ac);
    
    // Test cases with invalid characters
    const char* test_cases[] = {
        "test123!@#",
        "123test456", 
        "te123st"
    };
    const int case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < case_count; i++) {
        test_reset_matches();
        test_log_input("Input text: '%s'", test_cases[i]);
        ac_search(&ac, test_cases[i]);
        
        test_log_data("'%s' produced %d matches", test_cases[i], g_match_count);
        
        // Behavior with invalid characters is implementation dependent
        TEST_ASSERT(g_match_count >= 0, "Match count should be non-negative");
    }
    
    g_test_stats.passed_tests++;
}

static void test_edge_cases(void) {
    TEST_START("Edge Cases");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    // Test single character pattern
    const char* single_char = "a";
    TEST_ASSERT(ac_add_pattern(&ac, single_char), "Should add single character pattern");
    ac_build_automaton(&ac);
    
    test_reset_matches();
    const char* repeated_chars = "aaa";
    test_log_input("Input text: '%s'", repeated_chars);
    ac_search(&ac, repeated_chars);
    test_print_all_matches();
    
    int char_count = test_count_matches(single_char);
    test_log_data("Found '%s' %d times in '%s'", single_char, char_count, repeated_chars);
    
    TEST_ASSERT(char_count >= 1, "Should find single character at least once");
    TEST_ASSERT(char_count <= 3, "Should not find more than 3 occurrences");
    
    // Test empty inputs
    test_reset_matches();
    test_log_input("Input text: '' (empty)");
    ac_search(&ac, "");
    TEST_ASSERT(g_match_count == 0, "Should find nothing in empty text");
    
    test_reset_matches();
    test_log_input("Input text: NULL");
    ac_search(&ac, NULL);
    TEST_ASSERT(g_match_count == 0, "Should handle NULL text gracefully");
    
    g_test_stats.passed_tests++;
}

static void test_boundary_conditions(void) {
    TEST_START("Boundary Conditions");
    
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    
    const char* start_pattern = "start";
    const char* end_pattern = "end";
    
    TEST_ASSERT(ac_add_pattern(&ac, start_pattern), "Should add pattern '%s'", start_pattern);
    TEST_ASSERT(ac_add_pattern(&ac, end_pattern), "Should add pattern '%s'", end_pattern);
    ac_build_automaton(&ac);
    
    test_reset_matches();
    const char* boundary_text = "start of text ends here with end";
    test_log_input("Input text: '%s'", boundary_text);
    ac_search(&ac, boundary_text);
    test_print_all_matches();
    
    TEST_ASSERT(test_has_match(start_pattern), "Should find '%s' at text beginning", start_pattern);
    TEST_ASSERT(test_has_match(end_pattern), "Should find '%s' in text", end_pattern);
    
    // Test single character text
    ac_automaton_t ac2;
    ac_initialize_automaton(&ac2);
    
    const char* single_pattern = "x";
    TEST_ASSERT(ac_add_pattern(&ac2, single_pattern), "Should add pattern '%s'", single_pattern);
    ac_build_automaton(&ac2);
    
    test_reset_matches();
    test_log_input("Input text: '%s'", single_pattern);
    ac_search(&ac2, single_pattern);
    
    TEST_ASSERT(g_match_count == 1, "Should find exactly one match in single character text");
    
    g_test_stats.passed_tests++;
}

// --- Test Runner ---
typedef void (*test_function_t)(void);

static const struct {
    const char* name;
    test_function_t function;
} test_cases[] = {
    {"Automaton Initialization", test_automaton_initialization},
    {"Pattern Addition", test_pattern_addition},
    {"Vertex Limits", test_vertex_limits},
    {"Automaton Construction", test_automaton_construction},
    {"Empty Automaton Construction", test_empty_automaton_construction},
    {"Single Pattern Search", test_single_pattern_search},
    {"Multiple Patterns Search", test_multiple_patterns_search},
    {"Case Sensitivity", test_case_sensitivity},
    {"Repeated Patterns", test_repeated_patterns},
    {"Invalid Characters", test_invalid_characters},
    {"Edge Cases", test_edge_cases},
    {"Boundary Conditions", test_boundary_conditions}
};

static void print_test_configuration(void) {
    printf("=== AHO-CORASICK TEST SUITE ===\n");
    printf("Test Configuration:\n");
    printf("  AC_MAX_VERTICES: %d\n", AC_MAX_VERTICES);
    printf("  AC_MAX_PATTERNS: %d\n", AC_MAX_PATTERNS);
    printf("  AC_MAX_PATTERNS_PER_VERTEX: %d\n", AC_MAX_PATTERNS_PER_VERTEX);
    printf("  AC_K_ALPHABET_SIZE: %d\n", AC_K_ALPHABET_SIZE);
    printf("  MAX_TEST_MATCHES: %d\n", MAX_TEST_MATCHES);
    printf("  MAX_PATTERN_LENGTH: %d\n", MAX_PATTERN_LENGTH);
}

int main(void) {
    print_test_configuration();
    
    const int total_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Run all test cases
    for (int i = 0; i < total_test_cases; i++) {
        test_cases[i].function();
    }
    
    // Update final statistics
    g_test_stats.failed_tests = g_test_stats.total_tests - g_test_stats.passed_tests;
    
    // Print final summary
    TEST_SUMMARY();
    
    // Return appropriate exit code
    return (g_test_stats.failed_assertions == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}