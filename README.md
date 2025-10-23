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

Executa a suíte de testes automatizados:

```bash
make test
```

Ao final, o script exibirá um resumo dos testes que passaram e falharam.

### 🧹 Limpar artefatos gerados

Remove arquivos gerados por Bison/Flex e binários:

```bash
make clean
```

---

## 🗂️ Estrutura do Projeto

```text
asreroids-equipe12/
├── src/
│   ├── parser.y          # Gramática principal (Bison)
│   ├── scanner.l         # Analisador léxico (Flex)
│   ├── ast_base.c/.h     # Estrutura base da AST
│   ├── ast_expr.c/.h     # Expressões e operadores
│   ├── ast_printer.c/.h  # Impressão da AST
│   ├── ast_free.c/.h     # Liberação de memória da AST
│   ├── main.c            # Programa principal (executa parser)
│
├── include/              # Headers e definições globais
│
├── tests/
│   ├── syntax/           # Casos de teste sintático
│   ├── semantic/         # Casos de teste semântico
│   ├── intermediate/     # Casos de código intermediário
│   ├── generation/       # Casos de geração de código
│   └── run.sh            # Script automatizado de testes
│
├── docs/                 # Documentação adicional (diagramas, relatórios)
└── Makefile              # Automação de build, run e test
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
