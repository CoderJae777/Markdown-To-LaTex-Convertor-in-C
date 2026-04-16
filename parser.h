// #ifndef PARSER_H
// #define PARSER_H

// #include <stdio.h>
// #include "lexer.h"

// typedef enum
// {
//     NODE_DOCUMENT,
//     NODE_HEADING,
//     NODE_PARAGRAPH,
//     NODE_TEXT,
//     NODE_BOLD,
//     NODE_ITALIC,
//     NODE_BOLD_ITALIC,
//     NODE_CODE_BLOCK,
//     NODE_CODE_INLINE,
//     NODE_LINK,
//     NODE_IMAGE,
//     NODE_BLOCKQUOTE,
//     NODE_MATH_INLINE,
//     NODE_MATH_BLOCK,
//     NODE_LIST,
//     NODE_TABLE,
//     NODE_TABLE_ROW,
//     NODE_TABLE_CELL,
//     NODE_ITEM
// } NodeType;

// typedef struct ASTNode
// {
//     NodeType type;
//     int level;
//     int ordered;
//     char value[256];
//     struct ASTNode *children[512];
//     int child_count;
// } ASTNode;

// typedef struct
// {
//     TokenList *tokens;
//     int pos;
// } Parser;

// ASTNode *parse_document(Parser *p);
// void generate_latex(ASTNode *root, FILE *out);

// ASTNode *parse_block(Parser *p);
// ASTNode *parse_heading(Parser *p);
// ASTNode *parse_paragraph(Parser *p);
// ASTNode *parse_list(Parser *p);

// void parse_inline(Parser *p, ASTNode *parent);
// void parse_inline_until_newline(Parser *p, ASTNode *parent);
// ASTNode *parse_text(Parser *p);
// ASTNode *parse_bold(Parser *p);

// void print_ast(ASTNode *node, int indent);

// #endif
