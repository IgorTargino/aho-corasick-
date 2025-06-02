#include "aho_corasick.h"
#include <stdio.h>

void ac_on_match(const char* pattern, int pos, int pattern_index) {
    printf("Padrão encontrado: '%s' na posição %d (índice do padrão: %d)\n", pattern, pos, pattern_index);
}

int main(void) {
    ac_automaton_t ac;
    ac_initialize_automaton(&ac);

    ac_add_pattern(&ac, "he");
    ac_add_pattern(&ac, "hers");
    ac_add_pattern(&ac, "she");
    ac_add_pattern(&ac, "his");

    ac_build_automaton(&ac);

    const char* texto = "hersheys";
    ac_search(&ac, texto);

    return 0;
}