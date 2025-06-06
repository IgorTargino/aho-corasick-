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
    
    if (read_file_complete(input_filename, input_buffer, sizeof(input_buffer)) != 0) {
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    int message_count = 0;
    int blocked_count = 0;
    int buffer_pos = 0;
    int line_pos = 0;
    int output_pos = 0;
    
    output_pos += snprintf(output_buffer + output_pos, MAX_FILE_BUFFER - output_pos,
                          "=== RESULTADO FILTRO AHO-CORASICK ===\n\n");
    
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

static int write_execution_report(const char* report_filename, 
                                 const ac_automaton_t* automaton,
                                 int patterns_loaded,
                                 int total_processed, 
                                 int total_blocked,
                                 bool processing_success) {
    char report_buffer[MAX_FILE_BUFFER];
    int pos = 0;
    
    // Cabeçalho do relatório
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "========================================\n"
                   "   RELATÓRIO DE EXECUÇÃO AHO-CORASICK\n"
                   "========================================\n\n");
    
    // Informações do autômato
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "📊 CONFIGURAÇÃO DO AUTÔMATO:\n"
                   "  • Padrões carregados: %d\n"
                   "  • Vértices criados: %d\n"
                   "  • Máximo de padrões: %d\n"
                   "  • Máximo de vértices: %d\n\n",
                   patterns_loaded,
                   automaton->current_vertex_count,
                   AC_MAX_PATTERNS,
                   AC_MAX_VERTICES);
    
    // Status da execução
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "⚙️  STATUS DA EXECUÇÃO:\n"
                   "  • Processamento: %s\n"
                   "  • Mensagens processadas: %d\n"
                   "  • Mensagens bloqueadas: %d\n",
                   processing_success ? "✅ SUCESSO" : "❌ FALHOU",
                   total_processed,
                   total_blocked);
    
    // Estatísticas detalhadas
    if (processing_success && total_processed > 0) {
        double spam_rate = (100.0 * total_blocked) / total_processed;
        double clean_rate = 100.0 - spam_rate;
        
        pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                       "  • Taxa de spam: %.2f%%\n"
                       "  • Taxa de limpas: %.2f%%\n",
                       spam_rate, clean_rate);
        
        // Classificação da eficácia
        const char* effectiveness;
        if (spam_rate == 0.0) {
            effectiveness = "📗 CONTEÚDO COMPLETAMENTE LIMPO";
        } else if (spam_rate < 10.0) {
            effectiveness = "📘 BAIXO NÍVEL DE SPAM";
        } else if (spam_rate < 30.0) {
            effectiveness = "📙 NÍVEL MODERADO DE SPAM";
        } else {
            effectiveness = "📕 ALTO NÍVEL DE SPAM";
        }
        
        pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                       "  • Classificação: %s\n", effectiveness);
    }
    
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos, "\n");
    
    // Informações técnicas
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "🔧 INFORMAÇÕES TÉCNICAS:\n"
                   "  • Buffer de arquivo: %d bytes\n"
                   "  • Tamanho máximo de linha: %d chars\n"
                   "  • Tamanho máximo de padrão: %d chars\n"
                   "  • Uso de memória: Estático (8KB limite)\n\n",
                   MAX_FILE_BUFFER,
                   MAX_LINE_LENGTH,
                   MAX_PATTERN_LENGTH);
    
    // Detalhes dos arquivos processados
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "📁 ARQUIVOS PROCESSADOS:\n"
                   "  • Padrões: %s\n"
                   "  • Entrada: %s\n"
                   "  • Saída: %s\n"
                   "  • Relatório: %s\n\n",
                   PATTERNS_FILE,
                   INPUT_FILE,
                   OUTPUT_FILE,
                   report_filename);
    
    // Lista dos padrões carregados (se houver espaço)
    if (patterns_loaded > 0 && pos < MAX_FILE_BUFFER - 200) {
        pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                       "🔍 PADRÕES CARREGADOS (%d):\n", patterns_loaded);
        
        for (int i = 0; i < patterns_loaded && i < automaton->num_total_patterns && 
             pos < MAX_FILE_BUFFER - 100; i++) {
            pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                           "  %2d. \"%s\"\n", i + 1, automaton->patterns[i]);
        }
        
        if (patterns_loaded > automaton->num_total_patterns) {
            pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                           "  ... (lista truncada)\n");
        }
        pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos, "\n");
    }
    
    // Rodapé com timestamp simulado
    pos += snprintf(report_buffer + pos, MAX_FILE_BUFFER - pos,
                   "========================================\n"
                   "Relatório gerado pelo Filtro Aho-Corasick\n"
                   "Versão: Ultra-Compacta v1.0\n"
                   "========================================\n");
    
    // Escrever o relatório
    DEBUG_PRINTF("📝 Gerando relatório: %s\n", report_filename);
    
    if (write_file_complete(report_filename, report_buffer) == 0) {
        DEBUG_PRINTF("✅ Relatório gerado com sucesso (%d bytes)\n", pos);
        return 0;
    } else {
        DEBUG_PRINTF("❌ Erro ao gerar relatório\n");
        return 1;
    }
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
    

	write_execution_report(REPORT_FILE, &content_filter, patterns_added, total_processed, total_blocked,  processing_ok);
	
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