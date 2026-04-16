#ifndef PARSER_INLINE_H
#define PARSER_INLINE_H

#include "parser_block.h"

ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines);
void parse_inline_until(Parser *p, ASTNode *parent, int end_type);

void parse_inline(Parser *p, ASTNode *parent);
void parse_inline_until_newline(Parser *p, ASTNode *parent);
ASTNode *parse_text(Parser *p);
ASTNode *parse_bold(Parser *p);

#endif
