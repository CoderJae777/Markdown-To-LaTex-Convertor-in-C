#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_inline.h"
#include "parser_block.h"

static ASTNode *parse_emphasis(Parser *p, int open_type, NodeType node_type, int *valid);
static ASTNode *parse_image(Parser *p);
static ASTNode *parse_link(Parser *p);
static ASTNode *parse_inline_code(Parser *p);
static ASTNode *parse_math_inline(Parser *p, int delimiter_type);

static int is_emphasis_token(int type)
{
    return type == TOK_STAR ||
           type == TOK_DOUBLE_STAR ||
           type == TOK_TRIPLE_STAR ||
           type == TOK_UNDERSCORE ||
           type == TOK_DOUBLE_UNDERSCORE ||
           type == TOK_TRIPLE_UNDERSCORE;
}

static NodeType node_type_for_emphasis(int token_type)
{
    switch (token_type)
    {
    case TOK_TRIPLE_STAR:
    case TOK_TRIPLE_UNDERSCORE:
        return NODE_BOLD_ITALIC;
    case TOK_DOUBLE_STAR:
    case TOK_DOUBLE_UNDERSCORE:
        return NODE_BOLD;
    default:
        return NODE_ITALIC;
    }
}

static void free_ast(ASTNode *node)
{
    if (!node)
        return;

    for (int i = 0; i < node->child_count; i++)
        free_ast(node->children[i]);

    free(node);
}

ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines)
{
    ASTNode *node = create_node(NODE_TEXT);
    int len = 0;

    for (int i = start_pos; i < end_pos; i++)
    {
        Token *t = &p->tokens->tokens[i];
        const char *value = t->value;
        int value_len = strlen(value);

        if (t->type == TOK_NEWLINE && preserve_newlines)
        {
            if (len + 1 < (int)sizeof(node->value))
                node->value[len++] = '\n';
            continue;
        }

        int to_copy = value_len;
        if (len + to_copy >= (int)sizeof(node->value))
            to_copy = sizeof(node->value) - len - 1;

        if (to_copy > 0)
        {
            memcpy(node->value + len, value, to_copy);
            len += to_copy;
            node->value[len] = '\0';
        }
    }

    node->value[len] = '\0';
    return node;
}

void parse_inline_until(Parser *p, ASTNode *parent, int end_type)
{
    while (peek(p)->type != end_type &&
           peek(p)->type != TOK_EOF &&
           peek(p)->type != TOK_NEWLINE)
    {
        parse_inline(p, parent);
    }
}

ASTNode *parse_image(Parser *p)
{
    int start_pos = p->pos;

    if (!match(p, TOK_BANG) || !match(p, TOK_LBRACKET))
    {
        p->pos = start_pos;
        return NULL;
    }

    ASTNode *node = create_node(NODE_IMAGE);

    parse_inline_until(p, node, TOK_RBRACKET);

    if (!match(p, TOK_RBRACKET) || !match(p, TOK_LPAREN))
    {
        free_ast(node);
        p->pos = start_pos;
        return NULL;
    }

    int url_start = p->pos;
    while (peek(p)->type != TOK_RPAREN && peek(p)->type != TOK_EOF)
        advance(p);

    if (!match(p, TOK_RPAREN))
    {
        free_ast(node);
        p->pos = start_pos;
        return NULL;
    }

    ASTNode *url_node = make_raw_text_from_tokens(p, url_start, p->pos - 1, 0);
    strncpy(node->value, url_node->value, sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    free_ast(url_node);

    return node;
}

ASTNode *parse_link(Parser *p)
{
    int start_pos = p->pos;

    if (!match(p, TOK_LBRACKET))
        return NULL;

    ASTNode *node = create_node(NODE_LINK);

    parse_inline_until(p, node, TOK_RBRACKET);

    if (!match(p, TOK_RBRACKET) || !match(p, TOK_LPAREN))
    {
        free_ast(node);
        p->pos = start_pos;
        return NULL;
    }

    int url_start = p->pos;
    while (peek(p)->type != TOK_RPAREN && peek(p)->type != TOK_EOF)
        advance(p);

    if (!match(p, TOK_RPAREN))
    {
        free_ast(node);
        p->pos = start_pos;
        return NULL;
    }

    ASTNode *url_node = make_raw_text_from_tokens(p, url_start, p->pos - 1, 0);
    strncpy(node->value, url_node->value, sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    free_ast(url_node);

    return node;
}

ASTNode *parse_inline_code(Parser *p)
{
    int start_pos = p->pos;
    match(p, TOK_BACKTICK);

    int content_start = p->pos;
    while (peek(p)->type != TOK_BACKTICK &&
           peek(p)->type != TOK_EOF &&
           peek(p)->type != TOK_NEWLINE)
    {
        advance(p);
    }

    if (!match(p, TOK_BACKTICK))
    {
        return make_raw_text_from_tokens(p, start_pos, p->pos, 0);
    }

    ASTNode *node = create_node(NODE_CODE_INLINE);
    ASTNode *text = make_raw_text_from_tokens(p, content_start, p->pos - 1, 0);
    strncpy(node->value, text->value, sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    free_ast(text);
    return node;
}

ASTNode *parse_math_inline(Parser *p, int delimiter_type)
{
    int start_pos = p->pos;
    match(p, delimiter_type);

    int content_start = p->pos;
    while (peek(p)->type != delimiter_type &&
           peek(p)->type != TOK_EOF &&
           peek(p)->type != TOK_NEWLINE)
    {
        advance(p);
    }

    if (!match(p, delimiter_type))
    {
        return make_raw_text_from_tokens(p, start_pos, p->pos, 0);
    }

    ASTNode *node = create_node(NODE_MATH_INLINE);
    ASTNode *text = make_raw_text_from_tokens(p, content_start, p->pos - 1, 0);
    strncpy(node->value, text->value, sizeof(node->value) - 1);
    node->value[sizeof(node->value) - 1] = '\0';
    free_ast(text);
    return node;
}

void parse_inline_until_newline(Parser *p, ASTNode *parent)
{
    while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
        parse_inline(p, parent);
}

ASTNode *parse_text(Parser *p)
{
    Token *t = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, t->value);
    return node;
}

ASTNode *parse_bold(Parser *p)
{
    int valid = 0;
    ASTNode *node = parse_emphasis(p, TOK_DOUBLE_STAR, NODE_BOLD, &valid);
    if (!valid)
        return node;
    return node;
}

static ASTNode *parse_emphasis(Parser *p, int open_type, NodeType node_type, int *valid)
{
    int start_pos = p->pos;
    match(p, open_type);

    ASTNode *node = create_node(node_type);

    while (peek(p)->type != open_type &&
           peek(p)->type != TOK_EOF &&
           peek(p)->type != TOK_NEWLINE)
    {
        Token *t = peek(p);

        if (is_emphasis_token(t->type))
        {
            int child_valid = 0;
            ASTNode *child = parse_emphasis(p, t->type, node_type_for_emphasis(t->type), &child_valid);
            if (!child_valid)
            {
                free_ast(node);
                *valid = 0;
                free_ast(child);
                return make_raw_text_from_tokens(p, start_pos, p->pos, 0);
            }
            add_child(node, child);
            continue;
        }

        if (t->type == TOK_TEXT)
        {
            ASTNode *text = parse_text(p);
            if (text)
                add_child(node, text);
            continue;
        }

        Token *unknown = advance(p);
        ASTNode *text = create_node(NODE_TEXT);
        strcpy(text->value, unknown->value);
        add_child(node, text);
    }

    if (!match(p, open_type))
    {
        free_ast(node);
        *valid = 0;
        return make_raw_text_from_tokens(p, start_pos, p->pos, 0);
    }

    *valid = 1;
    return node;
}

void parse_inline(Parser *p, ASTNode *parent)
{
    Token *t = peek(p);

    if (t->type == TOK_BANG && p->pos + 1 < p->tokens->count &&
        p->tokens->tokens[p->pos + 1].type == TOK_LBRACKET)
    {
        ASTNode *node = parse_image(p);
        if (node)
        {
            add_child(parent, node);
            return;
        }
    }

    if (t->type == TOK_LBRACKET)
    {
        ASTNode *node = parse_link(p);
        if (node)
        {
            add_child(parent, node);
            return;
        }
    }

    if (t->type == TOK_BACKTICK)
    {
        ASTNode *node = parse_inline_code(p);
        add_child(parent, node);
        return;
    }

    if (t->type == TOK_DOLLAR)
    {
        ASTNode *node = parse_math_inline(p, TOK_DOLLAR);
        add_child(parent, node);
        return;
    }

    if (is_emphasis_token(t->type))
    {
        int valid = 0;
        ASTNode *node = parse_emphasis(p, t->type, node_type_for_emphasis(t->type), &valid);
        add_child(parent, node);
        return;
    }

    if (t->type == TOK_TEXT)
    {
        ASTNode *text = parse_text(p);
        if (text)
            add_child(parent, text);
        return;
    }

    Token *unknown = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, unknown->value);
    add_child(parent, node);
}
