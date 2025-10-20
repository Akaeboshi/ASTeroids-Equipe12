#!/usr/bin/env bash
# Runner de testes — varre tests/*/{ok,err} e roda tudo em ordem fixa.
# Ordem: syntax → semantic → intermediate → generate
# Suporta cores e emojis (com fallback).

set -u -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN="${ROOT_DIR}/src/parser"

# ------------------------------------------------------------------------------
# Cores e estilos (somente se stdout é TTY e suporta cores)
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

# ------------------------------------------------------------------------------
# Sanity check
# ------------------------------------------------------------------------------
if [[ ! -x "$BIN" ]]; then
  echo "${RED}ERRO:${RESET} binário não encontrado: $BIN" >&2
  exit 2
fi

pass=0
fail=0

run_case() {
  local file="$1"
  local expect_ok="$2"   # "yes" | "no"

  local out; out="$("$BIN" "$file" 2>&1)"
  local status=$?
  local base; base="$(basename "$file")"

  if [[ "$expect_ok" == "yes" ]]; then
    if [[ $status -eq 0 ]]; then
      printf "%b %s%s%s (Passou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    else
      printf "%b %s → retorno %d%s\n" "$ERR_EMOJI" "$base" "$status" "$RESET"
      echo "$out"
      ((fail++))
    fi
  else
    if [[ $status -ne 0 ]]; then
      printf "%b %s%s%s (Falhou como esperado)\n" "$OK_EMOJI" "$GREEN" "$base" "$RESET"
      ((pass++))
    else
      printf "%b %s (Deveria falhar, mas passou)%s\n" "$ERR_EMOJI" "$base" "$RESET"
      echo "$out"
      ((fail++))
    fi
  fi
}

run_suite() {
  local suite_name="$1"
  local suite_dir="$ROOT_DIR/tests/$suite_name"
  local ok_dir="$suite_dir/ok"
  local err_dir="$suite_dir/err"

  [[ -d "$suite_dir" ]] || return 0  # ignora se não existir

  shopt -s nullglob
  local ok_files=( "$ok_dir"/ok_*.txt )
  local err_files=( "$err_dir"/err_*.txt )
  if [[ ${#ok_files[@]} -eq 0 && ${#err_files[@]} -eq 0 ]]; then
    return 0
  fi

  echo
  line
  printf "%s %s\n" "$(suite_title "$suite_name")" "${DIM}(${suite_name})${RESET}"
  line

  if [[ ${#ok_files[@]} -gt 0 ]]; then
    printf "%sOK (devem passar)%s\n" "$CYAN" "$RESET"
    for f in "${ok_files[@]}"; do
      run_case "$f" "yes"
    done
  fi

  if [[ ${#err_files[@]} -gt 0 ]]; then
    echo
    printf "%sERR (devem falhar)%s\n" "$YELLOW" "$RESET"
    for f in "${err_files[@]}"; do
      run_case "$f" "no"
    done
  fi
}

# ------------------------------------------------------------------------------
# Execução
# ------------------------------------------------------------------------------
echo "${BOLD}Executando testes…${RESET}"

# Ordem fixa das suítes
ordered_suites=(syntax semantic intermediate generate)

for suite_name in "${ordered_suites[@]}"; do
  run_suite "$suite_name"
done

echo
line
printf "%sResumo:%s %s%d%s passaram, %s%d%s falharam.\n" "$BOLD" "$RESET" "$GREEN" "$pass" "$RESET" "$RED" "$fail" "$RESET"
line

if [[ $fail -eq 0 ]]; then exit 0; else exit 1; fi
