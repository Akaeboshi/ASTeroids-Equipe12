# Documento de Referência da Linguagem (Subconjunto C++ → JavaScript)

Este documento consolida o escopo definido para o compilador e organiza as informações necessárias para o desenvolvimento: tokens, palavras-chave, operadores e exemplos válidos/ inválidos. Ele servirá como referência para toda a equipe.

---

##  Tabela de Tokens

| Categoria       | Nome / Token       | Regex Simplificada          | Exemplos válidos             |
|-----------------|-------------------|-----------------------------|------------------------------|
| Identificadores | IDENT             | `[a-zA-Z_][a-zA-Z0-9_]*`   | `x`, `contador`, `_valor1`   |
| Números inteiros | INT_LITERAL      | `[0-9]+`                   | `0`, `42`, `1234`            |
| Números float   | FLOAT_LITERAL     | `[0-9]+\.[0-9]+`           | `3.14`, `0.5`, `10.0`        |
| Booleanos       | BOOL_LITERAL      | `true|false`               | `true`, `false`              |
| Strings         | STRING_LITERAL    | `"(.*?)"`                  | `"abc"`, `"texto"`           |
| Operadores aritméticos | PLUS, MINUS, STAR, SLASH | `\+`, `-`, `\*`, `/` | `+`, `-`, `*`, `/` |
| Operadores relacionais | EQ, NEQ, LT, GT, LE, GE | `==`, `!=`, `<`, `>`, `<=`, `>=` | `x == y`, `a < b` |
| Operadores lógicos | AND, OR        | `&&`, `\|\|`               | `x && y`, `a || b`           |
| Atribuição      | ASSIGN            | `=`                        | `x = 10`                     |
| Pontuação       | LPAR, RPAR, LBRACE, RBRACE, SEMI, COMMA | `\(`, `\)`, `{`, `}`, `;`, `,` | `( ) { } ; ,` |
| Comentário linha | COMMENT_LINE     | `//.*`                     | `// exemplo`                 |
| Comentário bloco | COMMENT_BLOCK    | `/\*[^*]*\*+(?:[^/*][^*]*\*+)*/` | `/* exemplo */` |

---

##  Palavras-chave Reservadas

```

int, float, bool, string,
if, else, while, for,
true, false,
return, print, read

````

 **Observação**: Identificadores não podem ter o mesmo nome das palavras-chave.

---

##  Precedência e Associatividade de Operadores

| Nível (maior → menor) | Operadores              | Associatividade |
|------------------------|-------------------------|-----------------|
| 1 (maior)             | `()` (parênteses)      | Esquerda → Direita |
| 2                     | `*`, `/`               | Esquerda → Direita |
| 3                     | `+`, `-`               | Esquerda → Direita |
| 4                     | `<`, `<=`, `>`, `>=`   | Esquerda → Direita |
| 5                     | `==`, `!=`             | Esquerda → Direita |
| 6                     | `&&`, `||`             | Esquerda → Direita |
| 7 (menor)             | `=` (atribuição)       | Direita → Esquerda |

---

##  Exemplos Mínimos

### Declarações de variáveis
```cpp
int x;
float y = 3.14;
bool ativo = true;
string nome = "teste";
````

### Expressões e atribuições

```cpp
x = 10;
y = x + 2.5;
ativo = (x > y);
nome = "abc";
```

### Controle de fluxo

```cpp
if (x > 0) {
    y = y + 1;
} else {
    y = 0;
}

while (y < 10) {
    y = y + 1;
}

for (int i = 0; i < 10; i = i + 1) {
    x = x + i;
}
```

### Funções

```cpp
int soma(int a, int b) {
    return a + b;
}

int r = soma(2, 3);
```

---

##  Exemplos Inválidos (para testes negativos)

```cpp
int 1abc;          // identificador não pode começar com número
float x = "abc";   // tipo incompatível
bool ativo = 10;   // tipo incompatível
string nome = abc; // string precisa de aspas
if x > 0 {         // parênteses obrigatórios na condição
    y = 1;
}
int soma(a, b) {   // parâmetros precisam de tipo
    return a + b;
}
```

| Versão | Data       | Autor(es)                                                                                                                                                 | Descrição                               |
|--------|------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------|
| 1.0    | 24/09/2025 | [Antônio Júnior](https://github.com/antonioleaojr), [Heyttor Augusto](https://github.com/H3ytt0r62), [João Pedro Sampaio](https://github.com/jopesmp), [Lucas Heler](https://github.com/Akaeboshi), [Maciel Júnior](https://github.com/macieljuniormax) | Criação inicial do documento de referência |

