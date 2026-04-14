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
#include "parser.h"

static Token *peek(Parser *p)
{
    return &p->tokens->tokens[p->pos];
}

static Token *advance(Parser *p)
{
    if (p->pos < p->tokens->count)
        p->pos++;
    return &p->tokens->tokens[p->pos - 1];
}

static int match(Parser *p, int type)
{
    if (peek(p)->type == type)
    {
        advance(p);
        return 1;
    }
    return 0;
}

static ASTNode *create_node(NodeType type)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->level = 0;
    node->ordered = 0;
    node->value[0] = '\0';
    node->child_count = 0;
    return node;
}

static void add_child(ASTNode *parent, ASTNode *child);
static void free_ast(ASTNode *node);

static void trim_trailing_whitespace(ASTNode *parent)
{
    if (parent->child_count == 0)
        return;

    ASTNode *last = parent->children[parent->child_count - 1];
    if (last->type != NODE_TEXT)
        return;

    int len = strlen(last->value);
    while (len > 0 && (last->value[len - 1] == ' ' || last->value[len - 1] == '\t'))
    {
        last->value[--len] = '\0';
    }

    if (len == 0)
    {
        parent->child_count--;
        free(last);
    }
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

static void parse_inline_until(Parser *p, ASTNode *parent, int end_type);

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
static ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines);

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

static ASTNode *make_raw_text_from_tokens(Parser *p, int start_pos, int end_pos, int preserve_newlines)
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

static void parse_inline_until(Parser *p, ASTNode *parent, int end_type)
{
    while (peek(p)->type != end_type &&
           peek(p)->type != TOK_EOF &&
           peek(p)->type != TOK_NEWLINE)
    {
        parse_inline(p, parent);
    }
}

static ASTNode *parse_image(Parser *p)
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

static ASTNode *parse_link(Parser *p)
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

static ASTNode *parse_inline_code(Parser *p)
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

static ASTNode *parse_math_inline(Parser *p, int delimiter_type)
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
        {
            Token *space = peek(p);
            if (space->value[0] == ' ')
            {
                if (strlen(space->value) == 1)
                    advance(p);
                else
                    memmove(space->value, space->value + 1, strlen(space->value));
            }
        }

        ASTNode *paragraph = create_node(NODE_PARAGRAPH);
        int first = 1;
        while (peek(p)->type != TOK_NEWLINE &&
               peek(p)->type != TOK_EOF)
        {
            int before = paragraph->child_count;
            parse_inline(p, paragraph);

            if (first && paragraph->child_count > before)
            {
                ASTNode *child = paragraph->children[before];
                if (child->type == NODE_TEXT)
                {
                    while (child->value[0] == ' ')
                    {
                        memmove(child->value, child->value + 1, strlen(child->value));
                    }
                }
                first = 0;
            }
        }

        trim_trailing_whitespace(paragraph);
        match(p, TOK_NEWLINE);

        if (paragraph->child_count > 0)
            add_child(quote, paragraph);
        else
            free(paragraph);
    }

    return quote;
}

static void add_child(ASTNode *parent, ASTNode *child)
{
    if (!child)
        return;

    // merge adjacent TEXT nodes
    if (parent->child_count > 0)
    {
        ASTNode *last = parent->children[parent->child_count - 1];

        if (last->type == NODE_TEXT && child->type == NODE_TEXT)
        {
            int last_len = strlen(last->value);

            // prevent double spaces
            if (last_len > 0 &&
                last->value[last_len - 1] == ' ' &&
                child->value[0] == ' ')
            {
                memmove(child->value, child->value + 1, strlen(child->value));
            }

            int remaining = 255 - strlen(last->value) - 1;
            if (remaining > 0)
            {
                strncat(last->value, child->value, remaining);
            }

            free(child);
            return;
        }
    }

    int max_children = sizeof(parent->children) / sizeof(parent->children[0]);
    if (parent->child_count < max_children)
        parent->children[parent->child_count++] = child;
}

// entry point
ASTNode *parse_document(Parser *p)
{
    ASTNode *doc = create_node(NODE_DOCUMENT);

    while (peek(p)->type != TOK_EOF)
    {
        int start_pos = p->pos;

        if (match(p, TOK_NEWLINE))
            continue;

        ASTNode *node = parse_block(p);
        if (node)
            add_child(doc, node);

        if (p->pos == start_pos)
        {
            printf("Parser stuck at token %d, forcing advance\n", p->pos);
            advance(p);
        }
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

    // parse inline content
    parse_inline_until_newline(p, node);
    match(p, TOK_NEWLINE);

    return node;
}

ASTNode *parse_paragraph(Parser *p)
{
    ASTNode *node = create_node(NODE_PARAGRAPH);
    int first = 1;

    while (peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF)
    {
        int before = node->child_count;

        parse_inline(p, node);

        // trim leading space on first TEXT node
        if (first && node->child_count > before)
        {
            ASTNode *child = node->children[before];

            if (child->type == NODE_TEXT)
            {
                while (child->value[0] == ' ')
                {
                    memmove(child->value,
                            child->value + 1,
                            strlen(child->value));
                }
            }

            first = 0;
        }
    }

    trim_trailing_whitespace(node);
    match(p, TOK_NEWLINE);

    int has_content = 0;

    for (int i = 0; i < node->child_count; i++)
    {
        ASTNode *c = node->children[i];

        if (c->type != NODE_TEXT)
        {
            has_content = 1;
            break;
        }

        // check if TEXT has any non-space character
        for (int j = 0; c->value[j]; j++)
        {
            if (c->value[j] != ' ' && c->value[j] != '\t')
            {
                has_content = 1;
                break;
            }
        }

        if (has_content)
            break;
    }

    if (!has_content)
    {
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *parse_list(Parser *p)
{
    return parse_list_at(p, 0);
}

void parse_inline_until_newline(Parser *p, ASTNode *parent)
{
    int first = 1;

    while (peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF)
    {
        int before = parent->child_count;

        parse_inline(p, parent);

        // trim leading space on first TEXT node
        if (first && parent->child_count > before)
        {
            ASTNode *child = parent->children[before];

            if (child->type == NODE_TEXT)
            {
                while (child->value[0] == ' ')
                {
                    memmove(child->value, child->value + 1, strlen(child->value));
                }
            }

            first = 0;
        }
    }

    trim_trailing_whitespace(parent);
}

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

    // fallback - consume unknown tokens as TEXT
    Token *unknown = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, unknown->value);
    add_child(parent, node);
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

static void escape_latex(const char *text, FILE *out)
{
    for (int i = 0; text[i]; i++)
    {
        switch (text[i])
        {
        case '\\':
            fputs("\\textbackslash{}", out);
            break;
        case '{':
            fputs("\\{", out);
            break;
        case '}':
            fputs("\\}", out);
            break;
        case '#':
            fputs("\\#", out);
            break;
        case '$':
            fputs("\\$", out);
            break;
        case '%':
            fputs("\\%", out);
            break;
        case '&':
            fputs("\\&", out);
            break;
        case '_':
            fputs("\\_", out);
            break;
        case '^':
            fputs("\\^{}", out);
            break;
        case '~':
            fputs("\\~{}", out);
            break;
        default:
            fputc(text[i], out);
            break;
        }
    }
}

static void emit_inline(ASTNode *node, FILE *out)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_TEXT:
        escape_latex(node->value, out);
        break;

    case NODE_ITALIC:
        fputs("\\emph{", out);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("}", out);
        break;

    case NODE_BOLD:
        fputs("\\textbf{", out);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("}", out);
        break;

    case NODE_BOLD_ITALIC:
        fputs("\\textbf{\\emph{", out);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("}}", out);
        break;

    case NODE_CODE_INLINE:
        fputs("\\texttt{", out);
        escape_latex(node->value, out);
        fputs("}", out);
        break;

    case NODE_LINK:
        fprintf(out, "\\href{");
        escape_latex(node->value, out);
        fputs("}{", out);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("}", out);
        break;

    case NODE_IMAGE:
        fputs("\\includegraphics[height=1.5em]{", out);
        escape_latex(node->value, out);
        fputs("}", out);
        if (node->child_count > 0)
        {
            fputs("\\textit{", out);
            for (int i = 0; i < node->child_count; i++)
                emit_inline(node->children[i], out);
            fputs("}", out);
        }
        break;

    case NODE_MATH_INLINE:
        fputs("$", out);
        fputs(node->value, out);
        fputs("$", out);
        break;

    case NODE_MATH_BLOCK:
        fputs("\\[\n", out);
        for (int i = 0; i < node->child_count; i++)
        {
            if (node->children[i]->type == NODE_TEXT)
                fputs(node->children[i]->value, out);
            if (i < node->child_count - 1)
                fputs("\n", out);
        }
        fputs("\n\\]", out);
        break;

    default:
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        break;
    }
}

static void emit_block(ASTNode *node, FILE *out)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_DOCUMENT:
        fputs("\\documentclass{article}\n", out);
        fputs("\\usepackage[utf8]{inputenc}\n", out);
        fputs("\\usepackage[T1]{fontenc}\n", out);
        fputs("\\usepackage{lmodern}\n", out);
        fputs("\\usepackage{hyperref}\n", out);
        fputs("\\usepackage{graphicx}\n", out);
        fputs("\\setlength{\\parindent}{0pt}\n", out);
        fputs("\\setlength{\\parskip}{0.8\\baselineskip}\n\n", out);
        fputs("\\begin{document}\n\n", out);

        for (int i = 0; i < node->child_count; i++)
            emit_block(node->children[i], out);

        fputs("\\end{document}\n", out);
        break;

    case NODE_HEADING:
    {
        const char *cmd;
        if (node->level == 1)
            cmd = "section";
        else if (node->level == 2)
            cmd = "subsection";
        else if (node->level == 3)
            cmd = "subsubsection";
        else if (node->level == 4)
            cmd = "paragraph";
        else
            cmd = "subparagraph";

        fprintf(out, "\\%s{", cmd);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("}\n\n", out);
        break;
    }

    case NODE_PARAGRAPH:
        fputs("\\noindent ", out);
        for (int i = 0; i < node->child_count; i++)
            emit_inline(node->children[i], out);
        fputs("\n\n", out);
        break;

    case NODE_BLOCKQUOTE:
        fputs("\\begin{quote}\n", out);
        for (int i = 0; i < node->child_count; i++)
            emit_block(node->children[i], out);
        fputs("\\end{quote}\n\n", out);
        break;

    case NODE_CODE_BLOCK:
        fputs("\\begin{verbatim}\n", out);
        for (int i = 0; i < node->child_count; i++)
        {
            ASTNode *line = node->children[i];
            if (line->type == NODE_TEXT)
                fputs(line->value, out);
            fputs("\n", out);
        }
        fputs("\\end{verbatim}\n\n", out);
        break;

    case NODE_LIST:
        if (node->ordered)
            fputs("\\begin{enumerate}\n", out);
        else
            fputs("\\begin{itemize}\n", out);

        for (int i = 0; i < node->child_count; i++)
            emit_block(node->children[i], out);

        if (node->ordered)
            fputs("\\end{enumerate}\n\n", out);
        else
            fputs("\\end{itemize}\n\n", out);
        break;

    case NODE_TABLE:
    {
        const char *align = node->value;
        int cols = strlen(align);

        if (cols == 0 && node->child_count > 0)
            cols = node->children[0]->child_count;

        if (cols == 0)
            cols = 1;

        fputs("\\begin{tabular}{", out);
        if (strlen(align) > 0)
            fputs(align, out);
        else
            for (int i = 0; i < cols; i++)
                fputc('l', out);
        fputs("}\n", out);

        if (node->child_count > 0)
            fputs("\\hline\n", out);

        for (int i = 0; i < node->child_count; i++)
        {
            ASTNode *row = node->children[i];
            for (int j = 0; j < row->child_count; j++)
            {
                if (j > 0)
                    fputs(" & ", out);
                ASTNode *cell = row->children[j];
                for (int k = 0; k < cell->child_count; k++)
                    emit_inline(cell->children[k], out);
            }
            fputs(" \\\\\n", out);
            if (i == 0)
                fputs("\\hline\n", out);
        }

        fputs("\\hline\n", out);
        fputs("\\end{tabular}\n\n", out);
    }
    break;

    case NODE_MATH_BLOCK:
        fputs("\\[\n", out);
        for (int i = 0; i < node->child_count; i++)
        {
            if (node->children[i]->type == NODE_TEXT)
                fputs(node->children[i]->value, out);
            if (i < node->child_count - 1)
                fputs("\n", out);
        }
        fputs("\n\\]\n\n", out);
        break;

    default:
        for (int i = 0; i < node->child_count; i++)
            emit_block(node->children[i], out);
        break;
    }
}

void generate_latex(ASTNode *root, FILE *out)
{
    emit_block(root, out);
}

void print_ast(ASTNode *node, int indent)
{
    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type)
    {
    case NODE_DOCUMENT:
        printf("DOCUMENT\n");
        break;
    case NODE_HEADING:
        printf("HEADING (level=%d)\n", node->level);
        break;
    case NODE_PARAGRAPH:
        printf("PARAGRAPH\n");
        break;
    case NODE_TEXT:
        printf("TEXT: %s\n", node->value);
        break;
    case NODE_ITALIC:
        printf("ITALIC\n");
        break;
    case NODE_BOLD:
        printf("BOLD\n");
        break;
    case NODE_BOLD_ITALIC:
        printf("BOLD_ITALIC\n");
        break;
    case NODE_LIST:
        printf("LIST\n");
        break;
    case NODE_TABLE:
        printf("TABLE\n");
        break;
    case NODE_TABLE_ROW:
        printf("TABLE_ROW\n");
        break;
    case NODE_TABLE_CELL:
        printf("TABLE_CELL\n");
        break;
    case NODE_ITEM:
        printf("ITEM\n");
        break;
    case NODE_CODE_BLOCK:
        printf("CODE_BLOCK\n");
        break;
    case NODE_CODE_INLINE:
        printf("CODE_INLINE: %s\n", node->value);
        break;
    case NODE_LINK:
        printf("LINK (%s)\n", node->value);
        break;
    case NODE_IMAGE:
        printf("IMAGE (%s)\n", node->value);
        break;
    case NODE_BLOCKQUOTE:
        printf("BLOCKQUOTE\n");
        break;
    case NODE_MATH_INLINE:
        printf("MATH_INLINE: %s\n", node->value);
        break;
    case NODE_MATH_BLOCK:
        printf("MATH_BLOCK\n");
        break;
    default:
        printf("UNKNOWN\n");
        break;
    }

    for (int i = 0; i < node->child_count; i++)
    {
        print_ast(node->children[i], indent + 1);
    }
}