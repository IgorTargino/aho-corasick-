#include "aho_corasick.h"
#include <stdio.h>

void ac_set_match_callback(const char* pattern, int pos) {
    printf("Padrão encontrado: '%s' na posição %d\n", pattern, pos);
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