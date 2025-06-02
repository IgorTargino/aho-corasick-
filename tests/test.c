#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>

#define MAX_MATCHES 100
#define MAX_PATTERN_LEN 32

typedef struct {
    char patterns[MAX_MATCHES][MAX_PATTERN_LEN];
    int positions[MAX_MATCHES];
    int pattern_indices[MAX_MATCHES];
    int count;
} match_collector_t;

static match_collector_t collector;

static void test_on_match(const char* pattern, int pos, int pattern_index) {
    printf("[MATCH] pattern=\"%s\" pos=%d pattern_index=%d\n", pattern, pos, pattern_index);
    if (collector.count < MAX_MATCHES) {
        strncpy(collector.patterns[collector.count], pattern, MAX_PATTERN_LEN-1);
        collector.patterns[collector.count][MAX_PATTERN_LEN-1] = '\0';
        collector.positions[collector.count] = pos;
        collector.pattern_indices[collector.count] = pattern_index;
        collector.count++;
    }
}

static void reset_collector() {
    collector.count = 0;
}

static int assert_match(const char* pattern, int pos) {
    for (int i = 0; i < collector.count; ++i) {
        if (strcmp(collector.patterns[i], pattern) == 0 && collector.positions[i] == pos)
            return 1;
    }
    return 0;
}

static void test_overlapping_patterns() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    ac_add_pattern(&ac, "a");
    ac_add_pattern(&ac, "ab");
    ac_add_pattern(&ac, "bab");
    ac_add_pattern(&ac, "bc");
    ac_add_pattern(&ac, "bca");
    ac_add_pattern(&ac, "c");
    ac_add_pattern(&ac, "caa");
    ac_build_automaton(&ac);

    reset_collector();
    ac_search(&ac, "abccab");

    int pass = assert_match("a", 0) && assert_match("ab", 0) &&
               assert_match("bc", 1) && assert_match("c", 2) &&
               assert_match("c", 3) && assert_match("a", 4) &&
               assert_match("ab", 4);

    printf("test_overlapping_patterns: %s\n", pass ? "PASS" : "FAIL");
}

static void test_no_matches() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    ac_add_pattern(&ac, "xyz");
    ac_build_automaton(&ac);

    reset_collector();
    ac_set_match_callback(&ac, test_on_match);
    ac_search(&ac, "abcdefg");

    printf("test_no_matches: %s\n", collector.count == 0 ? "PASS" : "FAIL");
}

static void test_substring_patterns() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    ac_add_pattern(&ac, "a");
    ac_add_pattern(&ac, "aa");
    ac_build_automaton(&ac);

    reset_collector();
    ac_set_match_callback(&ac, test_on_match);
    ac_search(&ac, "aaa");

    int pass = assert_match("a", 0) && assert_match("aa", 0) &&
               assert_match("a", 1) && assert_match("aa", 1) &&
               assert_match("a", 2);

    printf("test_substring_patterns: %s\n", pass ? "PASS" : "FAIL");
}

static void test_empty_pattern_and_text() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    // No patterns added
    ac_build_automaton(&ac);

    reset_collector();
    ac_set_match_callback(&ac, test_on_match);
    ac_search(&ac, "");

    printf("test_empty_pattern_and_text: %s\n", collector.count == 0 ? "PASS" : "FAIL");
}

static void test_special_characters() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);
    ac_add_pattern(&ac, "he");
    ac_add_pattern(&ac, "she");
    ac_build_automaton(&ac);

    reset_collector();
    ac_set_match_callback(&ac, test_on_match);
    ac_search(&ac, "he she");

    int pass = assert_match("he", 0) && assert_match("she", 3);

    printf("test_special_characters: %s\n", pass ? "PASS" : "FAIL");
}

int main(void) {
    test_overlapping_patterns();
    test_no_matches();
    test_substring_patterns();
    test_empty_pattern_and_text();
    test_special_characters();
    return 0;
}