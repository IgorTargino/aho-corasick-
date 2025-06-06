#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "aho_corasick.h"

#define MAX_LINE_LENGTH 128
#define MAX_PATTERN_LENGTH 32
#define MAX_FILE_BUFFER 1024

#define PATTERNS_FILE "data/patterns.txt"
#define INPUT_FILE "data/input.txt"
#define OUTPUT_FILE "data/output.txt"
#define REPORT_FILE "data/report.txt"

volatile bool g_forbidden_pattern_found = false;

void ac_set_match_callback(const char* pattern_found, int text_position) {
    g_forbidden_pattern_found = true;
}

bool is_message_forbidden(ac_automaton_t* filter_automaton, const char* message) {
    g_forbidden_pattern_found = false; 
    ac_search(filter_automaton, message);
    return g_forbidden_pattern_found;
}

static int read_file_complete(const char* filename, char* buffer, int max_len) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        DEBUG_PRINTF("❌ Erro: não foi possível abrir '%s'\n", filename);
        return 1;
    }
    
    setvbuf(file, NULL, _IONBF, 0);
    
    int len = 0;
    int c;
    while ((c = fgetc(file)) != EOF && len < max_len - 1) {
        buffer[len++] = c;
    }
    buffer[len] = '\0';
    
    bool truncated = false;
    if (len == max_len - 1 && (c = fgetc(file)) != EOF) {
        truncated = true;
    }
    
    fclose(file);
    
    DEBUG_PRINTF("📊 Arquivo '%s': %d bytes lidos%s\n", 
                 filename, len, truncated ? " (TRUNCADO!)" : "");
    
    return 0;
}

static int write_file_complete(const char* filename, const char* buffer) {
    FILE* file = fopen(filename, "w");
    if (!file) return 1;
    
    setvbuf(file, NULL, _IONBF, 0);
    
    int len = strlen(buffer);
    for (int i = 0; i < len; i++) {
        fputc(buffer[i], file);
    }
    
    fclose(file);
    return 0;
}

static int parse_patterns_from_buffer(const char* buffer, ac_automaton_t* automaton) {
    char line[MAX_PATTERN_LENGTH];
    int patterns_added = 0;
    int buffer_pos = 0;
    int line_pos = 0;
    
    while (buffer[buffer_pos] && patterns_added < AC_MAX_PATTERNS) {
        char c = buffer[buffer_pos++];
        
        if (c == '\n' || c == '\r') {
            if (line_pos > 0) {
                line[line_pos] = '\0';
                
                // Ignora comentários e linhas vazias
                if (line[0] != '#' && line[0] != '\0') {
                    if (ac_add_pattern(automaton, line)) {
                        patterns_added++;
                    }
                }
                line_pos = 0;
            }
        } else if (line_pos < MAX_PATTERN_LENGTH - 1) {
            line[line_pos++] = c;
        }
    }
    
    // Processa última linha se não termina com \n
    if (line_pos > 0) {
        line[line_pos] = '\0';
        if (line[0] != '#' && line[0] != '\0') {
            if (ac_add_pattern(automaton, line)) {
                patterns_added++;
            }
        }
    }
    
    return patterns_added;
}

static int load_patterns_inmemory(const char* filename, ac_automaton_t* automaton) {
    char buffer[MAX_FILE_BUFFER];
    
    // Lê arquivo completo de uma vez - SEM FILE* persistente
    if (read_file_complete(filename, buffer, sizeof(buffer)) != 0) {
        return 0;
    }
    
    // Parse in-memory
    return parse_patterns_from_buffer(buffer, automaton);
}

static bool process_messages_inmemory(const char* input_filename, const char* output_filename,
                                     ac_automaton_t* filter, int* total_processed, int* total_blocked) {
    char input_buffer[MAX_FILE_BUFFER];
    char output_buffer[MAX_FILE_BUFFER];
    
    // 1. Lê input completo - SEM FILE* persistente
    if (read_file_complete(input_filename, input_buffer, sizeof(input_buffer)) != 0) {
        return false;
    }
    
    // 2. Processa in-memory
    char line[MAX_LINE_LENGTH];
    int message_count = 0;
    int blocked_count = 0;
    int buffer_pos = 0;
    int line_pos = 0;
    int output_pos = 0;
    
    // Header
    output_pos += snprintf(output_buffer + output_pos, MAX_FILE_BUFFER - output_pos,
                          "=== RESULTADO FILTRO AHO-CORASICK ===\n\n");
    
    // Processa linha por linha in-memory
    while (input_buffer[buffer_pos] && output_pos < MAX_FILE_BUFFER - 100) {
        char c = input_buffer[buffer_pos++];
        
        if (c == '\n' || c == '\r') {
            if (line_pos > 0) {
                line[line_pos] = '\0';
                message_count++;
                
                bool is_blocked = is_message_forbidden(filter, line);
                if (is_blocked) blocked_count++;
                
                output_pos += snprintf(output_buffer + output_pos, MAX_FILE_BUFFER - output_pos,
                                     "[%03d] %s: %s\n", 
                                     message_count,
                                     is_blocked ? "**SPAM**" : "CLEAN",
                                     line);
                line_pos = 0;
            }
        } else if (line_pos < MAX_LINE_LENGTH - 1) {
            line[line_pos++] = c;
        }
    }
    
    // Stats finais
    output_pos += snprintf(output_buffer + output_pos, MAX_FILE_BUFFER - output_pos,
                          "\n=== ESTATÍSTICAS ===\nTotal: %d\nSpam: %d\nTaxa: %.1f%%\n",
                          message_count, blocked_count,
                          message_count > 0 ? (100.0 * blocked_count) / message_count : 0.0);
    
    // 3. Escreve output - SEM FILE* persistente
    bool write_ok = (write_file_complete(output_filename, output_buffer) == 0);
    
    *total_processed = message_count;
    *total_blocked = blocked_count;
    
    return write_ok;
}

int main() {
    DEBUG_PRINTF("=== FILTRO AHO-CORASICK ULTRA-COMPACTO ===\n");

    ac_automaton_t content_filter;
    ac_initialize_automaton(&content_filter);

    // Carrega padrões in-memory
    int patterns_added = load_patterns_inmemory(PATTERNS_FILE, &content_filter);
    if (patterns_added == 0) {
        DEBUG_PRINTF("ERRO: Nenhum padrão carregado!\n");
        return 1;
    }

    ac_build_automaton(&content_filter);
    
    DEBUG_PRINTF("Automaton: %d padrões, %d vértices\n", 
           ac_get_pattern_count(&content_filter), 
           ac_get_vertex_count(&content_filter));

    // Processa mensagens in-memory
    int total_processed = 0;
    int total_blocked = 0;
    
    bool processing_ok = process_messages_inmemory(INPUT_FILE, OUTPUT_FILE, 
                                                  &content_filter, 
                                                  &total_processed, &total_blocked);
    
    if (processing_ok) {
        DEBUG_PRINTF("Processamento OK: %d mensagens, %d spam (%.1f%%)\n", 
               total_processed, total_blocked,
               total_processed > 0 ? (100.0 * total_blocked) / total_processed : 0.0);
    } else {
        DEBUG_PRINTF("ERRO no processamento!\n");
        return 1;
    }
    
    return 0;
}