# -----------------------------
# Diretórios
# -----------------------------
SRC_DIR     := src
INCLUDE_DIR := include
TEST_DIR    := tests

# -----------------------------
# Executáveis
# -----------------------------
EXEC_SYNTAX := $(SRC_DIR)/parser
EXEC_SEMA   := $(SRC_DIR)/semacheck

# -----------------------------
# Fontes comuns (AST + tabela)
# -----------------------------
AST_SRCS := \
  $(SRC_DIR)/ast_base.c \
  $(SRC_DIR)/ast_expr.c \
  $(SRC_DIR)/ast_printer.c \
  $(SRC_DIR)/ast_free.c

COMMON_SRCS := $(SRC_DIR)/symbol_table.c

# -----------------------------
# Mains
# -----------------------------
MAIN_SYNTAX := $(SRC_DIR)/main_syntax.c
MAIN_SEMA   := $(SRC_DIR)/main_semantic.c
SEMA_ANALYZER := $(SRC_DIR)/semantic_analyzer.c

# -----------------------------
# Bison/Flex
# -----------------------------
BISON_FILE := $(SRC_DIR)/parser.y
FLEX_FILE  := $(SRC_DIR)/scanner.l

BISON_C := $(SRC_DIR)/parser.tab.c
BISON_H := $(SRC_DIR)/parser.tab.h
FLEX_C  := $(SRC_DIR)/lex.yy.c

# -----------------------------
# Ferramentas e flags
# -----------------------------
CC      = gcc
CFLAGS := -I$(INCLUDE_DIR) -I$(SRC_DIR) -Wall -Wextra -Wno-unused-parameter
BISON_FLAGS := -d
FLEX_FLAGS  :=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LDFLAGS :=
else
  LDFLAGS := -lfl
endif

# -----------------------------
# Alvos principais
# -----------------------------
.PHONY: all build run test test-syntax test-semantic clean

# compila tudo: parser + semacheck
all: $(EXEC_SYNTAX) $(EXEC_SEMA)
build: all

# -----------------------------
# Regras de compilação
# -----------------------------
$(EXEC_SYNTAX): $(BISON_C) $(FLEX_C) $(AST_SRCS) $(COMMON_SRCS) $(MAIN_SYNTAX)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(AST_SRCS) $(COMMON_SRCS) $(MAIN_SYNTAX) $(LDFLAGS)

$(EXEC_SEMA): $(BISON_C) $(FLEX_C) $(AST_SRCS) $(COMMON_SRCS) $(SEMA_ANALYZER) $(MAIN_SEMA)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(AST_SRCS) $(COMMON_SRCS) $(SEMA_ANALYZER) $(MAIN_SEMA) $(LDFLAGS)

$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison $(BISON_FLAGS) -o $(BISON_C) $(BISON_FILE)

$(FLEX_C): $(FLEX_FILE)
	flex $(FLEX_FLAGS) -o $(FLEX_C) $(FLEX_FILE)

# -----------------------------
# Execução rápida (sintaxe)
# -----------------------------
run: $(EXEC_SYNTAX)
	@if [ -n "$(FILE)" ]; then \
	  echo ">> Rodando $(EXEC_SYNTAX) com arquivo: $(FILE)"; \
	  $(EXEC_SYNTAX) "$(FILE)"; \
	else \
	  echo ">> Rodando $(EXEC_SYNTAX) (stdin). Digite código e Ctrl+D para terminar."; \
	  $(EXEC_SYNTAX); \
	fi

# -----------------------------
# Testes
# -----------------------------
# test ⇒ roda TUDO (deixa o run.sh decidir as suítes existentes)
test: build
	@bash $(TEST_DIR)/run.sh

# test-syntax ⇒ só sintaxe
test-syntax: $(EXEC_SYNTAX)
	@bash $(TEST_DIR)/run.sh syntax

# test-semantic ⇒ só semântica
test-semantic: $(EXEC_SEMA)
	@bash $(TEST_DIR)/run.sh semantic

# -----------------------------
# Limpeza
# -----------------------------
clean:
	@echo "Limpando artefatos…"
	@rm -f $(EXEC_SYNTAX) $(EXEC_SEMA) $(BISON_C) $(BISON_H) $(FLEX_C)
