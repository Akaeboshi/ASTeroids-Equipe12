# Escopo da Linguagem (Projeto de Compilador)

Este documento define o **subconjunto da linguagem C++** que será incialmente suportado pelo compilador desenvolvido na disciplina **Compiladores 1**.
O escopo foi planejado para ser viável dentro do semestre e suficiente para todas as fases do compilador.

Este escopo poderá ser revisado e expandido ao longo da disciplina, conforme a evolução do projeto e a capacidade da equipe em implementar novas features.

---

## Tipos Primitivos

| Tipo     | Descrição                     | Exemplos           |
| -------- | ----------------------------- | ------------------ |
| `int`    | Números inteiros              | `10`, `42`         |
| `float`  | Números de ponto flutuante    | `3.14`, `0.5`      |
| `bool`   | Valores lógicos               | `true`, `false`    |
| `string` | Cadeias de caracteres (texto) | `"texto"`, `"abc"` |

**Observação sobre `string`**

No **C++ real**, cadeias de caracteres são representadas pela classe **`std::string`**, definida na biblioteca padrão (STL).
Isso implica em uso de namespaces (`std::`), inclusão de headers (`<string>`) e toda a infraestrutura de classes da linguagem.

Para **simplificação neste projeto**, consideraremos **`string` como um tipo primitivo**, no mesmo nível de `int`, `float` e `bool`.
Assim, o compilador pode lidar com variáveis e literais de string sem exigir suporte a namespaces, classes ou bibliotecas externas.

Caso seja decidido expandir o escopo futuramente, poderemos ajustar a gramática para reconhecer `std::string`, mas isso traria maior complexidade (namespaces e classes), que está fora do escopo mínimo inicial.

---

## Identificadores, Literais e Comentários

| Categoria       | Descrição                                                          | Exemplos                                |
| --------------- | ------------------------------------------------------------------ | ----------------------------------------|
| Identificadores | Devem começar com letra ou `_`, seguidos de letras, dígitos ou `_` | `x`, `contador`, `soma1`                |
| Literais        | Valores concretos escritos no código                               | `10`, `3.14`, `"abc"`, `true`           |
| Comentários     | Anotações ignoradas pelo compilador                                | `// comentário` <br> `/* comentário */` |

---

## Operadores

| Categoria      | Operadores                  | Exemplos                     |
|----------------|-----------------------------|------------------------------|
| Aritméticos    | `+`, `-`, `*`, `/`          | `x + y`, `a * b`             |
| Relacionais    | `==`, `!=`, `<`, `>`, `<=`, `>=` | `x == y`, `a < b`       |
| Lógicos        | `&&`, `||`, `!`             | `(x > 0 && y > 0)`           |
| Atribuição     | `=`                         | `x = 10`                     |

---

## Declarações de Variáveis

### Declaração Sem Inicialização

```cpp
int x;
float y;
bool ativo;
string nome;
```

### Declaração Com Inicialização
```cpp
int x = 5;
bool ativo = true;
string nome = "teste";
```

---

## Atribuições e Expressões

```cpp
x = 10;
y = x + 2.5;
ativo = (x > y);
nome = "teste";
```

---

## Estruturas de controle de fluxo

### If/Else
```cpp
if (x > 0) {
    y = y + 1;
} else {
    y = 0;
}
```

### While
```cpp
while (y < 10) {
    y = y + 1;
}
```

### For
```cpp
for (int i = 0; i < 10; i = i + 1) {
    x = x + i;
}
```

---

## Funções

### Definição
```cpp
int soma(int a, int b) {
    return a + b;
}
```

### Chamada
```cpp
int r = soma(2, 3);
```

---

## Palavras-chave reservadas

- `int`
- `float`
- `bool`
- `string`
- `if`
- `else`
- `while`
- `for`
- `true`
- `false`
- `return`
- `print`
- `read`

---

---

## Histórico de Versões

| Versão | Data       | Autor(es)                     | Descrição                              |
|--------|------------|-------------------------------|----------------------------------------|
| 1.0    | 13/09/2025 | [Antônio Júnior](https://github.com/antonioleaojr){target="_blank"}, [Heyttor Augusto](https://github.com/H3ytt0r62){target="_blank"}, [João Pedro Sampaio](https://github.com/jopesmp){target="_blank"}, [Lucas Heler](https://github.com/Akaeboshi){target="_blank"}, [Maciel Júnior](https://github.com/macieljuniormax){target="_blank"}  | Criação inicial do documento de escopo |
