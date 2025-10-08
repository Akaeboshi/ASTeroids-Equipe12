/* lexer_driver.c
   Programa de teste para o scanner:
   - bison -d parser.y  (gera parser.tab.h)
   - flex scanner.l     (gera lex.yy.c)
   - gcc lex.yy.c lexer_driver.c -lfl -o lexer_driver
   - ./lexer_driver < tests/lexer_tests.txt
*/

#include <stdio.h>
#include <string.h>
#include "parser.tab.h" /* precisa ter sido gerado por: bison -d parser.y */

extern int yylex(void);
extern char *yytext;
extern int yylineno;
extern int yycolumn;
extern int yyleng;

const char *token_name(int t) {
    switch(t) {
        case INT_LIT: return "INT_LIT";
        case FLOAT_LIT: return "FLOAT_LIT";
        case BOOL_LIT: return "BOOL_LIT";
        case STRING_LIT: return "STRING_LIT";
        case IDENT: return "IDENT";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case TIMES: return "TIMES";
        case DIVIDE: return "DIVIDE";
        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case LT: return "LT";
        case GT: return "GT";
        case LE: return "LE";
        case GE: return "GE";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case ASSIGN: return "ASSIGN";
        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case FOR: return "FOR";
        case FUNCTION: return "FUNCTION";
        case RETURN: return "RETURN";
        default: return "UNKNOWN";
    }
}

int main(void) {
    int tok;
    while ((tok = yylex()) != 0) {
        int startcol = yycolumn - yyleng + 1;
        if (startcol < 1) startcol = 1;
        int endcol = yycolumn - 1;
        printf("Token: %-12s | lexeme=\"%s\" | line: %d col: %d-%d\n",
               token_name(tok), yytext ? yytext : "", yylineno, startcol, endcol);
    }
    return 0;
}
