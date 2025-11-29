# 🚀 ASTeroids-Equipe12 Compilador

## 📝 Introdução

Este repositório contém os materiais e estudos desenvolvidos na disciplina de **Compiladores 1** do curso de **Engenharia de Software da Universidade de Brasília (FCTE/UnB)**.
O objetivo é aprofundar conceitos, métodos e técnicas de compiladores.

---

## 📚 Documentação

<https://akaeboshi.github.io/ASTeroids-Equipe12/>

---

## 🎯 Sobre Compiladores

Um **compilador** é um programa que transcreve um codigo em uma linguagem, no caso do grupo, **C++**, para outra linguagem, no caso do grupo, **javascript**. É como um tradutor de codigo de uma linguagem para outra.
Seu objetivo é permitir que programas escritos em uma linguagem de alto nível sejam entendidos e executados por outra plataforma ou máquina, garantindo eficiência e correção na execução.

---

## ⚙️ Execução do Projeto

### 🔧 Compilar

Compila todo o pipeline (Flex + Bison + AST):

```bash
make
```

### ▶️ Executar o parser

Rodar interativamente (entrada via teclado):

```bash
make run
```

Rodar com arquivo de entrada:

```bash
make run FILE=exemplos/if.txt
```

### 🧪 Rodar todos os testes

Executa a suíte de testes automatizados de geração de codigo:
```bash
make test-codegen
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

Executa a suíte de testes automatizados de lexico:
```bash
make test-lexer
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

Executa a suíte de testes automatizados de sintaticos:
```bash
make test-syntax
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

Executa a suíte de testes automatizados de semantica:
```bash
make test-semantic
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

Executa a suíte de testes automatizados de Codigo intermediario:
```bash
make test-ir
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

### 📝 Gerar código JavaScript a partir de um arquivo

Gera automaticamente o código JavaScript correspondente ao arquivo de entrada, salvando o resultado em `build/js/.js`

```bash
make jsfile FILE=tests/generation/ok_meu_teste.in
```


### 🧹 Limpar artefatos gerados

Remove arquivos gerados por Bison/Flex e binários:

```bash
make clean
```

---

## 🗂️ Estrutura do Projeto

```text
ASTeroids-Equipe12/
├── docs/                           # Documentação do projeto (relatórios, etc.)
│
├── include/                        # Headers públicos (interface dos módulos)
│   ├── ast_base.h
│   ├── ast_expr.h
│   ├── ast_free.h
│   ├── ast_printer.h
│   ├── ast.h
│   ├── codegen_js.h
│   ├── ir_builder.h
│   ├── ir_printer.h
│   ├── ir.h
│   ├── semantic_analyzer.h
│   ├── symbol_table.h
│   └── syntax_analyzer.h
│
├── src/
│   ├── drivers/                    # Prgramas “main” para cada fase
│   │   ├── codegen_js_driver.c     # Lê IR e gera JS
│   │   ├── ir_driver.c             # Lê código-fonte e imprime IR
│   │   ├── lexer_driver.c          # Teste isolado do léxico
│   │   ├── semantic_driver.c       # Parser + semântica
│   │   └── syntax_driver.c         # Parser (sintaxe) isolado
│   │
│   |                               # Implementação da AST
│   ├── ast_base.c                  # Criação de nós, enums, helpers
│   ├── ast_expr.c                  # Expressões, operadores, etc.
│   ├── ast_free.c                  # Liberação de memória da AST
│   ├── ast_printer.c               # Impressão/depuração da AST
│   │
│   ├── codegen_js.c                # Gerador de código JS a partir do IR
│   ├── ir_builder.c                # AST -> IR (construção do código intermediário)
│   ├── ir_printer.c                # Impressão de IR em formato textual
│   ├── ir.c                        # Infraestrutura do IR (tipos, criação, etc.)
│
│   ├── irgen                       # binário (build) para gerar IR a partir do fonte
│   ├── jsgen                       # binário (build) para gerar JS a partir do IR
│
│   ├── lex.yy.c                    # Código gerado pelo Flex
│   ├── parser.y                    # Gramática (Bison)
│   ├── parser.tab.c                # Parser gerado
│   ├── parser.tab.h                # Headers do parser gerado
│
│   ├── scanner.l                   # Especificação léxica (Flex)
│   ├── scanner/                    # (se existir: outros arquivos do léxico)
│
│   ├── semantic_analyzer.c         # Análise semântica (tipos, escopos, etc.)
│   ├── symbol_table.c              # Implementação da tabela de símbolos
│   └── syntax_analyzer.c           # análise sintática
│
├── tests/
│   ├── generation/                 # Casos de geração de código
│   ├── intermediate/               # Casos de código intermediário
│   ├── syntax/                     # Casos de teste sintático
│   ├── lexer/                      # Casos de teste léxico
│   ├── semantic/                   # Casos de teste semântico
│   ├── syntax/                     # Casos de teste sintático
│   └── run.sh                      # Script automatizado de testes
│
└── Makefile                        # Regras de build, run, test
```
---

## 👥 Equipe

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/Akaeboshi">
        <img src="https://github.com/Akaeboshi.png" width="100px" alt="Lucas Heler"/>
        <br />
        <sub><b>Lucas Heler </b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/macieljuniormax">
        <img src="https://github.com/macieljuniormax.png" width="100px" alt="Maciel Ferreira"/>
        <br />
        <sub><b>Maciel Ferreira </b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/antonioleaojr">
        <img src="https://github.com/antonioleaojr.png" width="100px" alt="Antonio Jose"/>
        <br />
        <sub><b>Antonio Jose </b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/jopesmp">
        <img src="https://github.com/jopesmp.png" width="100px" alt="João Pedro"/>
        <br />
        <sub><b>João Pedro</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/H3ytt0r62">
        <img src="https://github.com/H3ytt0r62.png" width="100px" alt="Heyttor Augusto"/>
        <br />
        <sub><b>Heyttor Augusto</b></sub>
      </a>
    </td>
  </tr>
</table>
