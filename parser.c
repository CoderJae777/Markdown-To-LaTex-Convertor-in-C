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

static Token* peek(Parser *p)
{
    return &p->tokens->tokens[p->pos];
}

static Token* advance(Parser *p)
{
    if(p->pos < p->tokens->count)
        p->pos++;
    return &p->tokens->tokens[p->pos - 1];
}

static int match(Parser *p, int type)
{
    if(peek(p)->type == type)
    {
        advance(p);
        return 1;
    }
    return 0;
}

static ASTNode* create_node(NodeType type)
{
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->level = 0;
    node->value[0] = '\0';
    node->child_count = 0;
    return node;
}

static void add_child(ASTNode *parent, ASTNode *child)
{
    if(!child) return;

    // merge adjacent TEXT nodes
    if(parent->child_count > 0)
    {
        ASTNode *last = parent->children[parent->child_count - 1];

        if(last->type == NODE_TEXT && child->type == NODE_TEXT)
        {
            int last_len = strlen(last->value);

            // prevent double spaces
            if(last_len > 0 &&
                last->value[last_len - 1] == ' ' &&
                child->value[0] == ' ')
            {
                memmove(child->value, child->value + 1, strlen(child->value));
            }

            int remaining = 255 - strlen(last->value) - 1;
            if(remaining > 0)
            {
                strncat(last->value, child->value, remaining);
            }

            free(child);
            return;
        }
    }

    if(parent->child_count < 64)
        parent->children[parent->child_count++] = child;
}

// entry point
ASTNode* parse_document(Parser *p)
{
    ASTNode *doc = create_node(NODE_DOCUMENT);

    while(peek(p)->type != TOK_EOF)
    {
        int start_pos = p->pos;

        if(match(p, TOK_NEWLINE))
            continue;

        ASTNode *node = parse_block(p);
        if(node)
            add_child(doc, node);

        if(p->pos == start_pos)
        {
            printf("Parser stuck at token %d, forcing advance\n", p->pos);
            advance(p);
        }
    }

    return doc;
}

ASTNode* parse_block(Parser *p)
{
    Token *t = peek(p);

    if(t->type == TOK_HASH)
        return parse_heading(p);

    if(t->type == TOK_DASH)
        return parse_list(p);

    return parse_paragraph(p);
}

ASTNode* parse_heading(Parser *p)
{
    Token *hash = advance(p);
    ASTNode *node = create_node(NODE_HEADING);
    node->level = strlen(hash->value);

    // parse inline content
    parse_inline_until_newline(p, node);
    match(p, TOK_NEWLINE);

    return node;
}

ASTNode* parse_paragraph(Parser *p)
{
    ASTNode *node = create_node(NODE_PARAGRAPH);
    int first = 1;

    while(peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF)
    {
        int before = node->child_count;

        parse_inline(p, node);

        // trim leading space on first TEXT node
        if(first && node->child_count > before)
        {
            ASTNode *child = node->children[before];

            if(child->type == NODE_TEXT)
            {
                while(child->value[0] == ' ')
                {
                    memmove(child->value,
                            child->value + 1,
                            strlen(child->value));
                }
            }

            first = 0;
        }
    }

    match(p, TOK_NEWLINE);

    int has_content = 0;

    for(int i=0;i<node->child_count;i++)
    {
        ASTNode *c = node->children[i];

        if(c->type != NODE_TEXT)
        {
            has_content = 1;
            break;
        }

        // check if TEXT has any non-space character
        for(int j=0;c->value[j];j++)
        {
            if(c->value[j] != ' ' && c->value[j] != '\t')
            {
                has_content = 1;
                break;
            }
        }

        if(has_content)
            break;
    }

    if(!has_content)
    {
        free(node);
        return NULL;
    }

    return node;
}

ASTNode* parse_list(Parser *p)
{
    ASTNode *list = create_node(NODE_LIST);

    while(peek(p)->type == TOK_DASH)
    {
        advance(p); // consume '-'

        ASTNode *item = create_node(NODE_ITEM);

        parse_inline_until_newline(p, item);
        add_child(list, item);
        match(p, TOK_NEWLINE);
    }

    return list;
}

void parse_inline_until_newline(Parser *p, ASTNode *parent)
{
    int first = 1;

    while(peek(p)->type != TOK_NEWLINE &&
           peek(p)->type != TOK_EOF)
    {
        int before = parent->child_count;

        parse_inline(p, parent);

        // trim leading space on first TEXT node
        if(first && parent->child_count > before)
        {
            ASTNode *child = parent->children[before];

            if(child->type == NODE_TEXT)
            {
                while(child->value[0] == ' ')
                {
                    memmove(child->value, child->value + 1, strlen(child->value));
                }
            }

            first = 0;
        }
    }
}

void parse_inline(Parser *p, ASTNode *parent)
{
    Token *t = peek(p);

    if(t->type == TOK_DOUBLE_STAR)
    {
        ASTNode *bold = parse_bold(p);
        add_child(parent, bold);
        return;
    }

    if(t->type == TOK_TEXT)
    {
        ASTNode *text = parse_text(p);
        if(text)
            add_child(parent, text);
        return;
    }

    // fallback - consume unknown tokens as TEXT
    Token *unknown = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, unknown->value);
    add_child(parent, node);
}

ASTNode* parse_text(Parser *p)
{
    Token *t = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, t->value);

    // trim trailing spaces
    int len = strlen(node->value);
    while(len > 0 && (node->value[len-1] == ' ' || node->value[len-1] == '\t'))
    {
        node->value[len-1] = '\0';
        len--;
    }

    return node;
}

ASTNode* parse_bold(Parser *p)
{
    match(p, TOK_DOUBLE_STAR);

    ASTNode *node = create_node(NODE_BOLD);

    while(peek(p)->type != TOK_DOUBLE_STAR &&
           peek(p)->type != TOK_EOF)
    {
        parse_inline(p, node);
    }

    match(p, TOK_DOUBLE_STAR);

    return node;
}

void print_ast(ASTNode *node, int indent)
{
    for(int i=0;i<indent;i++)
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
        case NODE_BOLD:
            printf("BOLD\n");
            break;
        case NODE_LIST:
            printf("LIST\n");
            break;
        case NODE_ITEM:
            printf("ITEM\n");
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }

    for(int i=0;i<node->child_count;i++)
    {
        print_ast(node->children[i], indent + 1);
    }
}