#include <stdio.h>  /* fopen, fclose, fprintf, printf */
#include <string.h> /* string utilities */
#include <stdlib.h> /* malloc, free */
#include <sys/stat.h>
#include <errno.h>
#include "lexer.h" /* TokenList, lex(), print_tokens() */
#include "parser.h"

static char *read_file(const char *path)
{
    FILE *f;
    long size;
    char *buf;

    f = fopen(path, "rb");
    if (!f)
    {
        printf("Error: cannot open file %s\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    buf = (char *)malloc(size + 1);
    if (!buf)
    {
        printf("Error: malloc failed\n");
        fclose(f);
        exit(1);
    }

    size_t read_bytes = fread(buf, 1, size, f);
    if (read_bytes != size)
    {
        printf("Error: fread failed\n");
        fclose(f);
        free(buf);
        exit(1);
    }

    buf[size] = '\0';

    fclose(f);
    return buf;
}

static void ensure_output_dir(void)
{
    /* This is to check for permissions of the output file, 0 means octal. 7 means owner can read write and execute, 5 means group can read and execute. 5 means others can read and execute*/
    if (mkdir("output", 0755) == -1 && errno != EEXIST)
    {
        perror("Error: cannot create output directory");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    char *file_src = NULL;
    static TokenList tokens;
    Parser parser;
    ASTNode *root;
    FILE *out;

    if (argc < 2) // Makes sure it receives a file process
    {
        printf("Usage: %s <input.md>\n", argv[0]);
        return 1;
    }

    /*
    argv[0] — always the program name itself
    argv[1] — the first argument the user provided (the filename)
    */
    file_src = read_file(argv[1]);

    lex(file_src, &tokens);                    // Will throw error of Error: cannot open file <name> || when file is not available
    printf("Token count: %d\n", tokens.count); // This is just a status check of number of total tokens parsed

    ensure_output_dir(); // Checks for valid permissions to create or access a folder ./output

    out = fopen("output/tokens.txt", "w");
    if (!out)
    {
        printf("Error: cannot open file %s\n", "output/tokens.txt");
        exit(1);
    }

    print_tokens(&tokens, out); // saves the tokens into a temp file by saving it into a txt file. Mainly for debugging
    fclose(out);                // token.txt is done here and thus closed early.

    printf("Done. Token list written to output/tokens.txt (%d tokens)\n", tokens.count);

    printf("Starting parse...\n");
    parser.tokens = &tokens; // Parser is a way to store the TokenList as well as its current position
    parser.pos = 0;

    root = parse_document(&parser); // At this point the ASTNode is initialized as root, with the parser holding the TokenList and a start point

    // print_ast(root, 0);

    out = fopen("output/output.tex", "w");
    if (!out)
    {
        printf("Error: cannot open file %s\n", "output/output.tex");
        exit(1);
    }

    generate_latex(root, out);
    fclose(out);

    printf("Done. LaTeX document written to output/output.tex\n");
    printf("Parsing complete.\n");

    free(file_src);
    return 0;
}
