// Código gerado automaticamente a partir do IR

function fatorial(p0) {
  let t0 = p0, t1, t2, t3, t4, t5, t6, t7, t8, pc = 0;
  while (true) {
    switch (pc) {
      case 0:
        t1 = 1;
        t2 = t0 <= t1;
        if (!t2) { pc = 1; break; }
        t3 = 1;
        return t3;
        pc = 2; break;
        break;
      case 1: // L0
        t5 = 1;
        t6 = t4 - t5;
  t7 = fatorial(t6);
        t8 = t4 * t7;
        return t8;
        break;
      case 2: // L1
    }
    break;
  }
}

function main() {
  let t0, t1, t2, t3;
        t1 = 5;
  t2 = fatorial(t1);
        resultado = t2;
        return resultado;
}

function _entry() {
        return;
}

_entry();
