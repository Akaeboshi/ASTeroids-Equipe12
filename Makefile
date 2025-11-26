# =============================
# Diretórios
# =============================
SRC_DIR     := src
INCLUDE_DIR := include
TEST_DIR    := tests

# =============================
# Executáveis
# =============================
EXEC_LEXER    := $(SRC_DIR)/scanner
EXEC_SYNTAX   := $(SRC_DIR)/parser
EXEC_SEMANTIC := $(SRC_DIR)/analyzer
EXEC_IR       := $(SRC_DIR)/irgen
JS_BIN        := $(SRC_DIR)/jsgen

# =============================
# Fontes comuns (AST + Tabela)
# =============================
AST_SRCS := \
  $(SRC_DIR)/ast_base.c \
  $(SRC_DIR)/ast_expr.c \
  $(SRC_DIR)/ast_printer.c \
  $(SRC_DIR)/ast_free.c

COMMON_SRCS := $(SRC_DIR)/symbol_table.c

# Núcleo comum
CORE_SRCS := \
  $(AST_SRCS) \
  $(COMMON_SRCS)

# =============================
# Analyzers e Drivers
# =============================
SEMANTIC_ANALYZER := $(SRC_DIR)/semantic_analyzer.c
SYNTAX_ANALYZER   := $(SRC_DIR)/syntax_analyzer.c

MAIN_LEXER     := $(SRC_DIR)/drivers/lexer_driver.c
MAIN_SYNTAX    := $(SRC_DIR)/drivers/syntax_driver.c
MAIN_SEMANTIC  := $(SRC_DIR)/drivers/semantic_driver.c
MAIN_IR_DRIVER := $(SRC_DIR)/drivers/ir_driver.c
MAIN_JS_DRIVER := $(SRC_DIR)/drivers/codegen_js_driver.c

# =============================
# IR e Codegen Sources
# =============================
IR_CORE_SRCS := \
  $(SRC_DIR)/ir.c \
  $(SRC_DIR)/ir_builder.c \
  $(SRC_DIR)/ir_printer.c

# IR completo (irgen)
IR_SRCS := \
  $(IR_CORE_SRCS) \
  $(SEMANTIC_ANALYZER) \
  $(SYNTAX_ANALYZER) \
  $(CORE_SRCS) \
  $(MAIN_IR_DRIVER)

# JS Backend (codegen + driver), reaproveitando IR_CORE_SRCS
JS_SRCS := \
  $(SRC_DIR)/codegen_js.c \
  $(MAIN_JS_DRIVER) \
  $(IR_CORE_SRCS) \
  $(SEMANTIC_ANALYZER) \
  $(SYNTAX_ANALYZER) \
  $(CORE_SRCS)

# =============================
# Bison/Flex
# =============================
BISON_FILE := $(SRC_DIR)/parser.y
FLEX_FILE  := $(SRC_DIR)/scanner.l

BISON_C := $(SRC_DIR)/parser.tab.c
BISON_H := $(SRC_DIR)/parser.tab.h
FLEX_C  := $(SRC_DIR)/lex.yy.c

FRONTEND_SRCS := $(BISON_C) $(FLEX_C)

# =============================
# Ferramentas e Flags
# =============================
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

# =============================
# Alvos principais
# =============================
.PHONY: all build ir js run run-ir test test-lexer test-syntax test-semantic test-ir clean

# =============================
# Build completo
# =============================
all: $(EXEC_LEXER) $(EXEC_SYNTAX) $(EXEC_SEMANTIC) $(EXEC_IR) $(JS_BIN)
build: all

# =============================
# Regras de geração Bison/Flex
# =============================
$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison $(BISON_FLAGS) -o $(BISON_C) $(BISON_FILE)

$(FLEX_C): $(FLEX_FILE)
	flex $(FLEX_FLAGS) -o $(FLEX_C) $(FLEX_FILE)


# =============================
# Regras de compilação
# =============================

# --- LÉXICO (scanner) ---
$(EXEC_LEXER): $(FRONTEND_SRCS) $(CORE_SRCS) $(MAIN_LEXER)
	$(CC) $(CFLAGS) -o $@ $(FRONTEND_SRCS) $(CORE_SRCS) $(MAIN_LEXER) $(LDFLAGS)

# --- SINTÁTICO (parser) ---
$(EXEC_SYNTAX): $(FRONTEND_SRCS) $(CORE_SRCS) $(SYNTAX_ANALYZER) $(MAIN_SYNTAX)
	$(CC) $(CFLAGS) -o $@ $(FRONTEND_SRCS) $(CORE_SRCS) $(SYNTAX_ANALYZER) $(MAIN_SYNTAX) $(LDFLAGS)

# --- SEMÂNTICO (analyzer) ---
$(EXEC_SEMANTIC): $(FRONTEND_SRCS) $(CORE_SRCS) $(SEMANTIC_ANALYZER) $(MAIN_SEMANTIC)
	$(CC) $(CFLAGS) -o $@ $(FRONTEND_SRCS) $(CORE_SRCS) $(SEMANTIC_ANALYZER) $(MAIN_SEMANTIC) $(LDFLAGS)

# --- IR (intermediate representation generator) ---
$(EXEC_IR): $(FRONTEND_SRCS) $(IR_SRCS)
	$(CC) $(CFLAGS) -o $@ $(FRONTEND_SRCS) $(IR_SRCS) $(LDFLAGS)

# --- JS Backend (jsgen) ---
js: $(JS_BIN)

$(JS_BIN): $(FRONTEND_SRCS) $(JS_SRCS)
	$(CC) $(CFLAGS) -o $@ $(FRONTEND_SRCS) $(JS_SRCS) $(LDFLAGS)

ir: $(EXEC_IR)

# =============================
# Execução rápida
# =============================

# Sintaxe (parser)
run: $(EXEC_SYNTAX)
	@if [ -n "$(FILE)" ]; then \
	  echo ">> Rodando $(EXEC_SYNTAX) com arquivo: $(FILE)"; \
	  $(EXEC_SYNTAX) "$(FILE)"; \
	else \
	  echo ">> Rodando $(EXEC_SYNTAX) (stdin). Digite código e Ctrl+D para terminar."; \
	  $(EXEC_SYNTAX); \
	fi

# IR (irgen)
run-ir: $(EXEC_IR)
	@if [ -n "$(FILE)" ]; then \
	  echo ">> Rodando $(EXEC_IR) com arquivo: $(FILE)"; \
	  $(EXEC_IR) "$(FILE)"; \
	else \
	  echo ">> Rodando $(EXEC_IR) (stdin). Digite código e Ctrl+D para terminar."; \
	  $(EXEC_IR); \
	fi

# =============================
# Testes
# =============================

test: build
	@bash $(TEST_DIR)/run.sh

test-lexer: $(EXEC_LEXER)
	@bash $(TEST_DIR)/run.sh lexer

test-syntax: $(EXEC_SYNTAX)
	@bash $(TEST_DIR)/run.sh syntax

test-semantic: $(EXEC_SEMANTIC)
	@bash $(TEST_DIR)/run.sh semantic

test-ir: $(EXEC_IR)
	@bash $(TEST_DIR)/run.sh intermediate

test-codegen: $(JS_BIN)
	@bash $(TEST_DIR)/run.sh generation

# =============================
# Limpeza
# =============================
clean:
	@echo "Limpando artefatos…"
	@rm -f \
	  $(EXEC_LEXER) \
	  $(EXEC_SYNTAX) \
	  $(EXEC_SEMANTIC) \
	  $(EXEC_IR) \
	  $(JS_BIN) \
	  $(BISON_C) $(BISON_H) $(FLEX_C)
