#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

/* ========== CONTEXT TYPES ========== */
#define CTX_BOLD 1
#define CTX_ITALIC 2
#define CTX_CODE 3
#define CTX_MATH 4
#define CTX_BLOCK_MATH 5
#define CTX_CODE_BLOCK 6
#define CTX_HEADING 7
#define CTX_BLOCKQUOTE 8
#define CTX_LIST_ITEM 9

/* ========== CONTEXT FRAME ========== */
typedef struct
{
    int type;                /* CTX_BOLD, CTX_ITALIC, etc. */
    char content[4096];      /* accumulated text in this context */
    int content_len;         /* how many bytes used */
    char opening_value[256]; /* what opened it ("**", "*", "`", etc.) */
    int line, col;           /* line and column where it started */
} ContextFrame;

/* Stack DataType */
typedef struct
{
    ContextFrame frames[32]; /* room for up to 32 nested levels */
    int depth;               /* current stack position (0 = empty) */
} ContextStack;
static void push_context(ContextStack *stack, int type, const char *opening_value, int line, int col);
static ContextFrame *pop_context(ContextStack *stack);
static ContextFrame *current_context(ContextStack *stack);
static void append_to_context(ContextStack *stack, const char *text);
static void write_preamble(FILE *out);
static void write_epilogue(FILE *out);
/* ========== FUNCTION DECLARATIONS ========== */
void parse_and_emit(TokenList *tokens, FILE *out);
void set_text(ContextStack *stack, char *value, FILE *fLatex);
int set_heading(ContextStack *stack, Token tok);
int end_heading(ContextStack *stack, FILE *fLatex);

#endif /* PARSER_H */