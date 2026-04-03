/* lexer.h - Markdown tokenizer */
#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_VALUE 256
#define MAX_TOKENS 8192

/* Token types */
#define TOK_TEXT 0
#define TOK_HASH 1
#define TOK_STAR 2
#define TOK_DOUBLE_STAR 3
#define TOK_TRIPLE_STAR 4
#define TOK_BACKTICK 5
#define TOK_DOUBLE_TICK 6
#define TOK_LBRACKET 7
#define TOK_RBRACKET 8
#define TOK_LPAREN 9
#define TOK_RPAREN 10
#define TOK_BANG 11
#define TOK_DOLLAR 12
#define TOK_DOUBLE_DOL 13
#define TOK_NEWLINE 14
#define TOK_GT 15   /* > for blockquote */
#define TOK_DASH 16 /* - for list */
#define TOK_EOF 17

typedef struct
{
    int type;
    char value[MAX_TOKEN_VALUE];
    int line;
    int col;
} Token;

typedef struct
{
    Token tokens[MAX_TOKENS];
    int count;
} TokenList;

void lex(const char *src, TokenList *list);
void print_tokens(const TokenList *list, FILE *out);
const char *token_type_name(int type);

#endif /* LEXER_H */
