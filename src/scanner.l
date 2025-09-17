%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%}

%%

[0-9]+            { yylval.intValue = atoi(yytext); return NUM; }

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
