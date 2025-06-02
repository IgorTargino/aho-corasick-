#include "aho_corasick.h"
#include <stdio.h>

int main(void) {
    printf("--- Demonstração Aho-Corasick com Padrões e Texto Realistas (~8KB estrutura) ---\n");

    ac_initialize_automaton();

    const char* patterns[] = {
        "algoritmo", // 9 estados
        "debug",     // 5 estados
        "loop",      // 4 estados
        "array",     // 5 estados
        "string",    // 6 estados
        "function"   // 8 estados
    };

    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    printf("\nAdicionando padrões ao autômato:\n");
    for (int i = 0; i < num_patterns; ++i) {
        printf("- \"%s\"\n", patterns[i]);
        ac_add_pattern(patterns[i]);
    }

    printf("\nConstruindo o autômato Aho-Corasick...\n");
    ac_build_automaton();
    printf("Autômato construído.\n");

    const char* text_to_search = 
        "o programador ao desenvolver um novo algoritmo complexo e comum encontrar "
        "um bug inesperado que exija um processo de debug minucioso muitas vezes um "
        "loop infinito ou uma condicao de parada incorreta pode ser a causa para "

        "function deve ter um proposito claro e bem definido no sistema de software";

    ac_search(text_to_search);
    
    const char* text_to_search2 = 
        "este e um texto simples sobre hardware e redes nao contem termos como array ou function";
    ac_search(text_to_search2);


    printf("\n--- Demonstração Concluída ---\n");

    return 0;
}