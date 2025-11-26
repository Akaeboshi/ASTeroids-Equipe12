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
    * *Exemplo:* `var x` (IR) $\rightarrow$ `let x;` (JS)
* **Registradores Temporários (`t0`, `t1`, ...):**
    * *Mapeamento:* Também declarados como variáveis locais no JS.
    * *Convenção:* Use um prefixo para evitar conflito de nomes, ex: `t0` (IR) $\rightarrow$ `let temp0;` (JS)
* **Variáveis Globais:**
    * *Mapeamento:* (Se aplicável) Como serão acessadas no escopo global do JS?

---

## 3. Mapeamento de Instruções IR $\rightarrow$ JavaScript

Esta é a seção central que mapeia cada instrução da IR para seu equivalente em JavaScript.

| Categoria | Instrução IR | Semântica | Código JavaScript Equivalente | Exemplo de IR | Exemplo de JS Traduzido |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Aritmética** | `add t1, t2, t3` | $t_1 = t_2 + t_3$ | `let t1 = t2 + t3;` | `add t5, t0, 10` | `let t5 = t0 + 10;` |
| | `sub t1, t2, t3` | $t_1 = t_2 - t_3$ | `let t1 = t2 - t3;` | | |
| | ... | | | | |
| **Atribuição** | `load t1, @var` | $t_1 = var$ | `let t1 = var;` | `load t0, @counter` | `let t0 = counter;` |
| | `store t1, @var` | $var = t_1$ | `var = t1;` | | |
| | ... | | | | |
| **Comparação** | `cmp_eq t1, t2, t3` | $t_1 = t_2 == t_3$ | `let t1 = t2 === t3;` | | |
| | ... | | | | |

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
