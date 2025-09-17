%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%}

%%
true              { yylval.boolValue = 1; return BOOL_LIT; }
false             { yylval.boolValue = 0; return BOOL_LIT; }

[0-9]+"."[0-9]+   { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }

[0-9]+            { yylval.intValue = atoi(yytext); return INT_LIT; }

"+"               { return PLUS; }
"-"               { return MINUS; }
"*"               { return TIMES; }
"/"               { return DIVIDE; }
"("               { return LPAREN; }
")"               { return RPAREN; }

[ \t]+            { /* ignore */ }
\n                { /* ignore */  }

.                 { fprintf(stderr, "Caractere não reconhecido: %s\n", yytext); }

%%

int yywrap(void) { return 1; }
