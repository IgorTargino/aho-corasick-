#include "aho_corasick.h" // Inclui nossa biblioteca Aho-Corasick
#include <stdio.h>       // Para printf

static char application_internal_data_buffer[8000];

int main(void) {
    // --- Utilização da Biblioteca Aho-Corasick ---
    printf("--- Iniciando Teste do Aho-Corasick em C ---\n");

    // 1. Inicializa o autômato Aho-Corasick
    ac_initialize_automaton();

    // 2. Adiciona os padrões desejados
    const char* patterns_to_search[] = {"he", "she", "his", "hers", "mundo", "do", "ola"}; // "olá" com acento não cabe no alfabeto a-z
    // Nota: o padrão "olá" original com acento não será processado corretamente
    // pelo `char_idx = ch - 'a'` se 'á' não estiver no intervalo 'a'-'z'.
    // Removido acento para este exemplo de alfabeto simples.
    int num_patterns = sizeof(patterns_to_search) / sizeof(patterns_to_search[0]);

    printf("\nAdicionando padrões ao autômato:\n");
    for (int i = 0; i < num_patterns; ++i) {
        printf("- \"%s\"\n", patterns_to_search[i]);
        ac_add_pattern(patterns_to_search[i]);
    }

    // 3. Constrói os links de falha e transições 'go'
    printf("\nConstruindo o autômato Aho-Corasick...\n");
    ac_build_automaton();
    // printf("Autômato construído.\n"); 

    // 4. Realiza a busca no texto
    const char* text1 = "ushers hidomundo olamundo"; // "olámundo" com acento alterado para "olamundo"
    ac_search(text1);

    const char* text2 = "ele disse ola para ela e o mundo respondeu";
    ac_search(text2);
    
    const char* text3 = "hehehe";
    ac_search(text3);

    const char* text4 = "um texto simples sem nenhuma correspondencia";
    ac_search(text4);


    printf("\n--- Teste do Aho-Corasick em C Concluído ---\n");

    return 0;
}