#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    NODE_DOCUMENT,
    NODE_HEADING,
    NODE_PARAGRAPH,
    NODE_TEXT,
    NODE_BOLD,
    NODE_LIST,
    NODE_ITEM
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int level;
    char value[256];

    struct ASTNode *children[64];
    int child_count;
} ASTNode;

typedef struct {
    TokenList *tokens;
    int pos;
} Parser;

ASTNode* parse_document(Parser *p);

ASTNode* parse_block(Parser *p);
ASTNode* parse_heading(Parser *p);
ASTNode* parse_paragraph(Parser *p);
ASTNode* parse_list(Parser *p);

void parse_inline(Parser *p, ASTNode *parent);
void parse_inline_until_newline(Parser *p, ASTNode *parent);
ASTNode* parse_text(Parser *p);
ASTNode* parse_bold(Parser *p);

void print_ast(ASTNode *node, int indent);

#endif