#ifndef PARSER_LATEX_H
#define PARSER_LATEX_H

#include "parser_block.h"

void generate_latex(ASTNode *root, FILE *out);
void print_ast(ASTNode *node, int indent);

#endif
