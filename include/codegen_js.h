#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ir.h"

/**
 * Gera código JavaScript a partir de um IrProgram.
 *
 */
void codegen_js_program(const IrProgram *prog, FILE *out);

#endif /* CODEGEN_H */
