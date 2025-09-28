%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%x COMMENT
%}

%%
true              { yylval.boolValue = 1; return BOOL_LIT; }
false             { yylval.boolValue = 0; return BOOL_LIT; }

[0-9]+"."[0-9]+   { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return IDENT; }

[0-9]+            { yylval.intValue = atoi(yytext); return INT_LIT; }

/* Ignorar espaços em branco, tabulações, quebra de linhas e comentários */

[ \t\n]+            { /* ignora espaços e tabulações e quebra de linhas*/ }

"//".* { /* Ignora comentários de linha única (estilo C++) */ }

"/*"              { BEGIN(COMMENT); } /* Entra no modo de comentário multilinha */
<COMMENT>"*/"     { BEGIN(INITIAL); } /* Sai do modo de comentário */
<COMMENT>.|\n    { /* Ignora todo o conteúdo dentro do comentário */ }
<COMMENT><<EOF>>  { 
                      fprintf(stderr, "Erro: Comentário multilinha não foi fechado.\\n");
                      yyterminate(); /* Aborta o scanner por erro grave */
                  }

/* Operadores Aritméticos */
"+"               { return PLUS; }
"-"               { return MINUS; }
"*"               { return TIMES; }
"/"               { return DIVIDE; }

/* Operadores Relacionais */

"=="              { return EQ; }      
"!="              { return NEQ; }     
"<"               { return LT; }      
">"               { return GT; }      
"<="              { return LE; }      
">="              { return GE; }    

/* Símbolos */
"("               { return LPAREN; }
")"               { return RPAREN; }
"{"               { return LBRACE; }
"}"               { return RBRACE; }
";"               { return SEMICOLON;  }
","               { return COMMA;  }

/* Condicionais */

"if"              { return IF;       }
"else"            { return ELSE;     }
"while"           { return WHILE;    }
"for"             { return FOR;      }
"function"        { return FUNCTION; }
"return"          { return RETURN;      }

/* Operadores Lógicos */
"&&"              { return AND; }
"||"              { return OR; }
"!"               { return NOT; }

/* Operador de atribuição  */
"="               { return ASSIGN; }


.                 { fprintf(stderr, "Caractere não reconhecido: %s\n", yytext); }

%%

int yywrap(void) { return 1; }
