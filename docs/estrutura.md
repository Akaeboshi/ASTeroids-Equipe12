# Estrutura do Projeto de Compilador

## 📁 Include/

### Include/ - Cabeçalhos (.h) com definições de estruturas e interfaces
- AST, tabela de símbolos, analisadores sintático/semântico

#### ast.h
- Função: Header principal da AST que inclui todos os outros headers relacionados
- Depende de: `ast_base.h`, `ast_expr.h`, `ast_printer.h`, `ast_free.h`, `symbol_table.h`
- Função: Facilita o include de toda a estrutura de AST em um único arquivo

#### ast_base.h
- Função: Define a estrutura fundamental da Árvore Sintática Abstrata (AST)
- Componentes:
  - Enums: `TypeTag` (tipos), `NodeKind` (tipos de nós), `BinOp` (operações binárias), `UnOp` (operações unárias)
  - Struct `Node`: União que representa todos os tipos possíveis de nós da AST
  - Funções utilitárias: `xmalloc()`, `xstrdup()`, `new_node()`, `ast_copy()`

#### ast_expr.h
- Função: Declara funções construtoras para cada tipo de nó da AST
- Funções: Construtores para literais, expressões, declarações, estruturas de controle, funções

#### ast_free.h
- Função: Interface para liberação de memória da AST
- Função: `ast_free()` - libera recursivamente toda a árvore

#### ast_printer.h
- Função: Interface para impressão da AST
- Funções: `ast_print()` (formato compacto), `ast_print_pretty()` (formato indentado)

#### symbol_table.h
- Função: Define a estrutura da tabela de símbolos com escopos
- Componentes:
  - Struct `Symbol`: Representa um símbolo (nome, tipo, valor)
  - Struct `SymbolTable`: Tabela hash com encadeamento e suporte a escopos aninhados
  - Operações: inserção, busca, atualização, remoção (com versões recursivas para escopos)

#### semantic_analyzer.h
- Função: Interface do analisador semântico
- Funções: `check_semantics()` (executa análise), `semantics_ok()` (verifica se não há erros)

#### syntax_analyzer.h
- Função: Interface do analisador sintático
- Struct: `SyntaxResult` - padroniza o resultado da análise sintática
- Função: `syntax_parse_path()` - analisa arquivo ou stdin

## 📁 src/

### src/ - Implementação (.c) de toda a lógica do compilador
- Construtores de AST, análise léxica/sintática/semântica, gerenciamento de memória

#### ast_base.c
- Função: Implementa funções básicas da AST
- Funções Principais:
  - `xmalloc()`, `xstrdup()` - alocação segura de memória
  - `new_node()` - cria novo nó
  - `ast_copy()` - cópia profunda completa da AST

#### ast_expr.c
- Função: Implementa construtores de nós da AST
- Cobertura: Todos os tipos de nós definidos em `ast_base.h`
- Destaque: `ast_block_add_stmt()` - gerencia array dinâmico de statements

#### ast_free.c
- Função: Liberação recursiva de memória da AST
- Implementação: Switch que trata cada tipo de nó especificamente

#### ast_printer.c
- Função: Implementa impressão da AST em dois formatos
- Funções:
  - `ast_print()` - formato linear (para máquina)
  - `ast_print_pretty()` - formato indentado (para humanos)
- Helpers: Conversores de enums para strings

#### parser.y
- Função: Gramática Bison do compilador
- Características:
  - Gramática LALR(1) completa da linguagem
  - Suporte a expressões, declarações, estruturas de controle, funções
  - Precedência e associatividade de operadores
  - Recuperação de erros sintáticos
  - Constrói AST durante o parsing

#### scanner.l
- Função: Analisador léxico Flex
- Tokenização: Reconhece literais, identificadores, operadores, palavras-chave
- Características:
  - Suporte a comentários de linha e multilinha
  - Processamento de strings com sequências de escape
  - Contagem de linha e coluna
  - Recuperação de erros léxicos

#### semantic_analyzer.c
- Função: Implementa análise semântica completa
- Funcionalidades:
  - Sistema de escopos aninhados
  - Verificação de tipos em expressões e atribuições
  - Validação de declarações e uso de variáveis
  - Verificação de chamadas de função (aridade e tipos)
  - Controle de retorno em funções
  - Registro e validação de assinaturas de funções

#### symbol_table.c
- Função: Implementa tabela de símbolos com hash table
- Características:
  - Tabela hash com encadeamento
  - Suporte a escopos aninhados via campo `parent`
  - Operações de inserção, busca, atualização e remoção
  - Funções recursivas para busca em escopos pai

#### syntax_analyzer.c
- Função: Driver do analisador sintático
- Função: `syntax_parse_path()` - coordena parsing de arquivo/stdin

## 📁 src/drivers/

### src/drivers/ - Programas de teste independentes
- Testes específicos do lexer, parser e pipeline completo

#### lexer_driver.c
- Função: Teste independente do analisador léxico
- Funcionalidade: Tokeniza entrada e mostra tokens reconhecidos

#### syntax_driver.c
- Função: Teste do analisador sintático
- Funcionalidade: Parsing completo com impressão da AST resultante

#### semantic_driver.c
- Função: Teste do pipeline completo (léxico + sintático + semântico)
- Funcionalidade: Executa todas as fases e reporta erros

## Arquitetura Geral

### Fluxo de Compilação:
1. Análise Léxica (scanner.l) → Tokens
2. Análise Sintática (parser.y) → AST
3. Análise Semântica (semantic_analyzer.c) → AST Validada
4. Tabela de Símbolos (symbol_table.c) → Escopos e Tipos

### Características da Linguagem:
- Tipos: int, float, bool, string, void
- Estruturas: if/else, while, for, functions, return
- Expressões: Aritméticas, relacionais, lógicas
- Escopos: Blocos aninhados com tabela de símbolos hierárquica

### Design Patterns:
- Visitor Pattern: Para análise semântica e impressão da AST
- Factory Pattern: Construtores de nós em `ast_expr.c`
- Composite Pattern: Estrutura de árvore da AST

| Versão | Data       | Autor(es)                     | Descrição                              |
|--------|------------|-------------------------------|----------------------------------------|
| 1.0    | 10/11/2025 | [akaeboshi](https://github.com/akaeboshi) | Documentação completa da arquitetura do compilador incluindo estrutura de pastas, Função dos arquivos e fluxo de compilação |
