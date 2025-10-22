# -----------------------------
# Diretórios
# -----------------------------
SRC_DIR     := src
INCLUDE_DIR := include
TEST_DIR    := tests

# -----------------------------
# Executável final
# -----------------------------
EXEC := $(SRC_DIR)/parser

# -----------------------------
# Fontes (AST + main)
# -----------------------------
AST_SRCS   := $(SRC_DIR)/ast_base.c $(SRC_DIR)/ast_expr.c $(SRC_DIR)/ast_printer.c $(SRC_DIR)/ast_free.c
MAIN_SRC   := $(SRC_DIR)/main.c

# -----------------------------
# Gerados (Bison/Flex)
# -----------------------------

# Arquivos-fonte do Bison e do Flex
BISON_FILE := $(SRC_DIR)/parser.y
FLEX_FILE  := $(SRC_DIR)/scanner.l

# Arquivos que o Bison vai gerar
BISON_C := $(SRC_DIR)/parser.tab.c
BISON_H := $(SRC_DIR)/parser.tab.h

# Arquivo gerado pelo Flex
FLEX_C     := $(SRC_DIR)/lex.yy.c

# -----------------------------
# Ferramentas e flags
# -----------------------------
CC      = gcc
CFLAGS := -I$(INCLUDE_DIR) -I$(SRC_DIR)

BISON_FLAGS := -d      # <- GERA o parser.tab.h
FLEX_FLAGS  :=

# Linkagem da lib do Flex
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LDFLAGS :=
else
  LDFLAGS := -lfl
endif

# -----------------------------
# Alvos principais
# -----------------------------
.PHONY: all build run test clean

all: $(EXEC)

# Regra para gerar o executável: depende dos arquivos gerados por Bison e Flex
$(EXEC): $(BISON_C) $(FLEX_C) $(AST_SRCS) $(MAIN_SRC)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(AST_SRCS) $(MAIN_SRC) $(LDFLAGS)

# Regra para rodar o Bison: gera parser.tab.c e parser.tab.h
$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison $(BISON_FLAGS) -o $(BISON_C) $(BISON_FILE)

# Regra para rodar o Flex: gera lex.yy.c
$(FLEX_C): $(FLEX_FILE)
	flex $(FLEX_FLAGS) -o $(FLEX_C) $(FLEX_FILE)

# -----------------------------
# Execução rápida
# Use: make run            (lê de stdin)
#      make run FILE=path  (lê de arquivo)
# -----------------------------
run: $(EXEC)
	@if [ -n "$(FILE)" ]; then \
	  echo ">> Rodando $(EXEC) com arquivo: $(FILE)"; \
	  $(EXEC) "$(FILE)"; \
	else \
	  echo ">> Rodando $(EXEC) (stdin). Digite código e Ctrl+D para terminar."; \
	  $(EXEC); \
	fi

# -----------------------------
# Testes (usa tests/run.sh)
# -----------------------------
test: build
	@bash $(TEST_DIR)/run.sh

# -----------------------------
# Limpeza
# -----------------------------
clean:
	@echo "Limpando artefatos…"
	@rm -f $(EXEC) $(BISON_C) $(BISON_H) $(FLEX_C)
