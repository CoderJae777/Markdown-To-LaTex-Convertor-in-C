#include <stdio.h>
#include <string.h> //strings
#include "parser.h"
#include <stdlib.h> //malloc
#include <math.h>
#include <time.h>
#include "lexer.h"

// Cannot use for project
#include <ctype.h> //isalpha(), isdigit(), toupper() etc

/*
If you just have 1 file, run using the play button
If you using max.c max.h, use this:S
gcc nameoffile.c -o output/nameoffile.exe
output nameoffile.exe

*/

/* ========== STACK HELPERS ========== */
static void push_context(ContextStack *stack, int type,
                         const char *opening_value, int line, int col)
{
    if (stack->depth >= 32)
        return;

    ContextFrame *frame = &stack->frames[stack->depth];

    frame->type = type;
    frame->line = line;
    frame->col = col;
    frame->content_len = 0;
    frame->content[0] = '\0';

    strncpy(frame->opening_value, opening_value, 255);
    frame->opening_value[255] = '\0';

    stack->depth++;
}

static ContextFrame *pop_context(ContextStack *stack)
{
    if (stack->depth <= 0)
        return NULL;
    stack->depth--;
    return &stack->frames[stack->depth];
}

static ContextFrame *current_context(ContextStack *stack)
{
    if (stack->depth <= 0)
        return NULL;
    return &stack->frames[stack->depth - 1];
}

static void append_to_context(ContextStack *stack, const char *text)
{
    ContextFrame *frame = current_context(stack);
    if (!frame)
        return;

    size_t available = sizeof(frame->content) - frame->content_len - 1;
    size_t to_copy = strlen(text);
    if (to_copy > available)
        to_copy = available;

    strncat(frame->content, text, to_copy);
    frame->content_len += to_copy;
}

/* ========== LATEX OUTPUT HELPERS ========== */
static void write_preamble(FILE *out)
{
    fprintf(out, "\\documentclass{article}\n");
    fprintf(out, "\\usepackage{amsmath}\n");
    fprintf(out, "\\usepackage{amssymb}\n");
    fprintf(out, "\\usepackage{graphicx}\n");
    fprintf(out, "\\usepackage{hyperref}\n");
    fprintf(out, "\\usepackage{listings}\n");
    fprintf(out, "\\usepackage{xcolor}\n");
    fprintf(out, "\\begin{document}\n\n");
}

static void write_epilogue(FILE *out)
{
    fprintf(out, "\n\\end{document}\n");
}

/* static void emit_context(ContextStack *stack, FILE *out)
{
    if (stack->depth <= 0)
        return;

    ContextFrame *frame = &stack->frames[stack->depth - 1];

    switch (frame->type)
    {
    case CTX_BOLD:
        fprintf(out, "\\textbf{%s}", frame->content);
        break;
    case CTX_ITALIC:
        fprintf(out, "\\textit{%s}", frame->content);
        break;
    case CTX_CODE:
        fprintf(out, "\\texttt{%s}", frame->content);
        break;
    case CTX_MATH:
        fprintf(out, "$%s$", frame->content);
        break;
    case CTX_BLOCK_MATH:
        fprintf(out, "$$%s$$", frame->content);
        break;
    default:
        fprintf(out, "%s", frame->content);
    }
}
 */
/* ========== MAIN PARSER ========== */
void parse_and_emit(TokenList *tokens, FILE *fLatex)
{
    ContextStack stack;
    // ContextFrame *currentFrame;
    int i;
    int tokenLength;
    int currentContextType;

    /*Testing*/
    stack.depth = 0;
    currentContextType = 0;

    /* Write LaTeX preamble */
    write_preamble(fLatex);

    /* Loop through all tokens */
    tokenLength = tokens->count;
    for (i = 0; i < tokenLength; i++)
    {
        Token tok = tokens->tokens[i];
        /* printf("TYPE: %i | value: %s | line : %i | col: %i\n",
               tok.type,
               tok.value,
               tok.line,
               tok.col);
 */
        /* Check if there is any type already */
        /* type 0 is TEXT, 14 is NEWLINE */

        /* This part is to check for special char */
        /*  check for stack depth
        retrieve top stack and closing types
        check for them otherwise pass */
        switch (currentContextType)
        {
        case (0):
            break;
        case (CTX_HEADING):
            if (tok.type == TOK_NEWLINE)
            {
                currentContextType = end_heading(&stack, fLatex);
                continue;
            }

            break;
        }

        switch (tok.type)
        {
        case (TOK_HASH): /* TOK_HASH */
            /* I will need to push to stack later */
            currentContextType = set_heading(&stack, tok);
            continue;
            break;
        case (TOK_TEXT):
            set_text(&stack, tok.value, fLatex);
            continue;
            break;
        }
    }

    /* Write LaTeX epilogue */
    write_epilogue(fLatex);
}

void set_text(ContextStack *stack, char *value, FILE *fLatex)
{
    if (stack->depth == 0)
    {
        printf("PRINTED");
        fputs(value, fLatex);
    }
    else
    {
        append_to_context(stack, value);
    }
}

int set_heading(ContextStack *stack, Token tok)
{
    /* I assume here that the hash token is well defined already */
    switch (strlen(tok.value))
    {
    case 1:
        push_context(stack, CTX_HEADING, "\\section{", tok.line, tok.col);
        break;
    case 2:
        push_context(stack, CTX_HEADING, "\\subsection{", tok.line, tok.col);
        break;
    case 3:
        push_context(stack, CTX_HEADING, "\\subsubsection{", tok.line, tok.col);
        break;
    case 4:
        push_context(stack, CTX_HEADING, "\\paragraph{", tok.line, tok.col);
        break;
    case 5:
        push_context(stack, CTX_HEADING, "\\subparagraph{", tok.line, tok.col);
        break;
    case 6:
        push_context(stack, CTX_HEADING, "\\textbf{", tok.line, tok.col);
        break;
    }
    return CTX_HEADING;
}
/*
int end_heading(ContextStack *stack, char *value, FILE *fLatex)
{
    char buffer[4096];
    strcat(stack->frames[stack->depth - 1]->content, value);
    strcpy(buffer, stack->frames[stack->depth - 1]->content);
    pop_context(stack);
    set_text(stack, buffer, fLatex);
    return stack->frames[stack->depth - 1]->type;
} */
int end_heading(ContextStack *stack, FILE *fLatex)
{
    ContextFrame *frame = current_context(stack);
    if (!frame)
        return 0;

    // Print LaTeX heading
    fprintf(fLatex, "%s%s}\n", frame->opening_value, frame->content);

    // Pop the heading
    pop_context(stack);

    ContextFrame *newTop = current_context(stack);
    return newTop ? newTop->type : 0;
}