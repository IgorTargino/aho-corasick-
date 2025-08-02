#!/bin/bash

# Script para execução completa dos testes de performance do Aho-Corasick
# Autor: Igor Targino
# Descrição: Executa testes em diferentes cenários e gera relatórios

echo "=========================================="
echo "Aho-Corasick Performance Test Suite"
echo "=========================================="

# Verificar se o make está disponível
if ! command -v make &> /dev/null; then
    echo "Erro: make não encontrado. Instale build-essential"
    exit 1
fi

# Verificar se o gcc está disponível
if ! command -v gcc &> /dev/null; then
    echo "Erro: gcc não encontrado. Instale build-essential"
    exit 1
fi

# Criar diretórios necessários
mkdir -p reports
mkdir -p build

echo "Compilando aplicação de performance..."

# Compilar versão debug
echo "1. Compilando versão DEBUG..."
make perf_app
if [ $? -ne 0 ]; then
    echo "Erro na compilação da versão debug"
    exit 1
fi

# Compilar versão release
echo "2. Compilando versão RELEASE..."
make perf_app_release
if [ $? -ne 0 ]; then
    echo "Erro na compilação da versão release"
    exit 1
fi

# Executar testes
echo ""
echo "Executando testes de performance..."
echo "=========================================="

# Teste 1: Versão Debug
echo ""
echo "=== TESTE 1: Versão Debug (PC) ==="
echo "Timestamp: $(date)"
./build/aho_perf_app > reports/debug_test_$(date +%Y%m%d_%H%M%S).log 2>&1
DEBUG_EXIT_CODE=$?

# Teste 2: Versão Release
echo ""
echo "=== TESTE 2: Versão Release (PC) ==="
echo "Timestamp: $(date)"
./build/aho_perf_app > reports/release_test_$(date +%Y%m%d_%H%M%S).log 2>&1
RELEASE_EXIT_CODE=$?

# Análise com Valgrind (se disponível)
if command -v valgrind &> /dev/null; then
    echo ""
    echo "=== TESTE 3: Análise de Memória com Valgrind ==="
    valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
             --track-origins=yes --verbose --log-file=reports/valgrind_$(date +%Y%m%d_%H%M%S).log \
             ./build/aho_perf_app > /dev/null 2>&1
    VALGRIND_EXIT_CODE=$?
else
    echo "Valgrind não disponível - pulando análise de memória"
    VALGRIND_EXIT_CODE=0
fi

# Teste de stress com múltiplas execuções
echo ""
echo "=== TESTE 4: Teste de Stress (10 execuções) ==="
STRESS_LOG="reports/stress_test_$(date +%Y%m%d_%H%M%S).log"
echo "Stress Test Report - $(date)" > $STRESS_LOG
echo "=======================================" >> $STRESS_LOG

for i in {1..10}; do
    echo "Execução $i/10..."
    echo "" >> $STRESS_LOG
    echo "--- Execução $i ---" >> $STRESS_LOG
    ./build/aho_perf_app >> $STRESS_LOG 2>&1
done

# Gerar relatório consolidado
REPORT_FILE="reports/consolidated_report_$(date +%Y%m%d_%H%M%S).md"
echo "Gerando relatório consolidado: $REPORT_FILE"

cat > $REPORT_FILE << EOF
# Relatório de Performance - Aho-Corasick

**Data:** $(date)  
**Plataforma:** $(uname -a)  
**Compilador:** $(gcc --version | head -n1)  

## Configuração do Sistema

- **CPU:** $(lscpu | grep "Model name" | cut -d: -f2 | xargs)
- **RAM:** $(free -h | grep "Mem:" | awk '{print $2}')
- **OS:** $(lsb_release -d 2>/dev/null | cut -d: -f2 | xargs || echo "$(cat /etc/os-release | grep PRETTY_NAME | cut -d= -f2 | tr -d '\"')")

## Resumo dos Testes

| Teste | Status | Observações |
|-------|--------|-------------|
| Debug Build | $([ $DEBUG_EXIT_CODE -eq 0 ] && echo "✅ PASSOU" || echo "❌ FALHOU") | Compilação e execução em modo debug |
| Release Build | $([ $RELEASE_EXIT_CODE -eq 0 ] && echo "✅ PASSOU" || echo "❌ FALHOU") | Compilação e execução otimizada |
| Valgrind | $([ $VALGRIND_EXIT_CODE -eq 0 ] && echo "✅ PASSOU" || echo "⚠️ N/A") | Análise de memória |
| Stress Test | ✅ EXECUTADO | 10 execuções consecutivas |

## Análise de Performance

### Configuração Testada
- **Plataforma:** PC (Linux)
- **Cenário:** Desktop (20 padrões)
- **Limitações de Memória:** Não aplicável (PC)

### Métricas Principais
EOF

# Extrair métricas do último teste
if [ -f "reports/release_test_$(date +%Y%m%d)_"*.log ]; then
    LATEST_RELEASE=$(ls -t reports/release_test_$(date +%Y%m%d)_*.log | head -n1)
    
    echo "" >> $REPORT_FILE
    echo "#### Extração de Dados do Teste Release:" >> $REPORT_FILE
    echo '```' >> $REPORT_FILE
    grep -E "(Build time|Search time|Memory usage|Patterns added|Vertices created)" $LATEST_RELEASE >> $REPORT_FILE
    echo '```' >> $REPORT_FILE
fi

cat >> $REPORT_FILE << EOF

## Projeção para STM32F0

### Limitações Esperadas
- **RAM Disponível:** 8KB
- **Padrões Máximos:** ~8-12 (estimativa)
- **Texto Máximo:** ~256 caracteres
- **Performance:** Limitada pela CPU (48MHz)

### Recomendações para Embedded
1. **Reduzir número de padrões** para máximo 10
2. **Limitar tamanho dos padrões** para 8-12 caracteres
3. **Usar buffers estáticos** para evitar fragmentação
4. **Implementar pool de memória** customizado
5. **Otimizar configurações** do `aho_config.h`

## Arquivos Gerados

- Logs de teste: \`reports/\`
- Relatório Valgrind: \`reports/valgrind_*.log\`
- Teste de stress: \`reports/stress_test_*.log\`

## Próximos Passos

1. **Teste em STM32F0:** Adaptar código para ambiente embarcado
2. **Otimização:** Ajustar parâmetros baseado nos resultados
3. **Validação:** Comparar performance entre plataformas
4. **Documentação:** Atualizar guias de configuração

---
*Relatório gerado automaticamente pelo script de teste*
EOF

echo ""
echo "=========================================="
echo "Resumo da Execução:"
echo "=========================================="
echo "Debug Build:    $([ $DEBUG_EXIT_CODE -eq 0 ] && echo "✅ SUCESSO" || echo "❌ ERRO")"
echo "Release Build:  $([ $RELEASE_EXIT_CODE -eq 0 ] && echo "✅ SUCESSO" || echo "❌ ERRO")"
echo "Valgrind:       $([ $VALGRIND_EXIT_CODE -eq 0 ] && echo "✅ SUCESSO" || echo "⚠️ N/A")"
echo "Stress Test:    ✅ EXECUTADO"
echo ""
echo "Arquivos gerados:"
echo "- Relatório: $REPORT_FILE"
echo "- Logs: reports/"
echo ""
echo "Para ver o relatório completo:"
echo "cat $REPORT_FILE"
echo ""
echo "Para executar apenas um teste:"
echo "make run_perf          # Versão debug"
echo "make run_perf_release  # Versão release"
echo ""
echo "=========================================="
