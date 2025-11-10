#!/usr/bin/env bash
set -euo pipefail

# 1) gere o header de tokens do bison (necessário para compilar scanner)
bison -d parser.y

# 2) gere o scanner com flex
flex scanner.l

# 3) compile o driver (não precisamos linkar parser.tab.c aqui)
gcc -o lexer_driver lex.yy.c lexer_driver.c -lfl

# 4) rode o teste
mkdir -p tests
if [ ! -f tests/lexer_tests.txt ]; then
  echo "Arquivo tests/lexer_tests.txt não encontrado."
  exit 1
fi

./lexer_driver < tests/lexer_tests.txt
