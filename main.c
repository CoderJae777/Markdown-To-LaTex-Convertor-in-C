#include <stdio.h>  /* fopen, fclose, fprintf, printf */
#include <string.h> /* string utilities */
#include <stdlib.h> /* malloc, free */
#include "lexer.h"  /* TokenList, lex(), print_tokens() */
#include "parser.h"

static char *read_file(const char *path)
{
    FILE *f;
    long size;
    char *buf;

    f = fopen(path, "rb");
    if(!f) {
        printf("Error: cannot open file %s\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    buf = (char *)malloc(size + 1);
    if(!buf) {
        printf("Error: malloc failed\n");
        fclose(f);
        exit(1);
    }

    size_t read_bytes = fread(buf, 1, size, f);
    if(read_bytes != size) {
        printf("Error: fread failed\n");
        fclose(f);
        free(buf);
        exit(1);
    }

    buf[size] = '\0';

    fclose(f);
    return buf;
}

int main(int argc, char *argv[])
{
    char *file_src = NULL;
    static TokenList tokens;
    Parser parser;
    ASTNode *root;
    FILE *out;

    if(argc < 2) {
        printf("Usage: %s <input.md>\n", argv[0]);
        return 1;
    }

    /*
    argv[0] — always the program name itself
    argv[1] — the first argument the user provided (the filename)
    */
    file_src = read_file(argv[1]);

    lex(file_src, &tokens);
    printf("Token count: %d\n", tokens.count);

    out = fopen("output/tokens.txt", "w");
    if(!out) {
        printf("Error: cannot open file %s\n", "output/tokens.txt");
        exit(1);
    }

    print_tokens(&tokens, out);
    fclose(out);

    printf("Done. Token list written to output/tokens.txt (%d tokens)\n", tokens.count);

    printf("Starting parse...\n");
    parser.tokens = &tokens;
    parser.pos = 0;

    root = parse_document(&parser);

    print_ast(root, 0);

    printf("Parsing complete.\n");

    free(file_src);
    return 0;
}
