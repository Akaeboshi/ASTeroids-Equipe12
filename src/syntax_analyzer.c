#include <string.h>
#include "syntax_analyzer.h"
#include "ast.h"

extern int yyparse(void);
extern FILE *yyin;
extern int yylex_destroy(void);
extern Node *g_program_ast;
extern int g_parse_errors;

SyntaxResult syntax_parse_path(const char *path) {
    SyntaxResult r = { .parse_ok = 0, .parse_errors = 0, .ast = NULL };

    if (path && strcmp(path, "--") != 0) {
        yyin = fopen(path, "r");
        if (!yyin) {
            perror("Erro ao abrir arquivo");
            return r;
        }
    } else {
        yyin = stdin;
    }

    g_program_ast = NULL;
    g_parse_errors = 0;

    int rc = yyparse();

    if (yyin && yyin != stdin) fclose(yyin);

    if (rc == 0 && g_parse_errors == 0) {
        r.parse_ok = 1;
        r.ast = g_program_ast;
    } else {
        r.parse_ok = 0;
        r.ast = g_program_ast;
    }

    r.parse_errors = g_parse_errors;

    yylex_destroy();
    return r;
}
