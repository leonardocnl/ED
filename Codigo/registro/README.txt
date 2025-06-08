README - Sistema de Gerenciamento de Dados com Intercalação Polifásica
A PASTA ./registro DEVE EXISTIR PARA O FUNCIONAMENTO DO PROGRAMA
================================================================================

Este documento explica detalhadamente como funciona o sistema de gerenciamento de dados
implementado no arquivo polyphaseMerge.cpp. O sistema é capaz de ler dados de arquivos CSV,
convertê-los para formato binário, realizar operações de manipulação e executar
ordenação usando o algoritmo de intercalação polifásica.

================================================================================
ESTRUTURAS DE DADOS PRINCIPAIS
================================================================================

1. ESTRUTURA 'dado' - A Base do Sistema
----------------------------------------
A estrutura 'dado' representa um único registro de dados econômicos com os seguintes campos:

- dataValue (double): Valor numérico principal do registro
- seriesTitle1-5 (char[]): Títulos da série de dados (diferentes tamanhos)
- group (char[100]): Grupo ao qual o dado pertence
- subject (char[50]): Assunto do registro
- seriesReference (char[20]): Referência da série
- period (char[10]): Período temporal do dado
- status (char[10]): Status do dado
- units (char[10]): Unidade de medida
- magnitude (char): Magnitude do valor
- isEndOfRun (uint8_t): Marcador especial para o algoritmo de ordenação

Funções da estrutura 'dado':
- Construtor: Inicializa todos os campos com valores padrão
- operator>: Compara dois registros para ordenação (considera marcadores especiais)
- operator<: Versão inversa do operador >
- operator==: Verifica se dois registros são iguais
- operator<=: Combinação de < e ==
- endMarker(): Verifica se o registro é um marcador de fim
- setAsDataRecord(): Define o registro como dado normal
- setAsEndOfRun(): Define o registro como marcador de fim

2. ESTRUTURA 'PolyphaseTape' - Gerenciamento de Fitas
-----------------------------------------------------
Simula as "fitas" usadas no algoritmo de intercalação polifásica:

- filename (string): Nome do arquivo que representa a fita
- stream (fstream): Fluxo de dados para leitura/escrita
- actualRuns (long long): Número de sequências reais de dados
- dummyRuns (long long): Número de sequências fictícias (para balanceamento)

Funções:
- Construtor: Inicializa a fita com um nome de arquivo
- Destrutor: Fecha o arquivo automaticamente
- rewind_and_clear_flags(): Volta ao início do arquivo e limpa erros
- readRecord(): Lê um registro da fita
- writeRecord(): Escreve um registro na fita
- getFileSize(): Retorna o tamanho do arquivo em bytes

3. CLASSE 'mergeFileIn' - Leitura Durante Intercalação
------------------------------------------------------
Facilita a leitura de dados durante o processo de intercalação:

- tape: Ponteiro para a fita sendo lida
- currentReg: Registro atual sendo processado
- hasData: Indica se há dados válidos no registro atual

Funções:
- loadNextDataRecord(): Carrega o próximo registro válido
- prepareForNewLogicalRun(): Prepara para uma nova sequência de dados
- advance(): Avança para o próximo registro
- getCurrentRecord(): Retorna o registro atual
- hasRecords(): Verifica se há registros disponíveis
- isEndOfCurrentRun(): Verifica se a sequência atual terminou

================================================================================
FUNÇÕES DE ENTRADA/SAÍDA DE DADOS
================================================================================

1. parseCSVLine() - Análise de Linhas CSV
-----------------------------------------
Função que separa uma linha CSV em campos individuais, tratando corretamente:
- Campos entre aspas que podem conter vírgulas
- Campos vazios
- Escapes especiais

Parâmetros:
- line: String contendo a linha CSV
- fields[]: Array onde serão armazenados os campos separados
- max_fields: Número máximo de campos esperados

Retorna: Número de campos encontrados

2. csvReadWrite() - Conversão CSV para Binário
----------------------------------------------
Função principal para converter arquivos CSV em formato binário:

Como funciona:
1. Abre o arquivo CSV para leitura
2. Cria arquivo binário para escrita
3. Pula a primeira linha (cabeçalho)
4. Para cada linha:
   - Usa parseCSVLine() para separar os campos
   - Cria um registro 'dado'
   - Preenche os campos do registro
   - Escreve o registro no arquivo binário
5. Exibe estatísticas da conversão

Tratamento de erros:
- Linhas com número incorreto de campos
- Valores não numéricos em campos numéricos
- Campos muito longos (truncados automaticamente)

3. Funções de Impressão de Dados
--------------------------------

printDado(): Exibe um registro completo na tela
- Mostra todos os campos de forma organizada
- Converte arrays de char para strings legíveis

datPrint() (versão completa): Imprime todos os registros do arquivo
- Pergunta se o usuário quer realmente imprimir
- Mostra o número total de registros
- Imprime registro por registro

datPrint() (versão por intervalo): Imprime registros em uma faixa específica
- Recebe posições inicial e final
- Move o ponteiro do arquivo para a posição correta
- Imprime apenas os registros solicitados

================================================================================
FUNÇÕES DE MANIPULAÇÃO DE DADOS
================================================================================

1. datSwap() - Troca de Posições
-------------------------------
Troca dois registros de posição no arquivo:

Como funciona:
1. Abre o arquivo em modo leitura/escrita
2. Calcula as posições dos registros X e Y
3. Lê o registro da posição X
4. Lê o registro da posição Y
5. Escreve o registro Y na posição X
6. Escreve o registro X na posição Y

Validações:
- Verifica se as posições estão dentro do arquivo
- Confirma se o arquivo pode ser aberto

2. datInsert() - Inserção de Registro
------------------------------------
Insere um novo registro em uma posição específica:

Como funciona:
1. Cria arquivo temporário
2. Copia todos os registros a partir da posição de inserção para o temporário
3. Solicita ao usuário os dados do novo registro
4. Escreve o novo registro na posição desejada
5. Copia de volta os dados do arquivo temporário
6. Remove o arquivo temporário

3. datChange() - Modificação de Registro
---------------------------------------
Modifica um registro existente:

Como funciona:
1. Localiza o registro na posição especificada
2. Mostra os dados atuais
3. Solicita novos dados ao usuário
4. Sobrescreve o registro com os novos dados

================================================================================
ALGORITMO DE INTERCALAÇÃO POLIFÁSICA - EXPLICAÇÃO DETALHADA
================================================================================

A intercalação polifásica é um algoritmo sofisticado de ordenação externa que foi
desenvolvido especificamente para lidar com arquivos grandes demais para caber
na memória. É uma evolução do merge sort tradicional, otimizado para usar apenas
3 arquivos auxiliares (fitas) em vez de dividir pela metade recursivamente.

CONCEITOS FUNDAMENTAIS
======================

O que são "Fitas"?
------------------
No contexto histórico, "fitas" eram literalmente fitas magnéticas usadas em 
mainframes. Hoje, são arquivos temporários que simulam esse comportamento:
- T0.dat: Primeira fita auxiliar
- T1.dat: Segunda fita auxiliar  
- T2.dat: Terceira fita auxiliar

Por que usar 3 fitas?
--------------------
Com 3 fitas, podemos sempre intercalar 2 fitas em uma terceira, alternando
papéis a cada fase. Isso permite:
- Otimização de I/O (menos operações de disco)
- Uso mínimo de espaço auxiliar
- Padrão de acesso sequencial (mais eficiente)

SEQUÊNCIAS (RUNS) E MARCADORES
==============================

O que é uma Sequência (Run)?
----------------------------
Uma sequência é um trecho de dados já ordenados. Por exemplo:
- Sequência 1: [1, 3, 7, 9]
- Sequência 2: [2, 4, 5, 8]
- Sequência 3: [6, 10, 11, 12]

Marcadores de Fim de Sequência (EOR - End of Run)
-------------------------------------------------
Para saber onde uma sequência termina e outra começa, usamos marcadores especiais:
- isEndOfRun = 1: Indica fim de sequência
- isEndOfRun = 0: Indica registro normal de dados

Exemplo no arquivo:
[1] [3] [7] [9] [EOR] [2] [4] [5] [8] [EOR] [6] [10] [11] [12] [EOR]

SEQUÊNCIAS DUMMY (FICTÍCIAS) - CONCEITO CRUCIAL
===============================================

Por que Sequências Dummy são Necessárias?
------------------------------------------
O algoritmo polifásico funciona melhor quando o número total de sequências
segue um padrão específico baseado na sequência de Fibonacci. Se não temos
sequências suficientes, criamos sequências "fictícias" (dummy) para balancear.

O que são Sequências Dummy?
---------------------------
Sequências dummy são conceituais - não existem fisicamente no arquivo!
São apenas contadores que ajudam a sincronizar o algoritmo:

Exemplo prático:
- Temos 5 sequências reais
- Fibonacci ideal seria 8 (próximo número >= 5)  
- Precisamos de 3 sequências dummy (8 - 5 = 3)

Como Funcionam na Prática:
-------------------------
1. Durante distribuição: Contamos como se fossem sequências reais
2. Durante intercalação: São "processadas" instantaneamente (não fazem nada)
3. Garantem que o algoritmo termine corretamente

Estado das fitas com dummy:
- T0: 5 sequências reais + 0 dummy = 5 total
- T1: 0 sequências reais + 3 dummy = 3 total  
- T2: 0 sequências (fita de saída)

DISTRIBUIÇÃO FIBONACCI - O CORAÇÃO DO ALGORITMO
===============================================

Por que Fibonacci?
------------------
A sequência de Fibonacci (1, 1, 2, 3, 5, 8, 13, 21, ...) tem uma propriedade
matemática especial: F(n) = F(n-1) + F(n-2)

Esta propriedade garante que:
- O algoritmo sempre termine
- O número de fases seja mínimo
- As fitas sejam balanceadas adequadamente

Como Calcular a Distribuição?
------------------------------
Exemplo com 7 sequências reais:

1. Encontrar próximo Fibonacci >= 7:
   F(0)=0, F(1)=1, F(2)=1, F(3)=2, F(4)=3, F(5)=5, F(6)=8, F(7)=13
   Próximo >= 7 é F(6) = 8

2. Calcular distribuição nas 3 fitas:
   Para F(6) = 8, usamos F(5), F(4), F(3):
   - T0: F(5) = 5 sequências
   - T1: F(4) = 3 sequências  
   - T2: F(3) = 0 sequências (fita de saída)

3. Calcular sequências dummy necessárias:
   Total necessário: 5 + 3 + 0 = 8
   Sequências reais: 7
   Dummy necessárias: 8 - 7 = 1

4. Distribuição final:
   - T0: 5 reais + 0 dummy
   - T1: 2 reais + 1 dummy
   - T2: 0 (vazia)

ALGORITMO PASSO A PASSO
=======================

FASE 1: Criação de Sequências Iniciais
--------------------------------------
Objetivo: Criar sequências ordenadas que cabem na memória

1. Lê bloco de dados (ex: 1000 registros)
2. Ordena o bloco na memória (std::sort)
3. Escreve bloco ordenado + marcador EOR
4. Repete até processar todo o arquivo

Resultado: Arquivo temporário com várias sequências ordenadas
temp_initial_runs.dat: [seq1][EOR][seq2][EOR][seq3][EOR]...

FASE 2: Distribuição Fibonacci
------------------------------
Objetivo: Distribuir sequências nas fitas seguindo padrão Fibonacci

1. Calcula distribuição Fibonacci
2. Distribui sequências reais nas fitas T0 e T1
3. Ajusta contadores incluindo dummy runs
4. T2 permanece vazia (será fita de saída)

Estado após distribuição:
- T0: Contém algumas sequências + contador de dummy
- T1: Contém algumas sequências + contador de dummy
- T2: Vazia (pronta para receber resultado)

FASE 3: Intercalação Iterativa
------------------------------
Objetivo: Intercalar sequências até restar apenas uma

Exemplo detalhado - Iteração 1:
1. Inicializa leitores para T0 e T1
2. Lê primeiro registro de cada fita
3. Compara valores e escreve menor em T2
4. Avança na fita que forneceu o menor valor
5. Continua até encontrar EOR em uma das fitas
6. Copia resto da outra fita
7. Escreve EOR em T2
8. Repete para próxima sequência

Alternância de papéis:
- Iteração 1: T0 + T1 → T2
- Iteração 2: T1 + T2 → T0  
- Iteração 3: T2 + T0 → T1
- Continue até uma fita ter todas as sequências

FASE 4: Resultado Final
----------------------
1. Identifica fita com resultado completo
2. Copia conteúdo para arquivo de saída
3. Remove marcadores EOR
4. Limpa arquivos temporários

EXEMPLO PRÁTICO COMPLETO
========================

Dados iniciais: [9,1,5,3,8,2,7,4,6] (não cabem na memória, buffer=3)

FASE 1 - Sequências iniciais:
Buffer 1: [9,1,5] → ordenado: [1,5,9] + EOR
Buffer 2: [3,8,2] → ordenado: [2,3,8] + EOR  
Buffer 3: [7,4,6] → ordenado: [4,6,7] + EOR

Arquivo temporário: [1,5,9][EOR][2,3,8][EOR][4,6,7][EOR]
Total: 3 sequências

FASE 2 - Distribuição Fibonacci:
Fibonacci >= 3: F(4) = 3
Distribuição: T0=2, T1=1, T2=0
Sem dummy runs necessárias

T0: [1,5,9][EOR][2,3,8][EOR]
T1: [4,6,7][EOR]
T2: (vazia)

FASE 3 - Intercalação:

Iteração 1 (T0 + T1 → T2):
- Sequência 1 vs Sequência 3: [1,5,9] vs [4,6,7]
- Resultado: [1,4,5,6,7,9][EOR]
- Sequência 2: [2,3,8] (sem par, copia direto)
- T2: [1,4,5,6,7,9][EOR][2,3,8][EOR]

Iteração 2 (T1 + T2 → T0):
- T1 está vazia, T2 tem 2 sequências
- Intercala as 2 sequências de T2
- T0: [1,2,3,4,5,6,7,8,9][EOR]

Fim: T0 tem uma única sequência = arquivo ordenado!

VANTAGENS DO ALGORITMO
=====================

1. Eficiência de Memória: Usa apenas o buffer necessário
2. Acesso Sequencial: Otimizado para discos rígidos
3. Mínimo de Fitas: Apenas 3 arquivos auxiliares
4. Escalabilidade: Funciona para qualquer tamanho de arquivo
5. Previsibilidade: Número de fases é determinístico

IMPLEMENTAÇÃO NO CÓDIGO
=======================

1. calculateInitialDummyRuns() - Cálculo Fibonacci
-------------------------------------------------
Encontra a distribuição Fibonacci ideal e calcula quantas sequências dummy
são necessárias para balancear o algoritmo.

Entrada: Número total de sequências reais
Saída: Array com distribuição [T0, T1, T2] e total de dummies

2. polyphaseMerge() - Orquestração Principal  
-------------------------------------------
Coordena todas as fases do algoritmo usando funções auxiliares especializadas:

- inicializarTapesAuxiliares(): Prepara estruturas de dados
- validarBufferMemoria(): Confirma capacidade de memória
- validarArquivoEntrada(): Valida arquivo e conta registros
- criarSequenciasIniciais(): Implementa Fase 1
- distribuirSequenciasFibonacci(): Implementa Fase 2
- executarIntercalacaoCompleta(): Implementa Fase 3
- finalizarArquivoOrdenado(): Implementa Fase 4
- liberarRecursos(): Limpeza final

3. Estruturas de Suporte
------------------------
- PolyphaseTape: Abstrai uma "fita" com contadores de runs
- MergeFileIn: Facilita leitura durante intercalação
- Operadores de comparação: Implementam lógica de ordenação considerando EOR

3. blockRead() e blockWrite() - Operações de Bloco
-------------------------------------------------
Funções otimizadas para leitura e escrita de múltiplos registros:

blockRead(): Lê vários registros de uma vez
- Mais eficiente que ler registro por registro
- Retorna quantos registros foram realmente lidos
- Trata situações de fim de arquivo

blockWrite(): Escreve vários registros de uma vez
- Escreve todos os registros do buffer para o arquivo

================================================================================
FUNÇÕES DE INTERFACE DO USUÁRIO
================================================================================

1. Funções de Arte ASCII
------------------------
printFile(), printView(), printMod(), printBusca(), printExit():
- Exibem arte ASCII decorativa para diferentes seções do programa
- Tornam a interface mais amigável e visualmente atrativa

2. Funções de Gerenciamento de Arquivos
--------------------------------------

getFileByName(): Solicita nome do arquivo ao usuário
- Permite escolher entre converter novo CSV ou usar arquivo existente
- Valida se o arquivo existe e tem extensão correta

folderHandler(): Verifica estrutura de pastas
- Confirma se a pasta "./registro" existe
- Chama getFileByName() se tudo estiver correto

3. Funções de Utilidade
----------------------

cleanTemp(): Remove arquivos temporários
- Limpa arquivos auxiliares criados durante a ordenação
- Trata erros caso os arquivos não existam

checkSort(): Verifica se arquivo está ordenado
- Percorre todo o arquivo verificando se está em ordem crescente
- Útil para validar se a ordenação funcionou corretamente

printDataValue(): Extrai apenas valores numéricos
- Cria arquivo texto com apenas os valores dataValue
- Útil para análise ou debug

saveChanges(): Pergunta se usuário quer salvar
- Interface simples para confirmação
- Retorna true/false conforme escolha do usuário

================================================================================
FUNÇÃO PRINCIPAL E FLUXO DO PROGRAMA
================================================================================

telaMain() - Menu Principal
--------------------------
Apresenta menu com opções:
1. Adicionar registro em posição específica
2. Visualizar registros por intervalo
3. Modificar registro existente
4. Trocar posições de registros
5. Imprimir arquivo completo
6. Executar intercalação polifásica
7. Sair do programa

main() - Função Principal
------------------------
Fluxo do programa:
1. Verifica estrutura de pastas
2. Solicita arquivo CSV ao usuário
3. Converte CSV para binário
4. Executa intercalação polifásica automaticamente
5. Entra em loop do menu principal
6. Permite operações de manipulação
7. Encerra quando usuário escolhe sair

================================================================================
CONSTANTES E CONFIGURAÇÕES
================================================================================

DATA_SIZE: Tamanho de um registro em bytes
AVAILABLE_MEMORY_SIZE: Memória disponível para buffers (50MB)
NUM_ARQUIVOS_AUXILIARES: Número de fitas auxiliares (3)

================================================================================
TRATAMENTO DE ERROS
================================================================================

O sistema implementa vários níveis de tratamento de erros:

1. Validação de Entrada:
   - Verifica existência de arquivos
   - Valida extensões de arquivo
   - Confirma estrutura de pastas

2. Erros de E/S:
   - Trata falhas na abertura de arquivos
   - Verifica sucessos de leitura/escrita
   - Limpa flags de erro em streams

3. Erros de Dados:
   - Trata linhas CSV malformadas
   - Converte dados inválidos para valores padrão
   - Valida ranges de posições em arrays

4. Erros de Memória:
   - Verifica se alocações foram bem-sucedidas
   - Libera memória automaticamente com RAII
   - Usa ponteiros inteligentes quando possível

================================================================================
ARQUIVOS GERADOS PELO SISTEMA
================================================================================

- registro.dat: Arquivo binário principal com dados convertidos
- dados_ordenados.dat: Arquivo final com dados ordenados
- t0.dat, t1.dat, t2.dat: Fitas auxiliares temporárias
- temp_initial_runs.dat: Arquivo temporário para sequências iniciais
- dados_entrada.txt: Arquivo texto com valores para debug

================================================================================
CONCLUSÃO
================================================================================

Este sistema implementa um gerenciador completo de dados com capacidades de:
- Conversão de formatos (CSV → Binário)
- Manipulação de registros (inserir, modificar, trocar)
- Ordenação eficiente de grandes volumes de dados
- Interface amigável para o usuário
- Tratamento robusto de erros

O algoritmo de intercalação polifásica é especialmente adequado para ordenar
arquivos maiores que a memória disponível, usando o mínimo de recursos do sistema
enquanto mantém eficiência comparável a algoritmos mais complexos. 