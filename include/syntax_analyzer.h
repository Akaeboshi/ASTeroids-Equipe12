#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

#include <stdio.h>
#include "ast_base.h"

// Resultado padronizado da fase sintática
typedef struct {
    int parse_ok;       // 1 = sucesso sintático; 0 = falha
    int parse_errors;   // contador de erros de parser
    Node *ast;          // AST raiz (nulo se parse falhar)
} SyntaxResult;

/**
 * @brief Executa a análise sintática a partir de um caminho de arquivo ou stdin.
 * @param path Caminho do arquivo; se NULL ou "--", lê de stdin.
 * @return SyntaxResult com status e AST (se sucesso).
 */
SyntaxResult syntax_parse_path(const char *path);

#endif
