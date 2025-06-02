#include "aho_corasick.h"
#include <stdio.h>

void ac_set_match_callback(const char* pattern, int pos) {
    printf("Padrão encontrado: '%s' na posição %d\n", pattern, pos);
}

int main(void) {
    printf("Caso 1: Padrões simples\n");
    ac_automaton_t ac1;
    ac_initialize_automaton(&ac1);

    ac_add_pattern(&ac1, "he");
    ac_add_pattern(&ac1, "hers");
    ac_add_pattern(&ac1, "she");
    ac_add_pattern(&ac1, "his");

    ac_build_automaton(&ac1);

    const char* texto1 = "hersheys";
    ac_search(&ac1, texto1);

    printf("\nCaso 2: Padrões sobrepostos\n");
    ac_automaton_t ac2;
    ac_initialize_automaton(&ac2);

    ac_add_pattern(&ac2, "a");
    ac_add_pattern(&ac2, "ab");
    ac_add_pattern(&ac2, "bab");
    ac_add_pattern(&ac2, "bc");
    ac_add_pattern(&ac2, "bca");
    ac_add_pattern(&ac2, "c");
    ac_add_pattern(&ac2, "caa");

    ac_build_automaton(&ac2);

    const char* texto2 = "abccab";
    ac_search(&ac2, texto2);

    printf("\nCaso 3: Palavras-chave em notícia\n");
    ac_automaton_t ac3;
    ac_initialize_automaton(&ac3);

    ac_add_pattern(&ac3, "presidente");
    ac_add_pattern(&ac3, "brasil");
    ac_add_pattern(&ac3, "pib");
    
    ac_build_automaton(&ac3);

    const char* noticia = 
        "o presidente do brasil anunciou novas medidas para estimular a economia"
        "o governo espera que o crescimento do pib aumente e que o desemprego diminua"
        "investimentos em infraestrutura, saude e educacao tambem foram destacados"
        "a balanca comercial apresentou superavit apesar da alta do dolar e da inflacao"
        "especialistas analisam o impacto dos juros e das reformas no mercado";

    ac_search(&ac3, noticia);

    return 0;
}