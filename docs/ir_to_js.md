# Referência de Tradução: IR (Intermediate Representation) para JavaScript

**Contexto:** Antes de implementar o backend, esta documentação descreve como cada instrução da nossa Representação Intermediária (IR) é traduzida para o código JavaScript equivalente.

**Objetivo:** Servir como documentação de referência completa para a tradução de IR para JavaScript.

---

## 1. Definição do Formato de Funções JS

Aqui definiremos o formato de *wrapper* para as funções geradas em JavaScript, que encapsularão o código traduzido das instruções IR.

* **Convenção de Nomenclatura:** Como as funções IR serão nomeadas no JS?
    * *Exemplo:* A função IR `func@main` se torna `function main(...) { ... }`
* **Assinatura da Função (Parâmetros e Retorno):** Como os parâmetros e o valor de retorno serão manipulados?
* **Estrutura Base:**
    ```javascript
    // Formato Básico de Função JS
    function [NOME_FUNCAO]([PARAMETROS]) {
        // [CÓDIGO TRADUZIDO DAS INSTRUÇÕES IR]
    }
    ```

---

## 2. Regras para Temporários e Variáveis

Esta seção define como as variáveis (locais, globais) e os registradores temporários da IR são representados em JavaScript.

* **Variáveis Locais:**
    * *Mapeamento:* Devem ser declaradas com `let` ou `const` no escopo da função JS.
    * *Exemplo:* `var x` (IR) &rarr; `let x;` (JS)
* **Registradores Temporários (`t0`, `t1`, ...):**
    * *Mapeamento:* Também declarados como variáveis locais no JS.
    * *Convenção:* Use um prefixo para evitar conflito de nomes, ex: `t0` (IR) &rarr; `let temp0;` (JS)

---

## 3. Mapeamento de Instruções IR &rarr; JavaScript

Esta é a seção central que mapeia cada instrução da IR para seu equivalente em JavaScript.

| Categoria | Instrução IR | Semântica | Código JavaScript Equivalente | Observações |
| :--- | :--- | :--- | :--- | :--- |
| **Aritmética** | `add t1, t2, t3` | $t_1 = t_2 + t_3$ | `let t1 = t2 + t3;` | |
| | `sub t1, t2, t3` | $t_1 = t_2 - t_3$ | `let t1 = t2 - t3;` | |
| | `mul t1, t2, t3` | $t_1 = t_2 * t_3$ | `let t1 = t2 * t3;` | |
| | `div t1, t2, t3` | $t_1 = t_2 / t_3$ | `let t1 = t2 / t3;` | Se a IR usa divisão inteira, considere `Math.trunc(t2 / t3)`. |
| | `mod t1, t2, t3` | $t_1 = t_2 \bmod t_3$ | `let t1 = t2 % t3;` | |
| | `neg t1, t2` | $t_1 = -t_2$ | `let t1 = -t2;` | |
| **Lógica Bitwise** | `and t1, t2, t3` | $t_1 = t_2 \text{ \& } t_3$ | `let t1 = t2 & t3;` | |
| | `or t1, t2, t3` | $t_1 = t_2 \text{ \textbar } t_3$ | `let t1 = t2 \| t3;` | |
| | `xor t1, t2, t3` | $t_1 = t_2 \text{ \^{} } t_3$ | `let t1 = t2 ^ t3;` | |
| | `shl t1, t2, t3` | *Shift Left:* $t_1 = t_2 \ll t_3$ | `let t1 = t2 << t3;` | |
| **Comparação** | `cmp_eq t1, t2, t3` | $t_1 = (t_2 = t_3)$ | `let t1 = t2 === t3;` | **Importante:** Usar igualdade estrita (`===`). |
| | `cmp_lt t1, t2, t3` | $t_1 = (t_2 < t_3)$ | `let t1 = t2 < t3;` | |
| | `cmp_ge t1, t2, t3` | $t_1 = (t_2 \geq t_3)$ | `let t1 = t2 >= t3;` | |
| | `cmp_neq t1, t2, t3` | $t_1 = (t_2 \neq t_3)$ | `let t1 = t2 !== t3;` | |
| **Memória** | `load t1, @var` | $t_1 = var$ | `let t1 = var;` | `@var` é uma variável no escopo JS. |
| | `store t1, @var` | $var = t_1$ | `var = t1;` | |
| | `load_arr t1, @arr, t2` | $t_1 = arr[t_2]$ | `let t1 = arr[t2];` | Assume que `@arr` é um objeto Array ou similar. |
| | `store_arr t1, @arr, t2` | $arr[t_2] = t_1$ | `arr[t2] = t1;` | |
| **Chamada de Função** | `call t1, @func, t2, t3` | $t_1 = func(t_2, t_3)$ | `let t1 = func(t2, t3);` | Presume que a função `@func` foi traduzida para `func`. |
| | `ret t1` | *Retorna* $t_1$ | `return t1;` | |
---

## 4. Tradução de Fluxo de Controle

Aqui definiremos como as estruturas de controle IR (labels, desvios) são transformadas em estruturas de controle JS (loops, condicionais, `goto` simulado).

* **Labels (Blocos Básicos):**
    * *Abordagem:* Simulação de GOTO usando *labels* de bloco JS, *switch-case* ou reestruturação para `while(true)` com `break`.
    * *Exemplo (Simulação com Switch):* Cada *label* IR (`L0`, `L1`) se torna um *case* dentro de um *loop*.
* **Branch Incondicional (`br L1`):**
    * *Tradução:* `continue L1_label;` (se usando labels de bloco JS) ou atualização de uma variável de controle de *switch*.
* **Branch Condicional (`brfalse t1, L1`):**
    * *Tradução:* `if (!t1) { goto L1; }` (usando a técnica de GOTO simulado).
* **Função de Retorno (`ret t0`):**
    * *Tradução:* `return t0;`

---

## 5. Exemplos Completos

Exemplos da tradução dos códigos de IR para JS

### Exemplo 5.1: Função Simples (Soma)

**Código IR:**

```ir
func@sum {
    t0 = add p0, p1
    ret t0
}
```

**Código JavaScript Traduzido:**

```
function sum(p0, p1) {
    let t0 = p0 + p1;
    return t0;
}
```


### Exemplo 5.2: Acesso e Modificação de Array (Memória)

**Código IR:**

```ir
func@IncrementaPrimeiro(p0) {
    // p0 é o array de entrada
    
    // t0 = arr[0]
    t0 = load_arr p0, 0 
    
    // t1 = t0 + 1
    t1 = add t0, 1 
    
    // arr[0] = t1
    store_arr t1, p0, 0 
    
    ret t1
}
```

**Código JavaScript Traduzido:**

```
function IncrementaPrimeiro(p0) {
    // Declaração de temporários
    let t0;
    let t1; 
    
    // t0 = arr[0]
    t0 = p0[0];
    
    // t1 = t0 + 1
    t1 = t0 + 1;
    
    // arr[0] = t1
    p0[0] = t1;
    
    return t1;
}
```
### Exemplo 5.3: Chamada de Função e Retorno

**Código IR:**

```ir
func@UsaSoma(p0, p1) {
    // t0 = sum(p0, p1)
    t0 = call @sum, p0, p1
    
    // t1 = t0 * 2
    t1 = mul t0, 2
    
    ret t1
}
```

**Código JavaScript Traduzido:**

```
// A função 'sum' deve ser definida em outro lugar ou no mesmo escopo
function UsaSoma(p0, p1) {
    // Declaração de temporários
    let t0;
    let t1;
    
    // t0 = call @sum, p0, p1
    t0 = sum(p0, p1);
    
    // t1 = t0 * 2
    t1 = t0 * 2;
    
    return t1;
}
```
### Exemplo 5.4: Expressão Lógica (AND/OR)

**Código IR:**

```ir
func@ChecaIntervalo(p0, p1) {
    // t0 = (p0 > 10)
    t0 = cmp_gt p0, 10
    
    // t1 = (p1 < 5)
    t1 = cmp_lt p1, 5
    
    // t2 = t0 AND t1
    t2 = and t0, t1 
    
    ret t2
}
```

**Código JavaScript Traduzido:**

```
function ChecaIntervalo(p0, p1) {
    // Declaração de temporários
    let t0;
    let t1;
    let t2;
    
    // t0 = (p0 > 10)
    t0 = p0 > 10;
    
    // t1 = (p1 < 5)
    t1 = p1 < 5;
    
    // t2 = t0 AND t1
    // Nota: Em JS, booleanos são coercidos para 0/1 em operações bitwise.
    // Se a IR trata 'true' como 1 e 'false' como 0:
    t2 = t0 & t1; 
    
    // Se a IR quer o resultado como booleano JS (true/false):
    // t2 = t0 && t1; 
    
    return t2;
}
```

