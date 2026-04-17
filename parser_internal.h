#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "parser.h"

Token *peek(Parser *p);
Token *advance(Parser *p);
int match(Parser *p, int type);

ASTNode *create_node(NodeType type);
void add_child(ASTNode *parent, ASTNode *child);
void free_ast(ASTNode *node);

ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines);
void parse_inline_until(Parser *p, ASTNode *parent, int end_type);

#endif
