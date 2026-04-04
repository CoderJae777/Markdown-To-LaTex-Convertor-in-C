#include <stdio.h>
#include <string.h>
#include "lexer.h"

/*
Checks for any MEANINGUL characters in markdown
Returns 1 if any is identified
*/
static int is_special(char c)
{
    return c == '#' ||
           c == '*' ||
           c == '`' ||
           c == '[' ||
           c == ']' ||
           c == '(' ||
           c == ')' ||
           c == '!' ||
           c == '$' ||
           c == '\n' || /*Enter */
           c == ' ' ||
           c == '\t' || /*Tab*/
           c == '>' ||
           c == '-';
}

static void push(TokenList *list, int type, const char *value, int line, int col)
{
    Token *t;

    /*Current MAX_TOKENS is 8192 - approx 50-100 markdown pages already*/
    if (list->count >= MAX_TOKENS)
        return;

    /*set pointer t to point to the address of the next empty slot*/
    t = &list->tokens[list->count];
    /* move count forward */
    list->count = list->count + 1;

    /*this part writes everything*/
    t->type = type;
    t->line = line;
    t->col = col;

    /*copy the actual token and store it in also*/
    /*MAX_TOKEN_VALUE is 256 now */
    strncpy(t->value, value, MAX_TOKEN_VALUE - 1);

    /* Forces the last one to be null terminator*/
    t->value[MAX_TOKEN_VALUE - 1] = '\0';
}

/*
Used to emit the buffer that lex() built
lex() will loop until a meaningful character is found
Until then, it will continue building the buffer
flush_text emits (by using the push() function) and resets this buffer
*/
static void flush_text(TokenList *list, char *buf, int *len, int line, int col)
{
    /*Empty buffer*/
    if (*len == 0)
        return;
    /*Makes sure last element is null terminator*/
    buf[*len] = '\0';
    push(list, TOK_TEXT, buf, line, col);
    *len = 0; /*reset buffer*/
}

/*
    Main lex function
    - takes in a raw text that is read
    - takes in an EMPTY TokenList

*/
void lex(const char *src, TokenList *list)
{
    char text_buf[MAX_TOKEN_VALUE];
    int text_len;
    int line, col;
    int i;
    char c, next, after;

    list->count = 0;
    text_len = 0;
    line = 1;
    col = 1;

    /*
    Reads the md source 1 char at a time
    Starts at position i = 0 and keep going until it hits the null terminator

    if the current char isnt not a markdown symbol, add to text buffer
    if not, then it will brute force check what is it one by one
    */
    for (i = 0; src[i] != '\0'; i++)
    {
        c = src[i];
        next = src[i + 1];
        after = src[i + 2];

        /*
        plain text - accumulate until a special char
        */
        if (!is_special(c))
        {
            // text_buf[text_len++] = c;
            if (text_len < MAX_TOKEN_VALUE - 1)
            {
                text_buf[text_len++] = c;
            }
            col++;
            continue;
        }

        /*
        flush any accumulated text before emitting a special token
        */
        flush_text(list, text_buf, &text_len, line, col);

        /*
        Brute force if-else match
        */
        if (c == '\n')
        {
            push(list, TOK_NEWLINE, "\\n", line, col);
            line++;
            col = 1;
        }
        else if (c == ' ' || c == '\t')
        {
            // treat whitespace as NORMAL text
            if (text_len < MAX_TOKEN_VALUE - 1)
            {
                text_buf[text_len++] = c;
            }
            col++;
        }
        else if (c == '*')
        {
            if (next == '*' && after == '*')
            {
                push(list, TOK_TRIPLE_STAR, "***", line, col);
                i += 2;
                col += 3;
            }
            else if (next == '*')
            {
                push(list, TOK_DOUBLE_STAR, "**", line, col);
                i += 1;
                col += 2;
            }
            else
            {
                push(list, TOK_STAR, "*", line, col);
                col++;
            }
        }
        else if (c == '#')
        {
            /* count consecutive hashes */
            char hashes[8];
            int hlen = 0;
            while (src[i] == '#' && hlen < 7)
            {
                hashes[hlen++] = '#';
                i++;
            }
            hashes[hlen] = '\0';
            i--; /* back up one so loop increment lands correctly */
            push(list, TOK_HASH, hashes, line, col);
            col += hlen;
        }
        else if (c == '`')
        {
            if (next == '`')
            {
                push(list, TOK_DOUBLE_TICK, "``", line, col);
                i += 1;
                col += 2;
            }
            else
            {
                push(list, TOK_BACKTICK, "`", line, col);
                col++;
            }
        }
        else if (c == '$')
        {
            if (next == '$')
            {
                push(list, TOK_DOUBLE_DOL, "$$", line, col);
                i += 1;
                col += 2;
            }
            else
            {
                push(list, TOK_DOLLAR, "$", line, col);
                col++;
            }
        }
        else if (c == '!')
        {
            push(list, TOK_BANG, "!", line, col);
            col++;
        }
        else if (c == '[')
        {
            push(list, TOK_LBRACKET, "[", line, col);
            col++;
        }
        else if (c == ']')
        {
            push(list, TOK_RBRACKET, "]", line, col);
            col++;
        }
        else if (c == '(')
        {
            push(list, TOK_LPAREN, "(", line, col);
            col++;
        }
        else if (c == ')')
        {
            push(list, TOK_RPAREN, ")", line, col);
            col++;
        }
        else if (c == '>')
        {
            push(list, TOK_GT, ">", line, col);
            col++;
        }
        else if (c == '-')
        {
            push(list, TOK_DASH, "-", line, col);
            col++;
        }
    }

    /* flush any trailing text */
    flush_text(list, text_buf, &text_len, line, col);

    push(list, TOK_EOF, "EOF", line, col);
}

const char *token_type_name(int type)
{
    switch (type)
    {
    case TOK_TEXT:
        return "TEXT       ";
    case TOK_HASH:
        return "HASH       ";
    case TOK_STAR:
        return "STAR       ";
    case TOK_DOUBLE_STAR:
        return "DOUBLE_STAR";
    case TOK_TRIPLE_STAR:
        return "TRIPLE_STAR";
    case TOK_BACKTICK:
        return "BACKTICK   ";
    case TOK_DOUBLE_TICK:
        return "DOUBLE_TICK";
    case TOK_LBRACKET:
        return "LBRACKET   ";
    case TOK_RBRACKET:
        return "RBRACKET   ";
    case TOK_LPAREN:
        return "LPAREN     ";
    case TOK_RPAREN:
        return "RPAREN     ";
    case TOK_BANG:
        return "BANG       ";
    case TOK_DOLLAR:
        return "DOLLAR     ";
    case TOK_DOUBLE_DOL:
        return "DOUBLE_DOL ";
    case TOK_NEWLINE:
        return "NEWLINE    ";
    case TOK_GT:
        return "GT         ";
    case TOK_DASH:
        return "DASH       ";
    case TOK_EOF:
        return "EOF        ";
    default:
        return "UNKNOWN    ";
    }
}

/*
    Loops through the TokenList and prints type, line, col, value
*/
void print_tokens(const TokenList *list, FILE *out)
{
    int i;
    fprintf(out, "%-5s  %-12s  %-6s  %-6s  %s\n",
            "INDEX", "TYPE", "LINE", "COL", "VALUE");
    fprintf(out, "%-5s  %-12s  %-6s  %-6s  %s\n",
            "-----", "------------", "------", "------", "-----");

    for (i = 0; i < list->count; i++)
    {
        const Token *t = &list->tokens[i];
        fprintf(out, "%-5d  %s  %-6d  %-6d  [%s]\n",
                i,
                token_type_name(t->type),
                t->line,
                t->col,
                t->value);
    }
}
