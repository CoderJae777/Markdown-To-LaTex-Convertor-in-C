#include <stdio.h>
#include <string.h>
#include "parser.h"

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
        // fputs("\\noindent ", out);
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

    case NODE_HORIZONTAL_RULE:
        fputs("\\noindent\\hrulefill\n\n", out);
        break;

    case NODE_ITEM:
        fputs("\\item ", out);
        for (int i = 0; i < node->child_count; i++)
        {
            ASTNode *child = node->children[i];
            switch (child->type)
            {
            case NODE_TEXT:
            case NODE_BOLD:
            case NODE_ITALIC:
            case NODE_BOLD_ITALIC:
            case NODE_CODE_INLINE:
            case NODE_LINK:
            case NODE_IMAGE:
            case NODE_MATH_INLINE:
                emit_inline(child, out);
                break;
            default:
                fputs("\n", out);
                emit_block(child, out);
                break;
            }
        }
        fputs("\n", out);
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
    case NODE_HORIZONTAL_RULE:
        printf("HORIZONTAL_RULE\n");
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
