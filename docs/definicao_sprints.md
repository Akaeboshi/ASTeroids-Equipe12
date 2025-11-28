## Planejamento de Sprints 

Adotaremos um fluxo inspirado em Scrum: em cada Sprint Planning definimos metas e tarefas para a próxima sprint; fazemos acompanhamentos rápidos para destravar impedimentos e encerramos cada sprint com a sprint review para mostrar o que foi concluído e o que é esperado para a próxima sprint.

## Sprint 1

Inicio: 27/08
Fim: 09/09

**objetivos**


- Definir se seria feito um compilador ou intepretador (todos)
- Definir as linguagens para o compilador (todos)
- Definir tamanho e quantas sprints o trabalho terá (todos)
- Documentação do escopo (Maciel)

## Sprint 2

inicio: 10/09
Fim: 01/10

**Objetivos**

- Adição do readme (heyttor)
- implementação da ast minima (maciel)
- implementação do parser (heyttor e ranni)
- implementação do scarnner (antonio e joão)

## Sprint 3

inicio: 02/10
fim: 15/10

**Objetivos**

- Dar os primeiros passos na análise semântica (tipo de variáveis, escopos). (maciel)
- Evoluir o analisador sintático com novas produções gramaticais. (Heyttor)
- Analisador semântico inicial identificando erros básicos (variáveis não declaradas, tipos simples). (Antonio)
- Estender as regras gramaticais no Bison, cobrindo as principais construções da linguagem. (Ranni)
- Implementar verificação de tipos e de escopo simples (reportar erros se algo estiver fora das regras) (João)

  ## Sprint 4

  inicio: 16/10
  fim: 12/11

  **Objetivos**

  - Testes do scanner (tokens e literais) (Antonio)
  - Finalizar expressões regulares no Flex (tokens principais) (Antonio)
  - Criar infraestrutura de testes e documentação - Golden tests e make test (Antonio)
  - Precedência e associatividade (expressões) (Maciel)
  - Resolver conflito shift/reduce (Maciel)
  - Literais e identificadores na AST - AST — Construção da Árvore de Sintaxe Abstrata (Maciel)
  - Main + testes básicos de AST (Maciel)
  - Blocos {} e lista de statements (Heyttor)
  - Statements de expressão (Heyttor)
  - Integrar controle de fluxo e blocos ao parser (Heyttor)
  - Criar nós de controle de fluxo na AST (If, While, For, Return, Block) (Heyttor)
  - If / If-Else (dangling else) (Heyttor)
  - While e For (Heyttor)
  - nó chamada de função AST (Heyttor e maciel)
  - Tabela de símbolos: escopos (Ranni)
  - Criar tabela de símbolos e gestão de escopos ( Ranni)
  - Adicionar declaração de variáveis (var) (Ranni)
  - Atribuição IDENT = Expr (Ranni)
  - Implementar atribuição IDENT = Expr - Strings (escapes e memória) (Ranni)
  - Consolidar literais string e ND_STRING (Ranni)
  - Tipagem básica  (João)
  - Implementar inferência e checagem de tipos (João)
  - Erros semânticos: uso sem declaração (João)
  - Detectar identificadores não declarados (João)
  - Documentação das sprints e metodologia (Heyttor)
  - Documentação da estrutura 



  ## Sprint 6

  Inicio da sprint 13/11
  Fim da sprint 28/11

  **Objetivos**

  - Construção da logica de golden teste make test (maciel)
  - Suporte a declarações de tipos primitivos (maciel)
  - Implementação da sfunções de retorno (maciel)
  - Suporte a funções na geração IR (Heyttor)
  - Criar testes IR (Ranni)
  - Documentação da IR (Antonio)
  - Implementação das comparações em IR e na geração do codigo final (Heyttor)
  - Implementação das literias e mov em IR e na geração do codigo final (Maciel)
  - Implementação da Aritmetica(+,-,* e / ) em IR e na geração do codigo final (Maciel)
  - Implementação do controle de fluxo (labels e branches ) (Ranni)
  - Implementação das Strings em IR e na geração do codigo final (Maciel,Antonio e João)
  - Implementação dos testes de comparação na geração do codigo final (Heyttor e Marciel)
  