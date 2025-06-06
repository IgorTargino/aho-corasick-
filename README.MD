# **Aho-Corasick String Matching Algorithm - Sistema Embarcado** 🔍

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Memory](https://img.shields.io/badge/Memory-8KB_Limit-orange.svg)](#memory-requirements)
[![Build](https://img.shields.io/badge/Build-Make-red.svg)](#compilação-e-execução)

Uma implementação otimizada do algoritmo **Aho-Corasick** para detecção de múltiplos padrões de texto simultaneamente, projetada especificamente para **sistemas embarcados** com restrições rigorosas de memória (8KB).

## **📋 Índice**

- [Sobre o Algoritmo](#sobre-o-algoritmo)
- [Características da Implementação](#características-da-implementação)
- [Arquitetura do Sistema](#arquitetura-do-sistema)
- [Requisitos de Memória](#requisitos-de-memória)
- [Configuração e Instalação](#configuração-e-instalação)
- [Compilação e Execução](#compilação-e-execução)
- [Uso da API](#uso-da-api)
- [Casos de Teste](#casos-de-teste)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Exemplos de Uso](#exemplos-de-uso)
- [Performance](#performance)
- [Limitações](#limitações)
- [Contribuição](#contribuição)
- [Licença](#licença)

## **🧠 Sobre o Algoritmo**

O **Aho-Corasick** é um algoritmo de busca de strings que permite encontrar todas as ocorrências de múltiplos padrões em um texto de forma eficiente. É amplamente utilizado em:

- **Filtros de conteúdo** (spam, malware, palavras proibidas)
- **Sistemas de detecção de intrusão** (IDS)
- **Análise de logs** e monitoramento
- **Processamento de texto** em tempo real

### **Complexidade Algorítmica**
- **Pré-processamento:** O(Σ|padrões|) - construção do autômato
- **Busca:** O(|texto| + |ocorrências|) - busca no texto
- **Espaço:** O(Σ|padrões|) - memória para o autômato

## **✨ Características da Implementação**

### **🎯 Otimizada para Sistemas Embarcados**
- **Limite rigoroso de 8KB** de memória total
- **Sem alocação dinâmica** - apenas arrays estáticos
- **Configuração em tempo de compilação**
- **Overhead mínimo** de processamento

### **🔧 Funcionalidades**
- ✅ **Busca case-insensitive** (A-Z, a-z)
- ✅ **Múltiplos padrões simultâneos** (até 30 padrões)
- ✅ **Callback customizável** para matches
- ✅ **Validação robusta** de entrada
- ✅ **Debug condicional** (removível em produção)
- ✅ **API simples e intuitiva**

### **⚡ Otimizações**
- **Mapeamento inline** de caracteres
- **Inicialização eficiente** com `memset`
- **Verificações de limites** em tempo de compilação
- **Transições pré-computadas** para busca rápida

## **🏗️ Arquitetura do Sistema**

```
┌─────────────────────────────────────────────────────────────┐
│                    Aho-Corasick System                     │
├─────────────────────────────────────────────────────────────┤
│  Application Layer (main.c)                                │
│  ├─ Content Filter                                         │
│  ├─ Pattern Management                                     │
│  └─ Memory Usage Monitoring                                │
├─────────────────────────────────────────────────────────────┤
│  API Layer (aho_corasick.h)                               │
│  ├─ ac_initialize_automaton()                             │
│  ├─ ac_add_pattern()                                       │
│  ├─ ac_build_automaton()                                   │
│  └─ ac_search()                                            │
├─────────────────────────────────────────────────────────────┤
│  Core Implementation (aho_corasick.c)                     │
│  ├─ Trie Construction                                      │
│  ├─ Failure Link Computation                              │
│  ├─ Go Function (Transitions)                             │
│  └─ Pattern Matching Engine                               │
├─────────────────────────────────────────────────────────────┤
│  Supporting Components                                      │
│  ├─ Queue (aho_queue.c) - BFS for construction            │
│  └─ Configuration (aho_config.h) - Memory limits          │
└─────────────────────────────────────────────────────────────┘
```

## **💾 Requisitos de Memória**

### **Orçamento Total: 8KB (8192 bytes)**

| Componente | Tamanho | Descrição |
|------------|---------|-----------|
| **Autômato Principal** | ~7130B | Estrutura `ac_automaton_t` |
| **Buffer de Entrada** | 512B | Texto para análise |
| **Buffers Auxiliares** | 256B | Outros usos da aplicação |
| **Margem de Segurança** | ~294B | Stack e variáveis locais |

### **Detalhamento do Autômato (7130 bytes)**

```c
// Cálculo detalhado de memória:
Vértices: 30 * 229 bytes = 6870 bytes
├─ next[26]: 26 * 4 = 104 bytes por vértice
├─ go[26]: 26 * 4 = 104 bytes por vértice  
├─ link: 4 bytes por vértice
├─ output: 1 byte por vértice
├─ pattern_indices[2]: 2 * 4 = 8 bytes por vértice
└─ num_patterns_at_vertex: 4 bytes por vértice
   + padding: ~4 bytes por vértice

Padrões: 30 * 4 bytes = 120 bytes (ponteiros)
Fila BFS: 30 * 4 + 12 bytes = 132 bytes
Controle: 2 * 4 bytes = 8 bytes

Total: 6870 + 120 + 132 + 8 = 7130 bytes
```

### **Configuração de Limites**

```c
#define AC_MAX_VERTICES             30    // Máximo de vértices
#define AC_MAX_PATTERNS             30    // Máximo de padrões  
#define AC_MAX_PATTERNS_PER_VERTEX  2     // Padrões por vértice
#define AC_K_ALPHABET_SIZE          26    // A-Z, a-z
```

## **⚙️ Configuração e Instalação**

### **Pré-requisitos**
- **GCC** (ou compilador C compatível)
- **Make** (para automação de build)
- **Sistema Linux/Unix** (testado em Ubuntu/Debian)

### **Clone do Repositório**
```bash
git clone <repository-url>
cd aho-corasick
```

### **Estrutura de Diretórios**
```
aho-corasick/
├── include/          # Cabeçalhos (.h)
│   ├── aho_corasick.h
│   ├── aho_queue.h
│   └── aho_config.h
├── src/              # Código fonte (.c)
│   ├── main.c
│   ├── aho_corasick.c
│   ├── aho_queue.c
│   └── test_aho_corasick.c
├── build/            # Arquivos compilados (gerado)
├── Makefile          # Automação de build
└── README.md         # Esta documentação
```

## **🔨 Compilação e Execução**

### **Comandos Disponíveis**

```bash
# Compilação
make debug          # Versão debug (com logs detalhados)
make release        # Versão otimizada (produção)
make test           # Compila suite de testes

# Execução
make run            # Executa aplicação (modo debug)
make run_release    # Executa aplicação (modo release)
make run_test       # Executa todos os testes

# Limpeza
make clean          # Remove arquivos compilados
```

### **Exemplo de Uso Básico**

```bash
# Compilar e executar filtro de conteúdo
make run

# Saída esperada:
=== Filtro de Conteudo - Aho-Corasick ===

Configurando filtro de conteudo...
  ✅ Padrao adicionado: "spam"
  ✅ Padrao adicionado: "virus"
  ✅ Padrao adicionado: "hack"
  ...

✅ Automato construido com sucesso!

--- Estimativa de Uso de Memoria ---
  Automato (ac_automaton_t): 7130 bytes
  Total estimado: 7898 bytes (7.71 KB)
  ✅ Dentro do orcamento de memoria

=== Teste do Filtro de Conteudo ===
[1] Analisando: "esta e uma mensagem normal e segura"
    ✅ Resultado: Mensagem Permitida

[2] Analisando: "alerta um virus foi detectado"
    🚫 Resultado: MENSAGEM BLOQUEADA!
```

### **Executando Testes**

```bash
make run_test

# Saída esperada:
=== SUITE DE TESTES - AHO-CORASICK ===

[TEST 1] Inicializacao do Automato
  ✅ PASS: Root vertex deve ser criado
  ✅ PASS: Nenhum padrao inicial

[TEST 2] Adicao de Padroes
  ✅ PASS: Deve adicionar padrao valido
  ✅ PASS: Deve rejeitar padrao vazio

...

=== RESUMO DOS TESTES ===
Total de testes: 14
Testes passaram: 45
Testes falharam: 0
Taxa de sucesso: 100.0%
✅ TODOS OS TESTES PASSARAM
```

## **📚 Uso da API**

### **Inicialização**

```c
#include "aho_corasick.h"

// Criar e inicializar autômato
ac_automaton_t automaton;
ac_initialize_automaton(&automaton);
```

### **Adição de Padrões**

```c
// Adicionar padrões (antes de construir o autômato)
bool success1 = ac_add_pattern(&automaton, "spam");
bool success2 = ac_add_pattern(&automaton, "virus");
bool success3 = ac_add_pattern(&automaton, "malware");

// Verificar se foram adicionados com sucesso
if (!success1 || !success2 || !success3) {
    printf("Erro ao adicionar alguns padrões\n");
}
```

### **Construção do Autômato**

```c
// Construir autômato (após adicionar todos os padrões)
ac_build_automaton(&automaton);

// Validar se foi construído corretamente
if (!ac_is_built(&automaton)) {
    printf("Erro: Falha na construção do autômato\n");
    return -1;
}
```

### **Implementação do Callback**

```c
// Callback chamado quando um padrão é encontrado
void ac_set_match_callback(const char* pattern, int position) {
    printf("Padrão '%s' encontrado na posição %d\n", pattern, position);
    
    // Sua lógica aqui (ex: logging, bloqueio, etc.)
    take_action_on_forbidden_content(pattern, position);
}
```

### **Busca em Texto**

```c
// Buscar padrões no texto
const char* text = "Esta mensagem contém spam e virus";
ac_search(&automaton, text);

// O callback será chamado automaticamente para cada match
```

### **Monitoramento de Recursos**

```c
// Verificar uso de recursos
printf("Vértices utilizados: %d/%d\n", 
       ac_get_vertex_count(&automaton), AC_MAX_VERTICES);
       
printf("Padrões carregados: %d/%d\n",
       ac_get_pattern_count(&automaton), AC_MAX_PATTERNS);

printf("Memória estimada: %zu bytes\n", sizeof(ac_automaton_t));
```

## **🧪 Casos de Teste**

### **Cobertura de Testes**

| Categoria | Testes | Descrição |
|-----------|--------|-----------|
| **Inicialização** | 3 testes | Estado inicial, alocação de root |
| **Adição de Padrões** | 8 testes | Padrões válidos/inválidos, limites |
| **Construção** | 4 testes | Autômato vazio/com padrões |
| **Busca Básica** | 6 testes | Padrão único, múltiplos padrões |
| **Case Sensitivity** | 3 testes | Maiúsculas/minúsculas |
| **Casos Extremos** | 8 testes | Caracteres inválidos, texto vazio |
| **Performance** | 5 testes | Padrões sobrepostos, repetidos |
| **Memória** | 4 testes | Uso de recursos, limites |

### **Testes de Limite**

```c
// Teste de limite de padrões
void test_pattern_limit() {
    // Adiciona AC_MAX_PATTERNS + 5 padrões
    // Verifica que apenas AC_MAX_PATTERNS são aceitos
}

// Teste de limite de vértices  
void test_vertex_limit() {
    // Adiciona padrões muito longos
    // Verifica que não excede AC_MAX_VERTICES
}
```

### **Testes de Funcionalidade**

```c
// Teste de busca com padrões sobrepostos
void test_multiple_patterns_search() {
    // Padrões: "he", "she", "his", "hers"
    // Texto: "ushers"
    // Deve encontrar: "he", "she", "hers"
}

// Teste case-insensitive
void test_case_insensitive_search() {
    // Padrão: "test"
    // Texto: "TEST", "Test", "tEsT"
    // Deve encontrar em todos os casos
}
```

## **📁 Estrutura do Projeto**

### **Arquivos Principais**

| Arquivo | Descrição |
|---------|-----------|
| `include/aho_corasick.h` | API principal e estruturas |
| `src/aho_corasick.c` | Implementação core do algoritmo |
| `src/main.c` | Aplicação de exemplo (filtro de conteúdo) |
| `src/test_aho_corasick.c` | Suite completa de testes |
| `include/aho_queue.h` | Fila para construção BFS |
| `src/aho_queue.c` | Implementação da fila |

### **Arquivos de Configuração**

| Arquivo | Descrição |
|---------|-----------|
| `Makefile` | Automação de build e testes |
| `include/aho_config.h` | Configurações de limite de memória |

## **💡 Exemplos de Uso**

### **1. Filtro de Spam Simples**

```c
#include "aho_corasick.h"

volatile bool is_spam = false;

void ac_set_match_callback(const char* pattern, int position) {
    is_spam = true;
    printf("Spam detectado: '%s'\n", pattern);
}

bool check_email(const char* content) {
    static ac_automaton_t spam_filter;
    static bool initialized = false;
    
    if (!initialized) {
        ac_initialize_automaton(&spam_filter);
        ac_add_pattern(&spam_filter, "viagra");
        ac_add_pattern(&spam_filter, "casino");
        ac_add_pattern(&spam_filter, "lottery");
        ac_build_automaton(&spam_filter);
        initialized = true;
    }
    
    is_spam = false;
    ac_search(&spam_filter, content);
    return !is_spam;
}
```

### **2. Detector de Malware em Logs**

```c
void setup_malware_detector(ac_automaton_t* detector) {
    ac_initialize_automaton(detector);
    
    // Assinaturas de malware conhecidas
    ac_add_pattern(detector, "trojan");
    ac_add_pattern(detector, "rootkit"); 
    ac_add_pattern(detector, "backdoor");
    ac_add_pattern(detector, "keylogger");
    
    ac_build_automaton(detector);
}

void analyze_log_line(ac_automaton_t* detector, const char* log_line) {
    ac_search(detector, log_line);
    // Callback irá processar detecções
}
```

### **3. Filtro de Conteúdo para Chat**

```c
bool is_message_appropriate(const char* message) {
    static ac_automaton_t content_filter;
    static bool filter_ready = false;
    
    if (!filter_ready) {
        ac_initialize_automaton(&content_filter);
        
        // Palavras proibidas
        ac_add_pattern(&content_filter, "hate");
        ac_add_pattern(&content_filter, "violence");
        ac_add_pattern(&content_filter, "inappropriate");
        
        ac_build_automaton(&content_filter);
        filter_ready = true;
    }
    
    global_violation_found = false;
    ac_search(&content_filter, message);
    
    return !global_violation_found;
}
```

## **⚡ Performance**

### **Complexidade Real**

```c
// Para N padrões de tamanho médio M:
Pré-processamento: O(N * M)
Busca em texto T: O(|T| + número_de_matches)
Memória: O(N * M) limitada por AC_MAX_VERTICES
```

## **⚠️ Limitações**

### **Limitações de Design**

1. **Alfabeto Restrito**: Apenas A-Z, a-z (26 caracteres)
2. **Memória Fixa**: 8KB total, não expansível
3. **Sem Unicode**: Não suporta caracteres especiais/acentos
4. **Arrays Estáticos**: Sem alocação dinâmica

### **Limitações Operacionais**

| Limite | Valor | Configurável |
|--------|-------|--------------|
| **Máximo de vértices** | 30 | ✅ `AC_MAX_VERTICES` |
| **Máximo de padrões** | 30 | ✅ `AC_MAX_PATTERNS` |
| **Padrões por vértice** | 2 | ✅ `AC_MAX_PATTERNS_PER_VERTEX` |
| **Tamanho do alfabeto** | 26 | ✅ `AC_K_ALPHABET_SIZE` |

### **Quando NÃO Usar**

- **Textos com Unicode/UTF-8** (acentos, caracteres especiais)
- **Sistemas com > 1000 padrões** (limite de memória)
- **Padrões muito longos** (podem esgotar vértices)
- **Aplicações que requerem regex** (apenas matching exato)

## **🔧 Configuração Avançada**

### **Ajustando Limites de Memória**

```c
// No arquivo aho_corasick.h, ajuste conforme necessário:

// Para mais padrões (consome mais memória):
// Para padrões mais longos (consome mais vértices):
#define AC_MAX_PATTERNS 50
#define AC_MAX_VERTICES 100

// Para múltiplos padrões por final de palavra:
#define AC_MAX_PATTERNS_PER_VERTEX 5
```

### **Habilitando Debug**

```bash
# Compilação com debug detalhado
make debug

# Ou definir manualmente:
gcc -DAC_DEBUG_PRINTS ...
```

### **Otimização para Produção**

```bash
# Compilação otimizada (sem debug)
make release

# Flags aplicadas automaticamente:
# -O2 (otimização de performance)
# Sem -DAC_DEBUG_PRINTS (remove logs)
```

## **📄 Licença**

Este projeto está licenciado sob a **MIT License** - veja o arquivo [LICENSE](LICENSE) para detalhes.

---