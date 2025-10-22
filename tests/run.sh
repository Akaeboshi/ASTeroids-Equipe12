#!/usr/bin/env bash
# Runner de testes — varre tests/*/{ok,err} e roda tudo em ordem fixa.
# Usa .in como entrada e compara APENAS a "AST (Formatada)" com .golden (se existir).
# Ordem: syntax → semantic → intermediate → generate
# Suporta cores e emojis (com fallback).

set -u -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN="${ROOT_DIR}/src/parser"

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
EMOJI_SYNTAX="🧩"
EMOJI_SEMANTIC="🧠"
EMOJI_INTERMEDIATE="🏗️"
EMOJI_GENERATION="⚙️"
EMOJI_DEFAULT="🧪"

OK_EMOJI="🟢"
ERR_EMOJI="🔴"

# ------------------------------------------------------------------------------
# Funções utilitárias
# ------------------------------------------------------------------------------
line() { printf "%s\n" "────────────────────────────────────────────────────────"; }

suite_title() {
  local raw="$1"
  case "$raw" in
    syntax)       echo "${EMOJI_SYNTAX}  ${BOLD}Syntax Analysis${RESET}" ;;
    semantic)     echo "${EMOJI_SEMANTIC}  ${BOLD}Semantic Analysis${RESET}" ;;
    intermediate) echo "${EMOJI_INTERMEDIATE}  ${BOLD}Intermediate Representation${RESET}" ;;
    generate|generation) echo "${EMOJI_GENERATION}  ${BOLD}Code Generation${RESET}" ;;
    *)            echo "${EMOJI_DEFAULT}  ${BOLD}${raw^}${RESET}" ;;
  esac
}

# Extrai apenas a AST formatada (linhas após o cabeçalho)
extract_pretty() {
  # STDIN => saída do parser
  # remove CR, drop linhas vazias iniciais, corta a partir do cabeçalho, remove o cabeçalho, garante newline final
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

# ------------------------------------------------------------------------------
# Sanity check
# ------------------------------------------------------------------------------
if [[ ! -x "$BIN" ]]; then
  echo "${RED}ERRO:${RESET} binário não encontrado: $BIN" >&2
  exit 2
fi

pass=0
fail=0

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

  # extrai apenas a AST formatada
  local pretty
  pretty="$(printf "%s" "$out" | extract_pretty)"

  # preview opcional
  if [[ "${SHOW_PRETTY:-0}" != "0" ]]; then
    echo
    echo "${DIM}--- PREVIEW (${base}) ---${RESET}"
    printf "%s" "$pretty"
    echo "${DIM}--- FIM PREVIEW ---${RESET}"
  fi

  if [[ -f "$golden" ]]; then
    # normaliza golden idem (newline final)
    diff -u --strip-trailing-cr \
      <(sed -e '$a\' "$golden") \
      <(printf "%s\n" "$pretty") \
      > /tmp/diff.$$ 2>&1
    if [[ $? -eq 0 ]]; then
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

run_suite() {
  local suite_name="$1"
  local suite_dir="$ROOT_DIR/tests/$suite_name"
  local ok_dir="$suite_dir/ok"
  local err_dir="$suite_dir/err"
  [[ -d "$suite_dir" ]] || return 0
  suite_has_files "$ok_dir" "$err_dir" || return 0

  echo
  line
  printf "%s %s\n" "$(suite_title "$suite_name")" "${DIM}(${suite_name})${RESET}"
  line

  shopt -s nullglob
  local f
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
ordered_suites=(syntax semantic intermediate generate)
for suite_name in "${ordered_suites[@]}"; do
  run_suite "$suite_name"
done

echo
line
printf "%sResumo:%s %s%d%s passaram, %s%d%s falharam.\n" "$BOLD" "$RESET" "$GREEN" "$pass" "$RESET" "$RED" "$fail" "$RESET"
line

exit $(( fail == 0 ? 0 : 1 ))
