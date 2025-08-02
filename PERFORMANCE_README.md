# Aplicação de Performance Aho-Corasick

Esta aplicação foi desenvolvida para testar e medir a performance do algoritmo Aho-Corasick em diferentes plataformas, com foco especial na adequação para sistemas embarcados como o STM32F0 com 8KB de RAM.

## Estrutura do Projeto

```
├── src/
│   ├── aho_app.c           # Aplicação principal de performance
│   ├── aho_multi_test.c    # Teste multi-cenário
│   ├── aho_app_config.c    # Configurações de cenários
│   ├── aho_app_stm32.c     # Implementações específicas STM32F0
│   ├── aho_corasick.c      # Implementação do algoritmo
│   └── aho_queue.c         # Queue para construção do autômato
├── include/
│   ├── aho_app.h           # Interface da aplicação
│   ├── aho_app_config.h    # Configurações de cenários
│   ├── aho_app_stm32.h     # Interface STM32F0
│   ├── aho_corasick.h      # Interface do algoritmo
│   ├── aho_queue.h         # Interface da queue
│   └── aho_config.h        # Configurações globais
├── reports/                # Relatórios gerados
├── data/                   # Dados de teste
└── run_tests.sh           # Script automatizado de testes
```

## Cenários de Teste

### 1. SCENARIO_MINIMAL
- **Objetivo:** Sistemas extremamente limitados
- **Padrões:** 5 padrões curtos (3-4 chars)
- **Texto:** 64 caracteres máximo
- **Uso de memória:** ~200-300 bytes

### 2. SCENARIO_EMBEDDED
- **Objetivo:** STM32F0 (8KB RAM)
- **Padrões:** 10 padrões (até 8 chars)
- **Texto:** 256 caracteres máximo
- **Uso de memória:** ~500-800 bytes

### 3. SCENARIO_DESKTOP
- **Objetivo:** PC/Workstation
- **Padrões:** 20 padrões (até 16 chars)
- **Texto:** 1024 caracteres máximo
- **Uso de memória:** ~1-2KB

### 4. SCENARIO_STRESS
- **Objetivo:** Teste de limites
- **Padrões:** 30 padrões (até 32 chars)
- **Texto:** 2048 caracteres máximo
- **Uso de memória:** ~3-5KB

## Compilação e Execução

### Compilação

```bash
# Aplicação principal (debug)
make perf_app

# Aplicação principal (release)
make perf_app_release

# Multi-teste de cenários
make multi_test

# Compilar tudo
make all
```

### Execução

```bash
# Executar aplicação principal
make run_perf              # Versão debug
make run_perf_release      # Versão release

# Executar multi-teste
make run_multi_test

# Script automatizado completo
./run_tests.sh
```

## Métricas Coletadas

### Performance
- **Build Time:** Tempo para construir o autômato (microsegundos)
- **Search Time:** Tempo para buscar padrões no texto (microsegundos)
- **Throughput:** Caracteres processados por segundo
- **Total Time:** Tempo total de execução

### Memória
- **Automaton Size:** Tamanho da estrutura do autômato
- **Pattern Storage:** Memória usada para armazenar padrões
- **Vertex Count:** Número de vértices no autômato
- **Memory Efficiency:** Padrões por KB de memória

### Funcionalidade
- **Patterns Added:** Número de padrões adicionados com sucesso
- **Matches Found:** Número de correspondências encontradas
- **Success Rate:** Taxa de sucesso das operações

## Configuração para STM32F0

### Limitações do Hardware
- **RAM Total:** 8KB
- **Clock:** 48MHz (típico)
- **Flash:** 32-256KB (dependendo do modelo)

### Configurações Recomendadas

Para uso em STM32F0, ajuste `aho_config.h`:

```c
#define AC_MAX_VERTICES 50      // Reduzido de 80
#define AC_MAX_PATTERNS 12      // Reduzido de 40
#define AC_MAX_PATTERNS_PER_VERTEX 2
#define AC_MAX_TRANSITIONS_PER_VERTEX 26
```

### Cenário Recomendado
Baseado nos testes, o **SCENARIO_EMBEDDED** é o mais adequado:
- 10 padrões máximo
- Texto até 256 caracteres
- Uso de memória ~500-800 bytes (6-10% da RAM)
- Performance adequada para aplicações real-time

## Interpretação dos Resultados

### Tabela de Comparação
A tabela mostra métricas side-by-side para todos os cenários:
- Compare **Memory** para verificar adequação ao STM32F0
- Compare **Build(us)** e **Search(us)** para performance
- **Vertices** indica complexidade do autômato

### Análise de Adequação STM32F0
O programa avalia automaticamente cada cenário:
- ✅ **RECOMMENDED:** Adequado para STM32F0
- ⚠️ **CONSIDER:** Pode funcionar com otimizações
- ❌ **NOT SUITABLE:** Excede limitações do hardware

### Critérios de Avaliação
- **Memória:** ≤ 60% da RAM total (≤4.8KB)
- **Build Time:** ≤ 50ms a 48MHz
- **Search Time:** ≤ 10ms por busca

## Otimizações para Sistemas Embarcados

### 1. Redução de Memória
```c
// Use padrões mais curtos
const char* patterns[] = {"err", "ok", "warn", "info"};

// Limite o número de padrões
#define MAX_EMBEDDED_PATTERNS 8
```

### 2. Pool de Memória Estática
```c
// Evite malloc/free dinâmicos
static uint8_t memory_pool[2048];
static size_t pool_offset = 0;

void* embedded_malloc(size_t size) {
    if (pool_offset + size > sizeof(memory_pool)) return NULL;
    void* ptr = &memory_pool[pool_offset];
    pool_offset += size;
    return ptr;
}
```

### 3. Configuração de Compilação
```bash
# Para STM32F0
gcc -Os -mcpu=cortex-m0 -mthumb -DSTM32F0 \
    -ffunction-sections -fdata-sections \
    -DNDEBUG -Wl,--gc-sections
```

## Arquivo de Saída Exemplo

```
=== Aho-Corasick Application ===
Platform: PC

=== Build Performance ===
Patterns added: 10
Vertices created: 47
Build time: 245 microseconds

=== Search Performance ===
Text length: 256 characters
Matches found: 8
Search time: 89 microseconds
Throughput: 2876.40 chars/ms

=== Memory Analysis ===
Platform: PC
Total RAM: 1048576 bytes
Available RAM: 1048576 bytes
Automaton size: 892 bytes
Memory usage: 0.09%

=== Final Summary ===
Platform: PC
Memory efficiency: 0.09% of total RAM
Total execution time: 334 microseconds
Patterns/Memory ratio: 11.21 patterns/KB
```

## Próximos Passos

1. **Portar para STM32F0:** Adaptar código para ambiente embarcado real
2. **Validação de Hardware:** Testar em hardware STM32F0 real
3. **Otimização:** Ajustar baseado em resultados do hardware
4. **Documentação:** Criar guia específico para desenvolvimento embarcado

## Troubleshooting

### Erro de Compilação
```bash
# Verificar dependências
sudo apt-get install build-essential

# Limpar e recompilar
make clean
make perf_app
```

### Erro de Execução
```bash
# Verificar permissões
chmod +x run_tests.sh

# Executar com verbose
make run_perf 2>&1 | tee debug.log
```

### Análise de Memória
```bash
# Usar Valgrind para análise detalhada
valgrind --tool=memcheck --leak-check=full ./build/aho_perf_app
```

## Contribuições

Para contribuir com melhorias:

1. Teste novos cenários em `aho_app_config.c`
2. Adicione métricas específicas em `aho_app.c`
3. Implemente otimizações em `aho_app_stm32.c`
4. Atualize documentação

## Licença

Este projeto segue a mesma licença do projeto principal Aho-Corasick.
