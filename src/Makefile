# Nome do executável final
EXEC = parser

# Arquivos-fonte do Bison e do Flex
BISON_FILE = parser.y
FLEX_FILE  = scanner.l

# Arquivos que o Bison vai gerar
BISON_C   = parser.tab.c
BISON_H   = parser.tab.h

# Arquivo gerado pelo Flex
FLEX_C    = lex.yy.c

# Nossos arquivos .c da AST
AST_SRCS = ast_base.c ast_expr.c ast_free.c ast_printer.c symbol_table.c
AST_OBJS = ast_base.o ast_expr.o ast_free.o ast_printer.o symbol_table.o

# Parâmetros opcionais ao Bison e Flex
BISON_FLAGS = -d   # -d gera o arquivo .h (token definitions)
FLEX_FLAGS  =

# Parâmetros de compilação
CC      = gcc
CFLAGS  = -I.   # Inclui o diretório atual para encontrar ast.h
LDFLAGS = -ll   # biblioteca do Flex (em algumas distros, pode ser -ll)

# Regra padrão (alvo 'all' vai gerar o executável)
all: $(EXEC)

# Regra para gerar o executável: depende dos arquivos gerados por Bison, Flex e AST
$(EXEC): $(BISON_C) $(FLEX_C) $(AST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(AST_OBJS) $(LDFLAGS)

# Regra para compilar arquivos .c da AST
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para rodar o Bison: gera parser.tab.c e parser.tab.h
$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison $(BISON_FLAGS) $(BISON_FILE)

# Regra para rodar o Flex: gera lex.yy.c
$(FLEX_C): $(FLEX_FILE) $(BISON_H)
	flex $(FLEX_FLAGS) $(FLEX_FILE)

# adiciona symbol_table aos arquivos
AST_SRCS = ast_base.c ast_expr.c ast_free.c ast_printer.c symbol_table.c

# Regra de limpeza: remove arquivos gerados
clean:
	rm -f $(EXEC) $(BISON_C) $(BISON_H) $(FLEX_C) $(AST_OBJS)

# Para forçar recompilação completa
rebuild: clean all

.PHONY: all clean rebuild