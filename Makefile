# Nome do Compilador
CC = gcc

# Flags do Compilador Comuns
CFLAGS_COMMON = -Wall -Wextra -std=c99 -g
CFLAGS_DEBUG_SPECIFIC = -DDEBUG_PRINTS
CFLAGS_RELEASE_SPECIFIC = -O2

# Diretório de Inclusão de Cabeçalhos
INCLUDE_DIR = ./include

# Diretório dos Arquivos Fonte
SRC_DIR = ./src

# Diretório de Saída para Arquivos Compilados
BUILD_DIR = ./build

# Diretório para Relatórios de Análise
REPORTS_DIR = ./reports

# --- Configurações da Aplicação Principal ---
APP_EXECUTAVEL_NAME = filtro_conteudo_app
APP_EXECUTAVEL = $(BUILD_DIR)/$(APP_EXECUTAVEL_NAME)
APP_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c
APP_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(APP_SOURCES:.c=.o)))

# --- Configurações da Aplicação de Medição ---
PERF_APP_EXECUTAVEL_NAME = aho_perf_app
PERF_APP_EXECUTAVEL = $(BUILD_DIR)/$(PERF_APP_EXECUTAVEL_NAME)
PERF_APP_SOURCES = $(SRC_DIR)/aho_app.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c $(SRC_DIR)/aho_app_config.c
PERF_APP_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(PERF_APP_SOURCES:.c=.o)))

# --- Configurações do Multi-Teste ---
MULTI_TEST_EXECUTAVEL_NAME = aho_multi_test
MULTI_TEST_EXECUTAVEL = $(BUILD_DIR)/$(MULTI_TEST_EXECUTAVEL_NAME)
MULTI_TEST_SOURCES = $(SRC_DIR)/aho_multi_test.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c $(SRC_DIR)/aho_app_config.c
MULTI_TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(MULTI_TEST_SOURCES:.c=.o)))

# --- Configurações do Exemplo STM32F0 ---
STM32_EXAMPLE_EXECUTAVEL_NAME = stm32f0_example
STM32_EXAMPLE_EXECUTAVEL = $(BUILD_DIR)/$(STM32_EXAMPLE_EXECUTAVEL_NAME)
STM32_EXAMPLE_SOURCES = $(SRC_DIR)/stm32f0_example.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c
STM32_EXAMPLE_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(STM32_EXAMPLE_SOURCES:.c=.o)))

# --- Configurações do Runner de Testes ---
TEST_EXECUTAVEL_NAME = test_runner
TEST_EXECUTAVEL = $(BUILD_DIR)/$(TEST_EXECUTAVEL_NAME)
TEST_SOURCES = $(SRC_DIR)/test_aho_corasick.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))

# --- Configurações do Valgrind ---
TIMESTAMP = $(shell date +%Y%m%d_%H%M%S)
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes
VALGRIND_MASSIF_FLAGS = --tool=massif --stacks=yes --detailed-freq=1 --massif-out-file=$(REPORTS_DIR)/massif_$(TIMESTAMP).out
VALGRIND_CALLGRIND_FLAGS = --tool=callgrind --collect-jumps=yes --collect-systime=yes --callgrind-out-file=$(REPORTS_DIR)/callgrind_$(TIMESTAMP).out

# --- Regras do Makefile ---

# Regra padrão: compila a aplicação principal para depuração
all: debug

# --- Regras para a Aplicação Principal ---
debug: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG_SPECIFIC) -I$(INCLUDE_DIR)
debug: $(APP_EXECUTAVEL)

release: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_RELEASE_SPECIFIC) -I$(INCLUDE_DIR)
release: $(APP_EXECUTAVEL)

$(APP_EXECUTAVEL): $(APP_OBJECTS) | $(BUILD_DIR)
	@echo "----------------------------------------------------"
	@echo "Linkando a aplicacao principal: $@"
	@echo "----------------------------------------------------"
	$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "$@ compilado com sucesso!"
	@echo "Use 'make run' para executar."

# --- Regras para os Testes ---
test: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG_SPECIFIC) -I$(INCLUDE_DIR)
test: $(TEST_EXECUTAVEL)

$(TEST_EXECUTAVEL): $(TEST_OBJECTS) | $(BUILD_DIR)
	@echo "----------------------------------------------------"
	@echo "Linkando o runner de testes: $@"
	@echo "----------------------------------------------------"
	$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "$@ compilado com sucesso!"
	@echo "Use 'make run_test' para executar os testes."

# --- Regras para a Aplicação de Performance ---
perf_app: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG_SPECIFIC) -I$(INCLUDE_DIR)
perf_app: $(PERF_APP_EXECUTAVEL)

$(PERF_APP_EXECUTAVEL): $(PERF_APP_OBJECTS) | $(BUILD_DIR)
	@echo "----------------------------------------------------"
	@echo "Linkando aplicacao de performance: $@"
	@echo "----------------------------------------------------"
	$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "$@ compilado com sucesso!"
	@echo "Use 'make run_perf' para executar."

perf_app_release: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_RELEASE_SPECIFIC) -I$(INCLUDE_DIR)
perf_app_release: $(PERF_APP_EXECUTAVEL)

# --- Regras para o Multi-Teste ---
multi_test: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG_SPECIFIC) -I$(INCLUDE_DIR)
multi_test: $(MULTI_TEST_EXECUTAVEL)

$(MULTI_TEST_EXECUTAVEL): $(MULTI_TEST_OBJECTS) | $(BUILD_DIR)
	@echo "----------------------------------------------------"
	@echo "Linkando multi-teste: $@"
	@echo "----------------------------------------------------"
	$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "$@ compilado com sucesso!"
	@echo "Use 'make run_multi_test' para executar."

# --- Regras para o Exemplo STM32F0 ---
stm32_example: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG_SPECIFIC) -I$(INCLUDE_DIR)
stm32_example: $(STM32_EXAMPLE_EXECUTAVEL)

$(STM32_EXAMPLE_EXECUTAVEL): $(STM32_EXAMPLE_OBJECTS) | $(BUILD_DIR)
	@echo "----------------------------------------------------"
	@echo "Linkando exemplo STM32F0: $@"
	@echo "----------------------------------------------------"
	$(CC) $(CFLAGS) $^ -o $@
	@echo ""
	@echo "$@ compilado com sucesso!"
	@echo "Use 'make run_stm32_example' para executar."

# --- Regras Genéricas e Auxiliares ---

# Regra para criar o diretório de build, se não existir
$(BUILD_DIR):
	@echo "Criando diretorio de build: $(BUILD_DIR)..."
	@mkdir -p $(BUILD_DIR)

# Regra para criar o diretório de relatórios, se não existir
$(REPORTS_DIR):
	@echo "Criando diretorio de relatorios: $(REPORTS_DIR)..."
	@mkdir -p $(REPORTS_DIR)

# Regra genérica para compilar arquivos .c em arquivos .o dentro do BUILD_DIR
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compilando $< para $@ ..."
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# --- Regras de Execução ---
run: debug
	@echo "----------------------------------------------------"
	@echo "Executando aplicacao (debug): $(APP_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(APP_EXECUTAVEL)

run_release: release
	@echo "----------------------------------------------------"
	@echo "Executando aplicacao (release): $(APP_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(APP_EXECUTAVEL)

run_test: test
	@echo "----------------------------------------------------"
	@echo "Executando testes: $(TEST_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(TEST_EXECUTAVEL)

run_perf: perf_app
	@echo "----------------------------------------------------"
	@echo "Executando aplicacao de performance: $(PERF_APP_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(PERF_APP_EXECUTAVEL)

run_perf_release: perf_app_release
	@echo "----------------------------------------------------"
	@echo "Executando aplicacao de performance (release): $(PERF_APP_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(PERF_APP_EXECUTAVEL)

run_multi_test: multi_test
	@echo "----------------------------------------------------"
	@echo "Executando multi-teste: $(MULTI_TEST_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(MULTI_TEST_EXECUTAVEL)

run_stm32_example: stm32_example
	@echo "----------------------------------------------------"
	@echo "Executando exemplo STM32F0: $(STM32_EXAMPLE_EXECUTAVEL)"
	@echo "----------------------------------------------------"
	$(STM32_EXAMPLE_EXECUTAVEL)

# --- Regras de Análise de Memória com Valgrind ---

# Verifica se o Valgrind está instalado
check_valgrind:
	@which valgrind > /dev/null || (echo "❌ Erro: Valgrind nao encontrado!" && \
	echo "   Para instalar: sudo apt-get install valgrind" && \
	echo "   Ou: sudo yum install valgrind" && exit 1)
	@echo "✅ Valgrind encontrado: $$(valgrind --version)"

# Análise básica de vazamentos de memória
valgrind: debug check_valgrind | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "🔍 VALGRIND - Análise de Vazamentos de Memória"
	@echo "===================================================="
	@echo "Executando: $(APP_EXECUTAVEL)"
	@echo "Flags: $(VALGRIND_FLAGS)"
	@echo "Relatório: $(REPORTS_DIR)/valgrind_leak_$(TIMESTAMP).log"
	@echo "----------------------------------------------------"
	valgrind $(VALGRIND_FLAGS) $(APP_EXECUTAVEL) 2>&1 | tee $(REPORTS_DIR)/valgrind_leak_$(TIMESTAMP).log
	@echo ""
	@echo "✅ Análise de vazamentos concluída"
	@echo "📄 Relatório salvo em: $(REPORTS_DIR)/valgrind_leak_$(TIMESTAMP).log"

# Análise detalhada de uso de memória (Massif)
valgrind_memory: debug check_valgrind | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "📊 VALGRIND MASSIF - Análise Detalhada de Memória"
	@echo "===================================================="
	@echo "Executando: $(APP_EXECUTAVEL)"
	@echo "Flags: $(VALGRIND_MASSIF_FLAGS)"
	@echo "Arquivo de saída: $(REPORTS_DIR)/massif_$(TIMESTAMP).out"
	@echo "----------------------------------------------------"
	valgrind $(VALGRIND_MASSIF_FLAGS) $(APP_EXECUTAVEL)
	@echo ""
	@echo "✅ Análise de memória concluída"
	@echo "📈 Para visualizar o relatório execute:"
	@echo "   ms_print $(REPORTS_DIR)/massif_$(TIMESTAMP).out"
	@echo "   ou: make show_massif_report"

# Análise de performance e chamadas (Callgrind)
valgrind_profile: debug check_valgrind | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "⚡ VALGRIND CALLGRIND - Análise de Performance"
	@echo "===================================================="
	@echo "Executando: $(APP_EXECUTAVEL)"
	@echo "Flags: $(VALGRIND_CALLGRIND_FLAGS)"
	@echo "Arquivo de saída: $(REPORTS_DIR)/callgrind_$(TIMESTAMP).out"
	@echo "----------------------------------------------------"
	valgrind $(VALGRIND_CALLGRIND_FLAGS) $(APP_EXECUTAVEL)
	@echo ""
	@echo "✅ Análise de performance concluída"
	@echo "📊 Para visualizar o relatório execute:"
	@echo "   kcachegrind $(REPORTS_DIR)/callgrind_$(TIMESTAMP).out (GUI)"
	@echo "   ou: callgrind_annotate $(REPORTS_DIR)/callgrind_$(TIMESTAMP).out (terminal)"

# Visualizar relatório do Massif mais recente
show_massif_report:
	@echo "===================================================="
	@echo "📈 Visualizando Relatório Massif Mais Recente"
	@echo "===================================================="
	@if ls $(REPORTS_DIR)/massif_*.out 1> /dev/null 2>&1; then \
	    LATEST_MASSIF=$$(ls -t $(REPORTS_DIR)/massif_*.out | head -1); \
	    echo "Arquivo mais recente: $$LATEST_MASSIF"; \
	    echo "----------------------------------------------------"; \
	    ms_print $$LATEST_MASSIF; \
	else \
	    echo "❌ Nenhum arquivo massif encontrado em $(REPORTS_DIR)"; \
	    echo "   Execute 'make valgrind_memory' primeiro"; \
	fi

# Listar todos os relatórios disponíveis
list_reports:
	@echo "===================================================="
	@echo "📋 RELATÓRIOS DISPONÍVEIS EM $(REPORTS_DIR)"
	@echo "===================================================="
	@if [ -d "$(REPORTS_DIR)" ]; then \
	    echo "Análises de Vazamentos:"; \
	    ls -la $(REPORTS_DIR)/valgrind_*leak*.log 2>/dev/null || echo "  Nenhum encontrado"; \
	    echo ""; \
	    echo "Análises de Memória (Massif):"; \
	    ls -la $(REPORTS_DIR)/massif_*.out 2>/dev/null || echo "  Nenhum encontrado"; \
	    echo ""; \
	    echo "Análises de Performance (Callgrind):"; \
	    ls -la $(REPORTS_DIR)/callgrind_*.out 2>/dev/null || echo "  Nenhum encontrado"; \
	    echo ""; \
	    echo "Estatísticas do Sistema:"; \
	    ls -la $(REPORTS_DIR)/time_stats_*.log 2>/dev/null || echo "  Nenhum encontrado"; \
	    echo ""; \
	    TOTAL_SIZE=$$(du -sh $(REPORTS_DIR) 2>/dev/null | cut -f1 || echo "0"); \
	    echo "Espaço total usado: $$TOTAL_SIZE"; \
	else \
	    echo "❌ Diretório $(REPORTS_DIR) não existe"; \
	    echo "   Execute alguma análise primeiro"; \
	fi

# Estatísticas do sistema (GNU time)
time_stats: release | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "⏱️  GNU TIME - Estatísticas do Sistema"
	@echo "===================================================="
	@echo "Executando: $(APP_EXECUTAVEL)"
	@echo "Relatório: $(REPORTS_DIR)/time_stats_$(TIMESTAMP).log"
	@echo "----------------------------------------------------"
	/usr/bin/time -v $(APP_EXECUTAVEL) 2>&1 | tee $(REPORTS_DIR)/time_stats_$(TIMESTAMP).log
	@echo ""
	@echo "✅ Estatísticas coletadas"
	@echo "📄 Relatório salvo em: $(REPORTS_DIR)/time_stats_$(TIMESTAMP).log"

# Análise completa (todos os tipos de análise)
analyze_all: debug test | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "🔬 ANÁLISE COMPLETA - Todos os Testes de Memória"
	@echo "===================================================="
	@echo "📁 Relatórios serão salvos em: $(REPORTS_DIR)"
	@echo "⏰ Timestamp da execução: $(TIMESTAMP)"
	@echo ""
	@echo "1/3 - Vazamentos da Aplicação..."
	@$(MAKE) valgrind --no-print-directory
	@echo ""
	@echo "2/3 - Memória da Aplicação..."
	@$(MAKE) valgrind_memory --no-print-directory
	@echo ""
	@echo "3/3 - Estatísticas da Aplicação..."
	@$(MAKE) time_stats --no-print-directory
	@echo ""
	@echo "===================================================="
	@echo "✅ ANÁLISE COMPLETA CONCLUÍDA"
	@echo "===================================================="
	@echo "📋 Relatórios gerados em $(REPORTS_DIR):"
	@echo "   - valgrind_leak_$(TIMESTAMP).log"
	@echo "   - massif_$(TIMESTAMP).out"
	@echo "   - callgrind_$(TIMESTAMP).out"
	@echo "   - time_stats_$(TIMESTAMP).log"
	@echo ""
	@echo "📊 Para visualizar relatórios:"
	@echo "   make list_reports          - Lista todos os relatórios"
	@echo "   make show_massif_report    - Mostra análise de memória"
	@echo "   kcachegrind $(REPORTS_DIR)/callgrind_$(TIMESTAMP).out"

# Gerar relatório resumo de todos os testes
generate_summary_report: | $(REPORTS_DIR)
	@echo "===================================================="
	@echo "📊 GERANDO RELATÓRIO RESUMO"
	@echo "===================================================="
	@SUMMARY_FILE="$(REPORTS_DIR)/summary_report_$(TIMESTAMP).md"; \
	echo "# Relatório Resumo de Análise - $$(date)" > $$SUMMARY_FILE; \
	echo "" >> $$SUMMARY_FILE; \
	echo "## Informações do Sistema" >> $$SUMMARY_FILE; \
	echo "\`\`\`" >> $$SUMMARY_FILE; \
	uname -a >> $$SUMMARY_FILE; \
	echo "\`\`\`" >> $$SUMMARY_FILE; \
	echo "" >> $$SUMMARY_FILE; \
	echo "## Arquivos de Relatório Gerados" >> $$SUMMARY_FILE; \
	if [ -d "$(REPORTS_DIR)" ]; then \
	    find $(REPORTS_DIR) -name "*$(TIMESTAMP)*" -type f | sort >> $$SUMMARY_FILE; \
	fi; \
	echo "" >> $$SUMMARY_FILE; \
	echo "## Resumo de Tamanhos" >> $$SUMMARY_FILE; \
	echo "\`\`\`" >> $$SUMMARY_FILE; \
	ls -lh $(REPORTS_DIR)/*$(TIMESTAMP)* 2>/dev/null >> $$SUMMARY_FILE || echo "Nenhum arquivo encontrado"; \
	echo "\`\`\`" >> $$SUMMARY_FILE; \
	echo "✅ Relatório resumo gerado: $$SUMMARY_FILE"

# --- Regra de Limpeza ---
clean:
	@echo "----------------------------------------------------"
	@echo "Limpando diretorio de build: $(BUILD_DIR)..."
	@echo "----------------------------------------------------"
	rm -rf $(BUILD_DIR)
	@echo "✅ Diretório de build limpo"

# Limpeza completa (inclui relatórios)
clean_all: clean
	@echo "----------------------------------------------------"
	@echo "Limpando relatórios de análise: $(REPORTS_DIR)..."
	@echo "----------------------------------------------------"
	rm -rf $(REPORTS_DIR)
	@echo "✅ Limpeza completa concluída"

# Limpeza seletiva de relatórios antigos (mantém os 5 mais recentes)
clean_old_reports:
	@echo "===================================================="
	@echo "🧹 LIMPANDO RELATÓRIOS ANTIGOS"
	@echo "===================================================="
	@if [ -d "$(REPORTS_DIR)" ]; then \
	    echo "Mantendo os 5 relatórios mais recentes de cada tipo..."; \
	    for type in massif callgrind valgrind_leak valgrind_test_leak time_stats; do \
	        echo "Processando: $$type"; \
	        ls -t $(REPORTS_DIR)/$$type*.* 2>/dev/null | tail -n +6 | xargs -r rm -v; \
	    done; \
	    echo "✅ Limpeza de relatórios antigos concluída"; \
	else \
	    echo "❌ Diretório $(REPORTS_DIR) não existe"; \
	fi

# Help - mostra todos os comandos disponíveis
help:
	@echo "===================================================="
	@echo "🛠️  COMANDOS DISPONÍVEIS DO MAKEFILE"
	@echo "===================================================="
	@echo ""
	@echo "📦 COMPILAÇÃO:"
	@echo "   make debug          - Compila aplicação (modo debug)"
	@echo "   make release        - Compila aplicação (modo release)"
	@echo "   make test           - Compila testes"
	@echo "   make all            - Compila aplicação (debug) [padrão]"
	@echo ""
	@echo "▶️  EXECUÇÃO:"
	@echo "   make run            - Executa aplicação (debug)"
	@echo "   make run_release    - Executa aplicação (release)"
	@echo "   make run_test       - Executa testes"
	@echo ""
	@echo "🔍 ANÁLISE DE MEMÓRIA (VALGRIND):"
	@echo "   make valgrind       - Análise de vazamentos (app)"
	@echo "   make valgrind_memory - Análise detalhada de memória (app)"
	@echo "   make valgrind_profile - Análise de performance (app)"
	@echo ""
	@echo "📊 RELATÓRIOS:"
	@echo "   make show_massif_report - Mostra relatório Massif mais recente"
	@echo "   make list_reports   - Lista todos os relatórios disponíveis"
	@echo "   make generate_summary_report - Gera relatório resumo"
	@echo "   make time_stats     - Estatísticas do sistema (app)"
	@echo ""
	@echo "🔬 ANÁLISE COMPLETA:"
	@echo "   make analyze_all    - Executa todas as análises"
	@echo ""
	@echo "🧹 LIMPEZA:"
	@echo "   make clean          - Remove arquivos compilados"
	@echo "   make clean_all      - Remove compilados + relatórios"
	@echo "   make clean_old_reports - Remove relatórios antigos (mantém 5)"
	@echo ""
	@echo "❓ AJUDA:"
	@echo "   make help           - Mostra esta mensagem"
	@echo ""
	@echo "📁 ESTRUTURA DE DIRETÓRIOS:"
	@echo "   $(BUILD_DIR)/     - Arquivos compilados"
	@echo "   $(REPORTS_DIR)/   - Relatórios de análise"
	@echo ""
	@echo "===================================================="

# Phony targets
.PHONY: all clean clean_all clean_old_reports debug release test run run_release run_test \
	    check_valgrind valgrind valgrind_test valgrind_memory valgrind_memory_test \
	    valgrind_profile show_massif_report list_reports time_stats time_stats_test \
	    analyze_all generate_summary_report help $(BUILD_DIR) $(REPORTS_DIR)