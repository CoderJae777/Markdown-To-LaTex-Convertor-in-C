// block-level parsing
// looks at start of each line to figure out the layout of the line (e.g. header, list, etc)

// inline-level parsing
// looks at symbols inside each line to figure out formatting of the line (e.g. bold, italics, etc)

// Tokens → [Block Parser] → Blocks → [Inline Parser] → Final AST

// e.g. line = # Hello **world**

// step 1 - block parsing
// input:
// HASH (#) → heading
// outputs:
// NODE_HEADING (level=1)
//     content = "Hello **world**"   ← still raw

// step 2 - inline parsing
// input:
// "Hello **world**"
// outputs:
// NODE_HEADING
// ├── NODE_TEXT "Hello "
// └── NODE_BOLD
//     └── NODE_TEXT "world"

// high-level structure

// 1. Parser utilities (peek, advance, match)
// 2. AST helpers (create_node, add_child)
// 3. Entry point (parse_document)
// 4. Block-level parsing
//    - parse_block
//    - parse_heading
//    - parse_list
//    - parse_paragraph
// 5. Inline-level parsing
//    - parse_inline
//    - parse_bold
//    - parse_text

// supports:
// Headings (#)
// Paragraphs
// Lists (-)
// Bold (**)
// Text

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_internal.h"

Token *peek(Parser *p)
{
    return &p->tokens->tokens[p->pos];
}

Token *advance(Parser *p)
{
    return &p->tokens->tokens[p->pos++];
}

int match(Parser *p, int type)
{
    if (peek(p)->type == type)
    {
        advance(p);
        return 1;
    }
    return 0;
}

ASTNode *create_node(NodeType type)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->level = 0;
    node->ordered = 0;
    node->value[0] = '\0';
    node->child_count = 0;
    return node;
}

void add_child(ASTNode *parent, ASTNode *child);

static void trim_trailing_whitespace(ASTNode *parent)
{
    // Parser should not format text - removed
}

static int is_whitespace_text(Token *t)
{
    if (t->type != TOK_TEXT)
        return 0;

    for (int i = 0; t->value[i]; i++)
    {
        if (t->value[i] != ' ' && t->value[i] != '\t')
            return 0;
    }

    return 1;
}

static int is_list_marker_token(Token *t, int *ordered)
{
    if (t->type == TOK_DASH)
    {
        if (ordered)
            *ordered = 0;
        return 1;
    }

    if (t->type == TOK_TEXT)
    {
        const char *value = t->value;
        int i = 0;

        while (value[i] == ' ' || value[i] == '\t')
            i++;

        while (value[i] >= '0' && value[i] <= '9')
            i++;

        if (i > 0 && value[i] == '.' && value[i + 1] == '\0')
        {
            if (ordered)
                *ordered = 1;
            return 1;
        }
    }

    return 0;
}

static int peek_list_marker(Parser *p, int *indent, int *ordered, int *marker_pos)
{
    int pos = p->pos;
    int total_indent = 0;

    while (pos < p->tokens->count)
    {
        Token *tok = &p->tokens->tokens[pos];

        if (tok->type == TOK_TEXT && is_whitespace_text(tok))
        {
            total_indent += strlen(tok->value);
            pos++;
            continue;
        }

        if (pos >= p->tokens->count || tok->type == TOK_NEWLINE)
            return 0;

        if (is_list_marker_token(tok, ordered))
        {
            if (indent)
                *indent = total_indent;
            if (marker_pos)
                *marker_pos = pos;
            return 1;
        }

        break;
    }

    return 0;
}

// Forward declarations for functions used in parse_list_at
static ASTNode *parse_code_block(Parser *p);

static ASTNode *parse_list_at(Parser *p, int base_indent)
{
    int indent, ordered, marker_pos;

    if (!peek_list_marker(p, &indent, &ordered, &marker_pos) || indent != base_indent)
        return NULL;

    ASTNode *list = create_node(NODE_LIST);
    list->ordered = ordered;

    while (peek_list_marker(p, &indent, &ordered, &marker_pos) && indent == base_indent)
    {
        while (p->pos < marker_pos)
            advance(p);

        advance(p);

        ASTNode *item = create_node(NODE_ITEM);
        parse_inline_until_newline(p, item);
        match(p, TOK_NEWLINE);

        // Handle nested block content (lists, code blocks, etc.)
        while (1)
        {
            int next_indent, next_ordered, next_marker_pos;
            if (!peek_list_marker(p, &next_indent, &next_ordered, &next_marker_pos))
            {
                // No more list markers, but check for other block content (code blocks, etc)
                // that should be part of this item
                Token *t = peek(p);

                // Check for code block
                if ((t->type == TOK_TEXT && is_whitespace_text(t)) ||
                    t->type == TOK_TRIPLE_BACKTICK)
                {
                    ASTNode *block = parse_code_block(p);
                    if (block)
                    {
                        add_child(item, block);
                        continue;
                    }
                }

                break;
            }
            if (next_indent <= base_indent)
                break;

            ASTNode *nested = parse_list_at(p, next_indent);
            if (!nested)
            {
                // nested list parse failed, check for other block content
                Token *t = peek(p);

                // Check for code block
                if ((t->type == TOK_TEXT && is_whitespace_text(t)) ||
                    t->type == TOK_TRIPLE_BACKTICK)
                {
                    ASTNode *block = parse_code_block(p);
                    if (block)
                    {
                        add_child(item, block);
                        continue;
                    }
                }
                break;
            }

            add_child(item, nested);
        }

        add_child(list, item);
    }

    return list;
}

static int get_line_end(Parser *p, int pos)
{
    while (pos < p->tokens->count &&
           p->tokens->tokens[pos].type != TOK_NEWLINE &&
           p->tokens->tokens[pos].type != TOK_EOF)
    {
        pos++;
    }
    return pos;
}

static int line_has_pipe(Parser *p, int start_pos, int end_pos)
{
    for (int i = start_pos; i < end_pos; i++)
    {
        if (p->tokens->tokens[i].type == TOK_PIPE)
            return 1;
    }
    return 0;
}

static void collect_separator_segment(Parser *p, int start_pos, int end_pos, char *buf, int max_len)
{
    int len = 0;

    for (int i = start_pos; i < end_pos && len + 1 < max_len; i++)
    {
        Token *t = &p->tokens->tokens[i];

        if (t->type == TOK_PIPE || t->type == TOK_NEWLINE)
            continue;

        if (t->type == TOK_DASH)
        {
            buf[len++] = '-';
            continue;
        }

        if (t->type == TOK_TEXT)
        {
            for (int j = 0; t->value[j] && len + 1 < max_len; j++)
                buf[len++] = t->value[j];
            continue;
        }

        for (int j = 0; t->value[j] && len + 1 < max_len; j++)
            buf[len++] = t->value[j];
    }

    buf[len] = '\0';
}

static int segment_is_table_separator(const char *segment)
{
    int i = 0;

    while (segment[i] == ' ')
        i++;

    if (segment[i] == ':')
        i++;

    int dashes = 0;
    while (segment[i] == '-')
    {
        dashes++;
        i++;
    }

    if (dashes == 0)
        return 0;

    if (segment[i] == ':')
        i++;

    while (segment[i] == ' ')
        i++;

    return segment[i] == '\0';
}

static int is_table_separator_line(Parser *p, int start_pos, int end_pos)
{
    int has_pipe = 0;
    int seg_start = start_pos;
    char segment[256];

    for (int i = start_pos; i <= end_pos; i++)
    {
        if (i == end_pos || p->tokens->tokens[i].type == TOK_PIPE)
        {
            if (i > seg_start)
            {
                collect_separator_segment(p, seg_start, i, segment, sizeof(segment));
                if (!segment_is_table_separator(segment))
                    return 0;
            }
            if (i < end_pos && p->tokens->tokens[i].type == TOK_PIPE)
                has_pipe = 1;
            seg_start = i + 1;
            continue;
        }
    }

    return has_pipe;
}

static int peek_table_row(Parser *p)
{
    int end = get_line_end(p, p->pos);
    return line_has_pipe(p, p->pos, end);
}

static int peek_table(Parser *p)
{
    if (!peek_table_row(p))
        return 0;

    int end = get_line_end(p, p->pos);
    if (end >= p->tokens->count || p->tokens->tokens[end].type != TOK_NEWLINE)
        return 0;

    int next = end + 1;
    if (next >= p->tokens->count || p->tokens->tokens[next].type == TOK_EOF)
        return 0;

    int next_end = get_line_end(p, next);
    return is_table_separator_line(p, next, next_end);
}

static ASTNode *parse_table_row(Parser *p)
{
    ASTNode *row = create_node(NODE_TABLE_ROW);

    while (peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF)
    {
        if (peek(p)->type == TOK_PIPE)
        {
            advance(p);
            continue;
        }

        ASTNode *cell = create_node(NODE_TABLE_CELL);
        parse_inline_until(p, cell, TOK_PIPE);
        trim_trailing_whitespace(cell);
        add_child(row, cell);

        if (peek(p)->type == TOK_PIPE)
            continue;
        break;
    }

    match(p, TOK_NEWLINE);
    return row;
}

static char alignment_for_segment(const char *segment)
{
    int i = 0;
    int left = 0;
    int right = 0;
    int dashes = 0;

    while (segment[i] == ' ')
        i++;

    if (segment[i] == ':')
    {
        left = 1;
        i++;
    }

    while (segment[i] == '-')
    {
        dashes++;
        i++;
    }

    if (dashes == 0)
        return '\0';

    if (segment[i] == ':')
    {
        right = 1;
        i++;
    }

    while (segment[i] == ' ')
        i++;

    if (segment[i] != '\0')
        return '\0';

    if (left && right)
        return 'c';
    if (right)
        return 'r';
    return 'l';
}

static void parse_table_alignment(Parser *p, char *align, int max_cols)
{
    int col = 0;
    int seg_start = p->pos;
    char segment[256];

    while (peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF &&
           col < max_cols)
    {
        if (peek(p)->type == TOK_PIPE)
        {
            int seg_end = p->pos;
            collect_separator_segment(p, seg_start, seg_end, segment, sizeof(segment));
            char c = alignment_for_segment(segment);
            if (c != '\0')
                align[col++] = c;
            advance(p);
            seg_start = p->pos;
            continue;
        }

        advance(p);
    }

    if (seg_start < p->pos && col < max_cols)
    {
        collect_separator_segment(p, seg_start, p->pos, segment, sizeof(segment));
        char c = alignment_for_segment(segment);
        if (c != '\0')
            align[col++] = c;
    }

    align[col] = '\0';
}

static ASTNode *parse_table(Parser *p)
{
    if (!peek_table(p))
        return NULL;

    ASTNode *table = create_node(NODE_TABLE);
    ASTNode *header = parse_table_row(p);
    add_child(table, header);

    char align[256] = {0};
    parse_table_alignment(p, align, 255);
    match(p, TOK_NEWLINE);

    strncpy(table->value, align, sizeof(table->value) - 1);
    table->value[sizeof(table->value) - 1] = '\0';

    while (peek_table_row(p))
    {
        ASTNode *row = parse_table_row(p);
        add_child(table, row);
    }

    return table;
}

static int peek_blockquote(Parser *p)
{
    int pos = p->pos;

    while (pos < p->tokens->count &&
           p->tokens->tokens[pos].type == TOK_TEXT &&
           is_whitespace_text(&p->tokens->tokens[pos]))
    {
        pos++;
    }

    return pos < p->tokens->count && p->tokens->tokens[pos].type == TOK_GT;
}

static int peek_code_block(Parser *p)
{
    int pos = p->pos;

    while (pos < p->tokens->count &&
           p->tokens->tokens[pos].type == TOK_TEXT &&
           is_whitespace_text(&p->tokens->tokens[pos]))
    {
        pos++;
    }

    return pos < p->tokens->count && p->tokens->tokens[pos].type == TOK_TRIPLE_BACKTICK;
}

static int peek_block_math(Parser *p);
ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines);

static int peek_block_math(Parser *p)
{
    int pos = p->pos;

    while (pos < p->tokens->count &&
           p->tokens->tokens[pos].type == TOK_TEXT &&
           is_whitespace_text(&p->tokens->tokens[pos]))
    {
        pos++;
    }

    return pos < p->tokens->count && p->tokens->tokens[pos].type == TOK_DOUBLE_DOL;
}

static ASTNode *parse_block_math(Parser *p)
{
    while (peek(p)->type == TOK_TEXT && is_whitespace_text(peek(p)))
        advance(p);

    if (!match(p, TOK_DOUBLE_DOL))
        return NULL;

    while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
        advance(p);
    match(p, TOK_NEWLINE);

    ASTNode *node = create_node(NODE_MATH_BLOCK);

    while (!peek_block_math(p) && peek(p)->type != TOK_EOF)
    {
        int line_start = p->pos;
        while (peek(p)->type != TOK_NEWLINE &&
               peek(p)->type != TOK_EOF &&
               !peek_block_math(p))
        {
            advance(p);
        }

        ASTNode *line = make_raw_text_from_tokens(p, line_start, p->pos, 0);
        add_child(node, line);

        if (peek(p)->type == TOK_NEWLINE)
            advance(p);
    }

    if (!match(p, TOK_DOUBLE_DOL))
        return node;

    if (peek(p)->type == TOK_NEWLINE)
        advance(p);

    return node;
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

static ASTNode *parse_code_block(Parser *p)
{
    while (peek(p)->type == TOK_TEXT && is_whitespace_text(peek(p)))
        advance(p);

    if (!match(p, TOK_TRIPLE_BACKTICK))
        return NULL;

    while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
        advance(p);
    match(p, TOK_NEWLINE);

    ASTNode *node = create_node(NODE_CODE_BLOCK);

    while (peek(p)->type != TOK_TRIPLE_BACKTICK &&
           peek(p)->type != TOK_EOF)
    {
        int line_start = p->pos;
        while (peek(p)->type != TOK_NEWLINE &&
               peek(p)->type != TOK_EOF &&
               peek(p)->type != TOK_TRIPLE_BACKTICK)
        {
            advance(p);
        }

        ASTNode *line = make_raw_text_from_tokens(p, line_start, p->pos, 0);
        int max_children = sizeof(node->children) / sizeof(node->children[0]);
        if (node->child_count < max_children)
            node->children[node->child_count++] = line;
        else
            free(line);

        if (peek(p)->type == TOK_NEWLINE)
            advance(p);
    }

    match(p, TOK_TRIPLE_BACKTICK);
    match(p, TOK_NEWLINE);
    return node;
}

static ASTNode *parse_blockquote(Parser *p)
{
    ASTNode *quote = create_node(NODE_BLOCKQUOTE);

    while (1)
    {
        int saved_pos = p->pos;

        while (peek(p)->type == TOK_TEXT && is_whitespace_text(peek(p)))
            advance(p);

        if (peek(p)->type != TOK_GT)
        {
            p->pos = saved_pos;
            break;
        }

        advance(p);

        if (peek(p)->type == TOK_TEXT && is_whitespace_text(peek(p)))
            advance(p);

        ASTNode *paragraph = create_node(NODE_PARAGRAPH);
        while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
            parse_inline(p, paragraph);

        match(p, TOK_NEWLINE);

        if (paragraph->child_count > 0)
            add_child(quote, paragraph);
        else
            free(paragraph);
    }

    return quote;
}

void add_child(ASTNode *parent, ASTNode *child)
{
    if (child)
        parent->children[parent->child_count++] = child;
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;

    for (int i = 0; i < node->child_count; i++)
        free_ast(node->children[i]);

    free(node);
}

ASTNode *parse_document(Parser *p)
{
    ASTNode *doc = create_node(NODE_DOCUMENT);
    while (peek(p)->type != TOK_EOF)
    {
        if (match(p, TOK_NEWLINE))
            continue;
        ASTNode *node = parse_block(p);
        if (node)
            add_child(doc, node);
    }
    return doc;
}

ASTNode *parse_block(Parser *p)
{
    if (peek_code_block(p))
        return parse_code_block(p);

    if (peek_block_math(p))
        return parse_block_math(p);

    if (peek_blockquote(p))
        return parse_blockquote(p);

    if (peek_table(p))
        return parse_table(p);

    if (peek_list_marker(p, NULL, NULL, NULL))
        return parse_list(p);

    Token *t = peek(p);

    if (is_whitespace_text(t) && p->pos + 1 < p->tokens->count)
    {
        Token *next = &p->tokens->tokens[p->pos + 1];
        if (next->type == TOK_HASH)
        {
            advance(p);
            return parse_heading(p);
        }
    }

    if (t->type == TOK_HASH)
        return parse_heading(p);

    return parse_paragraph(p);
}

ASTNode *parse_heading(Parser *p)
{
    Token *hash = advance(p);
    ASTNode *node = create_node(NODE_HEADING);
    node->level = strlen(hash->value);
    while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
        parse_inline(p, node);
    match(p, TOK_NEWLINE);
    return node;
}

ASTNode *parse_paragraph(Parser *p)
{
    ASTNode *node = create_node(NODE_PARAGRAPH);
    while (peek(p)->type != TOK_NEWLINE && peek(p)->type != TOK_EOF)
        parse_inline(p, node);
    match(p, TOK_NEWLINE);
    return node;
}

ASTNode *parse_list(Parser *p)
{
    return parse_list_at(p, 0);
}
