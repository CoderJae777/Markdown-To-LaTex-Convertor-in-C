#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

static char *read_file(const char *path)
{
    FILE   *f;
    long    size;
    char   *buf;

    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    buf = (char *)malloc(size + 3);  /* +3 for \0 and 2-char lookahead safety */
    if (!buf) {
        fprintf(stderr, "Error: out of memory\n");
        fclose(f);
        return NULL;
    }

    fread(buf, 1, size, f);
    buf[size]     = '\0';
    buf[size + 1] = '\0';  /* lookahead safety: src[i+1] */
    buf[size + 2] = '\0';  /* lookahead safety: src[i+2] */
    fclose(f);
    return buf;
}

int main(int argc, char *argv[])
{
    char      *file_src = NULL;
    static TokenList tokens; 
    FILE      *out;

    if (argc < 2) {
        fprintf(stderr, "Usage: output/main.exe input.md\n");
        fprintf(stderr, "Error: no input file provided.\n");
        return 1;
    }

    file_src = read_file(argv[1]);
    if (!file_src) return 1;

    lex(file_src, &tokens);

    out = fopen("output/tokens.txt", "w");
    print_tokens(&tokens, out);
    fclose(out);

    printf("Done. Token list written to output/tokens.txt (%d tokens)\n", tokens.count);

    free(file_src);
    return 0;
}
