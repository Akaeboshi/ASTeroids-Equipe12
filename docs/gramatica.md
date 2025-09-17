# Gramática Formal (CFG) do Subconjunto de C++

Este documento descreve a gramática livre de contexto (CFG) correspondente ao subconjunto da linguagem C++ definido no [**Escopo da Linguagem**](./escopo.md){target="_blank"}. Ela servirá como base para o desenvolvimento do **analisador sintático** utilizando Bison.

---

## Símbolo inicial
```
Program → DeclOrStmtList
```

```
DeclOrStmtList → (DeclOrStmt)*
DeclOrStmt     → Decl ';' | Stmt
```

---

## Declarações
### Tipos suportados:
```
Type → 'int' | 'float' | 'bool' | 'string'
```

### Declarações:
```
Decl → Type IDENTIFIER ( '=' Expr )?
```

#### Exemplos válidos:
```cpp
int x;
float y = 3.14;
bool ativo = true;
string nome;
```

---

## Comandos
### Blocos e lista de comandos:
```
Block   → '{' StmtList? '}'
StmtList→ (Stmt)+
```

### Comandos possíveis:
```
Stmt → Assign ';'
     | If
     | While
     | For
     | Expr ';'
```

### Atribuições:
```
Assign → IDENTIFIER '=' Expr
```

#### Exemplos válidos:
```cpp
x = 10;
y = x + 2.5;
ativo = (x > y);
nome = "teste";
```

### If/Else:
```
If → 'if' '(' Expr ')' Stmt ('else' Stmt)?
```

#### Exemplo válido:
```cpp
if (x > 0) {
    y = y + 1;
} else {
    y = 0;
}
```

### While:
```
While → 'while' '(' Expr ')' Stmt
```

#### Exemplo válido:
```cpp
while (y < 10) {
    y = y + 1;
}
```

### For:
```
For     → 'for' '(' ForInit? ';' ForCond? ';' ForStep? ')' Stmt
ForInit → Decl | Assign
ForCond → Expr
ForStep → Assign
```

**Exemplo válido:**
```cpp
for (int i = 0; i < 10; i = i + 1) {
    soma = soma + i;
}
```

---

## Precedência de Operadores

1. `()` — Parênteses
2. `!`, `-` — Unários
3. `*`, `/` — Multiplicativos
4. `+`, `-` — Aditivos
5. `<`, `<=`, `>`, `>=` — Relacionais
6. `==`, `!=` — Igualdade
7. `&&` — Lógico AND
8. `||` — Lógico OR

---

## Léxico (terminais)
- **Identificadores**: `IDENTIFIER`
- **Literais**:
  - `INT_LIT` → números inteiros (ex.: `10`, `42`)
  - `FLOAT_LIT` → números de ponto flutuante (ex.: `3.14`, `0.5`)
  - `STRING_LIT` → cadeias de caracteres entre aspas (ex.: `"abc"`)
  - `BOOL_LIT` → `true` | `false`
- **Palavras-chave**: `int`, `float`, `bool`, `string`, `if`, `else`, `while`, `for`
- **Símbolos/operadores**:
  `= + - * / == != < > <= >= && || ! ( ) { } ; ,`

---

## Histórico de Versões

| Versão | Data       | Autor(es)                     | Descrição                              |
|--------|------------|-------------------------------|----------------------------------------|
| 1.0    | 14/09/2025 | [Antônio Júnior](https://github.com/antonioleaojr){target="_blank"}, [Heyttor Augusto](https://github.com/H3ytt0r62){target="_blank"}, [João Pedro Sampaio](https://github.com/jopesmp){target="_blank"}, [Lucas Heler](https://github.com/Akaeboshi){target="_blank"}, [Maciel Júnior](https://github.com/macieljuniormax){target="_blank"}  | Primeira versão da gramática formal |
