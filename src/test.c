#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_MATCHES 64

typedef struct {
    char pattern[32];
    int pos;
} match_t;

static match_t matches[MAX_MATCHES];
static int match_count = 0;

void ac_set_match_callback(const char* pattern, int pos) {
    assert(match_count < MAX_MATCHES);
    strncpy(matches[match_count].pattern, pattern, sizeof(matches[match_count].pattern) - 1);
    matches[match_count].pattern[sizeof(matches[match_count].pattern) - 1] = '\0';
    matches[match_count].pos = pos;
    match_count++;
}

void reset_matches() {
    match_count = 0;
    memset(matches, 0, sizeof(matches));
}

void test_overlapping_patterns() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    ac_add_pattern(&ac, "ana");
    ac_add_pattern(&ac, "nana");
    ac_add_pattern(&ac, "anana");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "bananana");

    int found_ana1 = 0, found_ana3 = 0, found_nana2 = 0, found_nana4 = 0, found_anana1 = 0, found_anana3 = 0;

    for (int i = 0; i < match_count; ++i) {
        if (strcmp(matches[i].pattern, "ana") == 0 && matches[i].pos == 3) found_ana1 = 1;
        if (strcmp(matches[i].pattern, "ana") == 0 && matches[i].pos == 5) found_ana3 = 1;
        if (strcmp(matches[i].pattern, "nana") == 0 && matches[i].pos == 5) found_nana2 = 1;
        if (strcmp(matches[i].pattern, "nana") == 0 && matches[i].pos == 7) found_nana4 = 1;
        if (strcmp(matches[i].pattern, "anana") == 0 && matches[i].pos == 7) found_anana1 = 1;
        if (strcmp(matches[i].pattern, "anana") == 0 && matches[i].pos == 5) found_anana3 = 1;
    }

    assert(found_ana1 && found_ana3 && found_nana2 && found_nana4 && found_anana1 && found_anana3);
}

void test_no_matches() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    ac_add_pattern(&ac, "xyz");
    ac_add_pattern(&ac, "uxl");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "abcdefgh");

    assert(match_count == 0);
}

void test_substring_pattern() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    ac_add_pattern(&ac, "abc");
    ac_add_pattern(&ac, "bc");
    ac_add_pattern(&ac, "c");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "abc");

    int found_abc = 0, found_bc = 0, found_c = 0;
    for (int i = 0; i < match_count; ++i) {
        if (strcmp(matches[i].pattern, "abc") == 0 && matches[i].pos == 2) found_abc = 1;
        if (strcmp(matches[i].pattern, "bc") == 0 && matches[i].pos == 2) found_bc = 1;
        if (strcmp(matches[i].pattern, "c") == 0 && matches[i].pos == 2) found_c = 1;
    }
    assert(found_abc && found_bc && found_c);
}

void test_empty_pattern_and_text() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    // Empty pattern should not be added/found
    ac_add_pattern(&ac, "");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "");

    assert(match_count == 0);

    // Now test with non-empty pattern and empty text
    ac_initialize_automaton(&ac);
    ac_add_pattern(&ac, "a");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "");

    assert(match_count == 0);
}

void test_special_characters() {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    ac_add_pattern(&ac, "a.c");
    ac_add_pattern(&ac, "b$c");
    ac_add_pattern(&ac, "^start");
    ac_build_automaton(&ac);

    reset_matches();
    ac_search(&ac, "a.c b$c ^start");

    int found_ac = 0, found_bc = 0, found_start = 0;
    for (int i = 0; i < match_count; ++i) {
        if (strcmp(matches[i].pattern, "a.c") == 0) found_ac = 1;
        if (strcmp(matches[i].pattern, "b$c") == 0) found_bc = 1;
        if (strcmp(matches[i].pattern, "^start") == 0) found_start = 1;
    }
    assert(found_ac && found_bc && found_start);
}

int main() {
    test_overlapping_patterns();
    test_no_matches();
    test_substring_pattern();
    test_empty_pattern_and_text();
    test_special_characters();
    printf("Todos os testes passaram!\n");
    return 0;
}