%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int lineno = 1;
%}

%%

"int"       { return INT_KW; }
"float"     { return FLOAT_KW; }
"bool"      { return BOOL_KW; }
"true"      { yylval.boolValue = 1; return BOOL_LIT; }
"false"     { yylval.boolValue = 0; return BOOL_LIT; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"for"       { return FOR; }
"return"    { return RETURN; }
"void"      { return VOID; }

[0-9]+"."[0-9]+   { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }
[0-9]+            { yylval.intValue = atoi(yytext); return INT_LIT; }

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.stringValue = strdup(yytext); 
    return IDENTIFIER; 
}

[ \t]+            { /* ignora espaços e tabulações */ }
\n                { lineno++; }

"//".*            { /* Ignora comentários de linha única */ }

"+"               { return PLUS; }
"-"               { return MINUS; }
"*"               { return TIMES; }
"/"               { return DIVIDE; }

"=="              { return EQ; }      
"!="              { return NEQ; }     
"<"               { return LT; }      
">"               { return GT; }      
"<="              { return LE; }      
">="              { return GE; }    

"("               { return LPAREN; }
")"               { return RPAREN; }
"{"               { return LBRACE; }
"}"               { return RBRACE; }
";"               { return SEMICOLON; }
","               { return COMMA; }

"&&"              { return AND; }
"||"              { return OR; }
"!"               { return NOT; }

"="               { return ASSIGN; }

.                 { fprintf(stderr, "Erro linha %d: Caractere não reconhecido: %s\n", lineno, yytext); }

%%

int yywrap(void) { 
    return 1; 
}