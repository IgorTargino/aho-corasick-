#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "aho_corasick.h"

#define MAX_LINE_LENGTH 256
#define MAX_PATTERNS_BUFFER 10

#define PATTERNS_FILE "data/patterns.txt"
#define INPUT_FILE "data/input.txt"
#define OUTPUT_FILE "data/output.txt"
#define REPORT_FILE "data/report.txt"

volatile bool g_forbidden_pattern_found = false;

void ac_set_match_callback(const char* pattern_found, int text_position) {
    g_forbidden_pattern_found = true;
    DEBUG_PRINTF("[FILTER] Detected: '%s' at position %d\n", pattern_found, text_position);
}

bool is_message_forbidden(ac_automaton_t* filter_automaton, const char* message) {
    g_forbidden_pattern_found = false; 
    ac_search(filter_automaton, message);
    return g_forbidden_pattern_found;
}

static bool validate_automaton_state(const ac_automaton_t* filter) {
    if (!ac_is_built(filter)) {
        DEBUG_PRINTF("Erro: Automato nao construido!\n");
        return false;
    }
    
    if (ac_get_vertex_count(filter) == 0 || ac_get_pattern_count(filter) == 0) {
        DEBUG_PRINTF("Erro: Automato vazio!\n");
        return false;
    }
    
    DEBUG_PRINTF("Automato validado com sucesso\n");
    return true;
}

static int load_patterns_directly(const char* filename, ac_automaton_t* automaton) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        DEBUG_PRINTF("Erro: Nao foi possivel abrir arquivo de padroes: %s\n", filename);
        return 0;
    }
    
    int patterns_loaded = 0;
    int patterns_added = 0;
    char line[MAX_LINE_LENGTH]; // Apenas um buffer pequeno
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0) continue;
        
        patterns_loaded++;

        if (patterns_added < AC_MAX_PATTERNS && ac_add_pattern(automaton, line)) {
            patterns_added++;
            DEBUG_PRINTF("  Padrao adicionado: '%s'\n", line);
        } else {
            DEBUG_PRINTF("  Padrao ignorado (limite ou erro): '%s'\n", line);
        }
    }
    
    fclose(file);
    DEBUG_PRINTF("Padroes carregados: %d, adicionados: %d\n", patterns_loaded, patterns_added);
    return patterns_added;
}

// Função para processar mensagens em lotes (uma por vez)
static bool process_messages_batch(const char* input_filename, const char* output_filename,
                                  ac_automaton_t* filter, int* total_processed, int* total_blocked) {
    FILE* input_file = fopen(input_filename, "r");
    if (!input_file) {
        DEBUG_PRINTF("Erro: Nao foi possivel abrir arquivo de entrada: %s\n", input_filename);
        return false;
    }
    
    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        DEBUG_PRINTF("Erro: Nao foi possivel criar arquivo de saida: %s\n", output_filename);
        fclose(input_file);
        return false;
    }
    
    // Escrever cabeçalho do arquivo de saída
    fprintf(output_file, "=== RESULTADO DA FILTRAGEM ===\n\n");
    
    char line[MAX_LINE_LENGTH]; // Apenas um buffer por vez
    int message_count = 0;
    int blocked_count = 0;
    
    // Processar linha por linha (sem array grande)
    while (fgets(line, sizeof(line), input_file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        message_count++;
        
        // Testar se mensagem é proibida
        bool is_blocked = is_message_forbidden(filter, line);
        if (is_blocked) {
            blocked_count++;
            DEBUG_PRINTF("  [%d] BLOQUEADA: '%.30s%s'\n", 
                        message_count, line, 
                        strlen(line) > 30 ? "..." : "");
        } else {
            DEBUG_PRINTF("  [%d] Permitida: '%.30s%s'\n", 
                        message_count, line, 
                        strlen(line) > 30 ? "..." : "");
        }
        
        // Escrever resultado diretamente no arquivo
        fprintf(output_file, "[%03d] %s: %s\n", 
                message_count,
                is_blocked ? "BLOQUEADA" : "PERMITIDA",
                line);
    }
    
    fclose(input_file);
    fclose(output_file);
    
    *total_processed = message_count;
    *total_blocked = blocked_count;
    
    DEBUG_PRINTF("Processamento concluido: %d mensagens, %d bloqueadas\n", 
                message_count, blocked_count);
    return true;
}

static bool write_report(const char* filename, int patterns_loaded, int patterns_added, 
                        int vertices, int messages_processed, int blocked_count) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        DEBUG_PRINTF("Erro: Nao foi possivel criar arquivo de relatorio: %s\n", filename);
        return false;
    }
    
    fprintf(file, "=== RELATORIO FILTRO DE CONTEUDO - AHO-CORASICK ===\n\n");
    fprintf(file, "Configuracao do Filtro:\n");
    fprintf(file, "  - Padroes carregados: %d\n", patterns_loaded);
    fprintf(file, "  - Padroes adicionados: %d\n", patterns_added);
    fprintf(file, "  - Vertices no automato: %d\n", vertices);
    fprintf(file, "\nResultados do Processamento:\n");
    fprintf(file, "  - Mensagens processadas: %d\n", messages_processed);
    fprintf(file, "  - Mensagens bloqueadas: %d\n", blocked_count);
    fprintf(file, "  - Taxa de bloqueio: %.1f%%\n", 
            messages_processed > 0 ? (100.0 * blocked_count) / messages_processed : 0.0);
    fprintf(file, "\nStatus: %s\n", 
            patterns_added > 0 ? "Filtro ativo e operacional" : "Erro na configuracao");
    
    fclose(file);
    DEBUG_PRINTF("Relatorio salvo em: %s\n", filename);
    return true;
}

int main() {
    DEBUG_PRINTF("=== Iniciando Filtro de Conteudo - Aho-Corasick ===\n");

    // Initialize automaton
    ac_automaton_t content_filter;
    ac_initialize_automaton(&content_filter);

    // Load patterns directly into automaton (sem array grande)
    DEBUG_PRINTF("Carregando padroes do arquivo...\n");
    int patterns_added = load_patterns_directly(PATTERNS_FILE, &content_filter);
    
    if (patterns_added == 0) {
        DEBUG_PRINTF("Erro: Nenhum padrao foi adicionado ao automato!\n");
        return 1;
    }

    // Build automaton
    DEBUG_PRINTF("Construindo automato...\n");
    ac_build_automaton(&content_filter);
    
    if (!validate_automaton_state(&content_filter)) {
        DEBUG_PRINTF("Erro: Falha na validacao do automato!\n");
        return 1;
    }
    
    DEBUG_PRINTF("Automato construido com sucesso!\n");

    // Process messages in batches (sem arrays grandes)
    DEBUG_PRINTF("Processando mensagens em lotes...\n");
    int total_processed = 0;
    int total_blocked = 0;
    
    bool processing_ok = process_messages_batch(INPUT_FILE, OUTPUT_FILE, 
                                               &content_filter, 
                                               &total_processed, &total_blocked);
    
    if (!processing_ok) {
        DEBUG_PRINTF("Erro: Falha no processamento das mensagens!\n");
        return 1;
    }

    // Write report
    DEBUG_PRINTF("Gerando relatorio...\n");
    bool report_ok = write_report(REPORT_FILE, patterns_added, patterns_added, 
                                 ac_get_vertex_count(&content_filter), 
                                 total_processed, total_blocked);

    DEBUG_PRINTF("=== Processamento Concluido ===\n");
    DEBUG_PRINTF("Relatorio: %s (%s)\n", REPORT_FILE, report_ok ? "OK" : "ERRO");
    DEBUG_PRINTF("Resultado: %s (%s)\n", OUTPUT_FILE, processing_ok ? "OK" : "ERRO");
    
    return (report_ok && processing_ok) ? 0 : 1;
}