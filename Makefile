# Nome do Compilador
CC = gcc

# Flags do Compilador Comuns
CFLAGS_COMMON = -Wall -Wextra -std=c99 -g
CFLAGS_DEBUG_SPECIFIC = -DAC_DEBUG_PRINTS
CFLAGS_RELEASE_SPECIFIC = -O2

# Diretório de Inclusão de Cabeçalhos
INCLUDE_DIR = ./include

# Diretório dos Arquivos Fonte
SRC_DIR = ./src

# Diretório de Saída para Arquivos Compilados
BUILD_DIR = ./build

# --- Configurações da Aplicação Principal ---
APP_EXECUTAVEL_NAME = filtro_conteudo_app
APP_EXECUTAVEL = $(BUILD_DIR)/$(APP_EXECUTAVEL_NAME)
APP_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c
APP_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(APP_SOURCES:.c=.o)))

# --- Configurações do Runner de Testes ---
TEST_EXECUTAVEL_NAME = test_runner
TEST_EXECUTAVEL = $(BUILD_DIR)/$(TEST_EXECUTAVEL_NAME)
# O teste usa a biblioteca aho_corasick e aho_queue, mais seu próprio main (test_aho_corasick.c)
TEST_SOURCES = $(SRC_DIR)/test_aho_corasick.c $(SRC_DIR)/aho_corasick.c $(SRC_DIR)/aho_queue.c
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))


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
	@echo "Use './$@' para executar."

# --- Regras para os Testes ---
# Compila o runner de testes (sempre com flags de debug, incluindo AC_DEBUG_PRINTS)
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

# --- Regras Genéricas e Auxiliares ---

# Regra para criar o diretório de build, se não existir
$(BUILD_DIR):
	@echo "Criando diretorio de build: $(BUILD_DIR)..."
	@mkdir -p $(BUILD_DIR)

# Regra genérica para compilar arquivos .c em arquivos .o dentro do BUILD_DIR
# Esta regra será usada tanto pela aplicação principal quanto pelos testes
# para os arquivos .c compartilhados (aho_corasick.c, aho_queue.c) e os específicos.
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

# --- Regra de Limpeza ---
clean:
	@echo "----------------------------------------------------"
	@echo "Limpando diretorio de build: $(BUILD_DIR)..."
	@echo "----------------------------------------------------"
	rm -rf $(BUILD_DIR)
	@echo "Limpeza concluida."

# Phony targets: Alvos que não são nomes de arquivos.
.PHONY: all clean debug release test run run_release run_test $(BUILD_DIR)
