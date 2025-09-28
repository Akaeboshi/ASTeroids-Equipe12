%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%}

%option noinput nounput
%x COMMENT

%%

true                { yylval.boolValue = 1; return BOOL_LIT; /* bool true */ }
false               { yylval.boolValue = 0; return BOOL_LIT; /* bool false */}

[0-9]+"."[0-9]+     { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; /* float */}
[0-9]+              { yylval.intValue = atoi(yytext); return INT_LIT;             /* int   */}

[ \t\n]+            { /* ignora espaços e tabulações e quebra de linhas*/ }

"//".*              {                   /* Ignora comentários de linha única (estilo C++) */ }
"/*"                { BEGIN(COMMENT);   /* Entra no modo de comentário multilinha */         }
<COMMENT>"*/"       { BEGIN(INITIAL);   /* Sai do modo de comentário */                      }
<COMMENT>.|\n       {                   /* Ignora todo o conteúdo dentro do comentário */    }
<COMMENT><<EOF>>    {
                      fprintf(stderr, "Erro: Comentário multilinha não foi fechado.\\n");
                      yyterminate(); /* Aborta o scanner por erro grave */
                    }

"+"                 { return PLUS;   /* operador aritmético de adição */        }
"-"                 { return MINUS;  /* operador aritmético de subtração */     }
"*"                 { return TIMES;  /* operador aritmético de multiplicação */ }
"/"                 { return DIVIDE; /* operador aritmético de divisão */       }

"=="                { return EQ;  /* operador relacional de igualdade */        }
"!="                { return NEQ; /* operador relacional de desigualdade */     }
"<"                 { return LT;  /* operador relacional de menor que */        }
">"                 { return GT;  /* operador relacional de maior que */        }
"<="                { return LE;  /* operador relacional de menor ou igual a */ }
">="                { return GE;  /* operador relacional de maior ou igual a */ }

"("                 { return LPAREN; /* parêntese esquerdo */ }
")"                 { return RPAREN; /* parêntese direito */  }

"&&"                { return AND; /* operador lógico E */    }
"||"                { return OR;  /* operador lógico OU */   }
"!"                 { return NOT; /* operador lógico NÃO */  }

"="                 { return ASSIGN; /* operador de atribuição */ }

.                   { fprintf(stderr, "Caractere não reconhecido: %s\n", yytext); }

%%

int yywrap(void) { return 1; }
