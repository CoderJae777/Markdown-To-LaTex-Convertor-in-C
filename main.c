#include <stdio.h>  /* fopen, fclose, fprintf, printf */
#include <string.h> /* string utilities */
#include <stdlib.h> /* malloc, free */
#include "lexer.h"  /* TokenList, lex(), print_tokens() */

static char *read_file(const char *path)
{
    FILE *f;
    long size;
    char *buf;

    f = fopen(path, "r");

    /*
    Find the size of the file using fseek and ftell
    */
    fseek(f, 0, SEEK_END);
    size = ftell(f);

    /*
    Then send the cursor back to the top
    */
    rewind(f);

    /*
    Just incase some idiot make the entire markdown ONE SINGLE STRING
    +3 because why not just in case
    */
    buf = (char *)malloc(size + 3);

    /*
    Copy file into buf
    */
    fread(buf, 1, size, f);
    /*
    Forces last one to be null terminator
    */
    buf[size] = '\0';

    fclose(f);
    return buf;
}

int main(int argc, char *argv[])
{
    char *file_src = NULL;
    static TokenList tokens;
    FILE *out;

    /*
    argv[0] — always the program name itself
    argv[1] — the first argument the user provided (the filename)
    */
    file_src = read_file(argv[1]);

    lex(file_src, &tokens);

    out = fopen("output/tokens.txt", "w");
    print_tokens(&tokens, out);
    fclose(out);

    printf("Done. Token list written to output/tokens.txt (%d tokens)\n", tokens.count);

    free(file_src);
    return 0;
}
