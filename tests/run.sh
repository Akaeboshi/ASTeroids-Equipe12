#!/usr/bin/env bash
# Runner de testes — varre tests/*/{ok,err} e roda tudo em ordem fixa.
# Suítes suportadas: lexer → syntax → semantic → intermediate → generation
# Regras:
#  - lexer: normaliza a saída do driver e compara tokens/mensagens com expected/
#  - syntax: extrai "AST (Formatada)" e compara com .golden (se existir)
#  - semantic (e outras): valida apenas pelo exit code
#
# Uso:
#  ./run.sh             # Executa todas as suítes na ordem padrão
#  ./run.sh SUITE ...   # Executa apenas as suítes especificadas (ex.: ./run.sh lexer syntax)

set -u -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"

# ------------------------------------------------------------------------------
# Cores e estilos
# ------------------------------------------------------------------------------
if [[ -t 1 ]] && command -v tput >/dev/null 2>&1 && [[ $(tput colors) -ge 8 ]]; then
  BOLD="$(tput bold)"; DIM="$(tput dim)"; RESET="$(tput sgr0)"
  GREEN="$(tput setaf 2)"; RED="$(tput setaf 1)"; CYAN="$(tput setaf 6)"; YELLOW="$(tput setaf 3)"
else
  BOLD=""; DIM=""; RESET=""; GREEN=""; RED=""; CYAN=""; YELLOW=""
fi

# ------------------------------------------------------------------------------
# Emojis por tipo de teste
# ------------------------------------------------------------------------------
EMOJI_LEXER="🔤"
EMOJI_SYNTAX="🧩"
EMOJI_SEMANTIC="🧠"
EMOJI_INTERMEDIATE="🏗️"
EMOJI_GENERATION="⚙️"
EMOJI_DEFAULT="🧪"

OK_EMOJI="🟢"
ERR_EMOJI="🔴"

# ------------------------------------------------------------------------------
# Seleção de binário por suíte
# ------------------------------------------------------------------------------
bin_for_suite() {
  case "$1" in
    lexer)          echo "${ROOT_DIR}/src/scanner" ;;
    syntax)             echo "${ROOT_DIR}/src/parser" ;;
    semantic)           echo "${ROOT_DIR}/src/analyzer" ;;
    intermediate)       echo "${ROOT_DIR}/src/irgen" ;;
    generation) echo "${ROOT_DIR}/src/jsgen" ;;
    *)                  echo "${ROOT_DIR}/src/parser" ;;
  esac
}

# Variáveis globais para a suíte corrente
CURRENT_SUITE=""
BIN=""

# ------------------------------------------------------------------------------
# Funções utilitárias
# ------------------------------------------------------------------------------
line() { printf "%s\n" "────────────────────────────────────────────────────────"; }

suite_title() {
  local raw="$1"
  case "$raw" in
    lexer)          echo "${EMOJI_LEXER}  ${BOLD}Lexical Analysis${RESET}" ;;
    syntax)             echo "${EMOJI_SYNTAX}  ${BOLD}Syntax Analysis${RESET}" ;;
    semantic)           echo "${EMOJI_SEMANTIC}  ${BOLD}Semantic Analysis${RESET}" ;;
    intermediate)       echo "${EMOJI_INTERMEDIATE}  ${BOLD}Intermediate Representation${RESET}" ;;
    generation) echo "${EMOJI_GENERATION}  ${BOLD}Code Generation${RESET}" ;;
    *)                  echo "${EMOJI_DEFAULT}  ${BOLD}${raw^}${RESET}" ;;
  esac
}

# Extrai apenas a AST formatada (linhas após o cabeçalho)
extract_pretty() {
  # STDIN => saída do parser
  sed -e 's/\r$//' -e '/./,$!d' \
    | awk 'found{print} /^=== AST \(Formatada\) ===$/{found=1}' \
    | sed -e '1{/^=== AST (Formatada) ===$/d;}' -e '$a\'
}

suite_has_files() {
  local ok_dir="$1" err_dir="$2"
  shopt -s nullglob
  local ok_files=( "$ok_dir"/ok_*.in )
  local err_files=( "$err_dir"/err_*.in )
  [[ ${#ok_files[@]} -gt 0 || ${#err_files[@]} -gt 0 ]]
}

pass=0
fail=0

# ------------------------------------------------------------------------------
# Casos genéricos (syntax/semantic/…)
# ------------------------------------------------------------------------------
run_ok_case() {
  local file="$1"            # .../ok_XXXX_nome.in
  local base; base="$(basename "$file")"
  local golden="${file%.in}.golden"

  local out; out="$("$BIN" "$file" 2>&1)"
  local status=$?

  if [[ $status -ne 0 ]]; then
    printf "%b %s%s%s (esperado sucesso) → retorno %d\n" "$ERR_EMOJI" "$RED" "$base" "$RESET" "$status"
    echo "$out"
    ((fail++))
    return
  fi

  if [[ "$CURRENT_SUITE" == "syntax" ]]; then
    # extrai apenas a AST formatada e compara golden (se existir)
    local pretty
    pretty="$(printf "%s" "$out" | extract_pretty)"

    if [[ "${SHOW_PRETTY:-0}" != "0" ]]; then
      echo
      echo "${DIM}--- PREVIEW (${base}) ---${RESET}"
      printf "%s" "$pretty"
      echo "${DIM}--- FIM PREVIEW ---${RESET}"
    fi

    if [[ -f "$golden" ]]; then
      if diff -u --strip-trailing-cr <(sed -e '$a\' "$golden") <(printf "%s\n" "$pretty") > /tmp/diff.$$ 2>&1; then
        printf "%b %s%s%s (bateu com golden)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
        ((pass++))
      else
        printf "%b %s%s%s (divergiu do golden)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
        cat /tmp/diff.$$
        ((fail++))
      fi
      rm -f /tmp/diff.$$ || true
    else
      printf "%b %s%s%s (Passou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    fi
  else
    # suites não-syntax: apenas status importa
    printf "%b %s%s%s (Passou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
    ((pass++))
  fi
}

run_err_case() {
  local file="$1"
  local base; base="$(basename "$file")"
  local out; out="$("$BIN" "$file" 2>&1)"
  local status=$?

  if [[ $status -ne 0 ]]; then
    printf "%b %s%s%s (Falhou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
    ((pass++))
  else
    printf "%b %s%s%s (Deveria falhar, mas passou)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
    echo "$out"
    ((fail++))
  fi
}

# ------------------------------------------------------------------------------
# Casos específicos do LÉXICO (normaliza tokens/mensagens)
# ------------------------------------------------------------------------------
run_ok_case_lex() {
  local file="$1"                        # tests/lexer/ok/ok_*.in
  local base; base="$(basename "$file")" # ok_*.in
  local name="${base%.in}"               # ok_*
  local expected="${ok_dir}/${name}.tokens"
  local tmp="/tmp/${name}.out.$$"

  local out; out="$("$BIN" < "$file" 2>&1)"
  local status=$?

  if [[ $status -ne 0 ]]; then
    printf "%b %s%s%s (esperado sucesso) → retorno %d\n" "$ERR_EMOJI" "$RED" "$base" "$RESET" "$status"
    echo "$out"
    ((fail++)); return
  fi

  # NORMALIZA: extrai só o nome do token da linha "Token: NAME | ..."
  printf "%s\n" "$out" \
    | awk '/^Token:/ {print $2} /^Erro lexico/ {print "ERROR"}' \
    > "$tmp"

  if [[ -f "$expected" ]]; then
    if diff -u --strip-trailing-cr "$expected" "$tmp" > /tmp/diff.$$ 2>&1; then
      printf "%b %s%s%s (tokens OK)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    else
      printf "%b %s%s%s (tokens divergiram)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
      cat /tmp/diff.$$
      ((fail++))
    fi
    rm -f /tmp/diff.$$ || true
  else
    printf "%b %s%s%s (Passou, mas expected .tokens não existe)\n" "$YELLOW" "$base" "$RESET" ""
    ((pass++))
  fi

  rm -f "$tmp" || true
}

run_err_case_lex() {
  local file="$1"                        # tests/lexer/err/err_*.in
  local base; base="$(basename "$file")" # err_*.in
  local name="${base%.in}"               # err_*
  local expected="${err_dir}/${name}.tokens"
  local tmp="/tmp/${name}.out.$$"

  local out; out="$("$BIN" < "$file" 2>&1)"

  printf "%s\n" "$out" \
    | awk '/^Token:/ {print $2} /^Erro lexico/ {print "ERROR"}' \
    > "$tmp"

  if [[ -f "$expected" ]]; then
    if diff -u --strip-trailing-cr "$expected" "$tmp" > /tmp/diff.$$ 2>&1; then
      printf "%b %s%s%s (Falhou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    else
      printf "%b %s%s%s (mensagem de erro divergente)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
      cat /tmp/diff.$$
      ((fail++))
    fi
    rm -f /tmp/diff.$$ || true
  else
    printf "%b %s%s%s (Expected .out não encontrado)\n" "$YELLOW" "$base" "$RESET" ""
    ((pass++))
  fi

  rm -f "$tmp" || true
}

# ------------------------------------------------------------------------------
# Casos específicos de GERAÇÃO (JS) — usa .golden
# ------------------------------------------------------------------------------
run_ok_case_generation() {
  local file="$1"                        # tests/generation/ok/ok_*.in
  local base; base="$(basename "$file")" # ok_*.in
  local name="${base%.in}"               # ok_*
  local golden="${ok_dir}/${name}.golden"

  local out; out="$("$BIN" "$file" 2>&1)"
  local status=$?

  if [[ $status -ne 0 ]]; then
    printf "%b %s%s%s (esperado sucesso) → retorno %d\n" "$ERR_EMOJI" "$RED" "$base" "$RESET" "$status"
    echo "$out"
    ((fail++)); return
  fi

  if [[ -f "$golden" ]]; then
    if diff -u --strip-trailing-cr <(sed -e '$a\' "$golden") <(printf "%s\n" "$out") > /tmp/diff.$$ 2>&1; then
      printf "%b %s%s%s (JS bateu com .golden)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    else
      printf "%b %s%s%s (JS divergente do .golden)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
      cat /tmp/diff.$$
      ((fail++))
    fi
    rm -f /tmp/diff.$$ || true
  else
    printf "%b %s%s%s (Passou, mas .golden não existe)\n" "$YELLOW" "$base" "$RESET" ""
    ((pass++))
  fi
}

run_err_case_generation() {
  local file="$1"                        # tests/generation/err/err_*.in
  local base; base="$(basename "$file")" # err_*.in

  local out; out="$("$BIN" "$file" 2>&1)"
  local status=$?

  if [[ $status -ne 0 ]]; then
    printf "%b %s%s%s (Falhou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
    ((pass++))
  else
    printf "%b %s%s%s (Deveria falhar, mas passou)\n" "$ERR_EMOJI" "$RED" "$base" "$RESET"
    echo "$out"
    ((fail++))
  fi
}

# ------------------------------------------------------------------------------
# Execução de uma suíte
# ------------------------------------------------------------------------------
run_suite() {
  local suite_name="$1"
  local suite_dir="$ROOT_DIR/tests/$suite_name"
  local ok_dir="$suite_dir/ok"
  local err_dir="$suite_dir/err"
  [[ -d "$suite_dir" ]] || return 0
  suite_has_files "$ok_dir" "$err_dir" || return 0

  CURRENT_SUITE="$suite_name"
  BIN="$(bin_for_suite "$suite_name")"

  if [[ ! -x "$BIN" ]]; then
    echo "${YELLOW}Aviso:${RESET} binário não encontrado para '${suite_name}': $BIN"
    echo "Compile antes (ex.: make). Pulando a suíte '${suite_name}'."
    return 0
  fi

  echo
  line
  printf "%s %s\n" "$(suite_title "$suite_name")" "${DIM}(${suite_name})${RESET}"
  line

  shopt -s nullglob
  local f

  # Léxico: tratamento próprio
  if [[ "$CURRENT_SUITE" == "lexer" ]]; then
    for f in "$ok_dir"/ok_*.in; do
      [[ -e "$f" ]] || break
      run_ok_case_lex "$f"
    done

    echo
    printf "%sERR (devem falhar)%s\n" "$YELLOW" "$RESET"
    for f in "$err_dir"/err_*.in; do
      [[ -e "$f" ]] || break
      run_err_case_lex "$f"
    done
    return
  fi

  # Geração (JS): usa .golden
  if [[ "$CURRENT_SUITE" == "generation" ]]; then
    for f in "$ok_dir"/ok_*.in; do
      [[ -e "$f" ]] || break
      run_ok_case_generation "$f"
    done

    echo
    printf "%sERR (devem falhar)%s\n" "$YELLOW" "$RESET"
    for f in "$err_dir"/err_*.in; do
      [[ -e "$f" ]] || break
      run_err_case_generation "$f"
    done
    return
  fi

  # Demais suítes
  for f in "$ok_dir"/ok_*.in; do
    [[ -e "$f" ]] || break
    run_ok_case "$f"
  done

  echo
  printf "%sERR (devem falhar)%s\n" "$YELLOW" "$RESET"
  for f in "$err_dir"/err_*.in; do
    [[ -e "$f" ]] || break
    run_err_case "$f"
  done
}

# ------------------------------------------------------------------------------
# Execução
# ------------------------------------------------------------------------------
echo "${BOLD}Executando testes…${RESET}"

if [[ $# -gt 0 ]]; then
  ordered_suites=("$@")
else
  ordered_suites=(lexer syntax semantic intermediate generation)
fi

for suite_name in "${ordered_suites[@]}"; do
  run_suite "$suite_name"
done

echo
line
printf "%sResumo:%s %s%d%s passaram, %s%d%s falharam.\n" "$BOLD" "$RESET" "$GREEN" "$pass" "$RESET" "$RED" "$fail" "$RESET"
line

exit $(( fail == 0 ? 0 : 1 ))
