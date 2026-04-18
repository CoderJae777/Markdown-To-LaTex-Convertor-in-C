# 50.051 Programming Language Concepts - Parser Project

A command-line compiler written in ANSI C that converts Markdown (`.md`) files into LaTeX (`.tex`) files.

**Built for the SUTD module 50.051 Programming Language Concepts.**

---

### Done by:

| Name                | Student ID |
| ------------------- | ---------- |
| Goh Yi Shen         | 1006852    |
| Joshua Chua Tze Ern | 1006627    |
| Chew Ming Hui       | 1006892    |
| Anika Ajay Handigol | 1006952    |
| Jun Jin             | 1004435    |

---

## Start: Build, Run, and Test

From the project root, compile the converter with:

```bash
make
```
Or manually:

```bash
gcc -Wall main.c lexer.c parser.c parser_inline.c parser_latex.c -o output/main.exe
```

This creates the executable:

```text
output/main.exe
```


Run the converter by passing a Markdown file as the first argument:

```bash
./output/main.exe TestPro.md
```

After each run, the program writes:

| Output file | Description |
| ----------- | ----------- |
| `output/tokens.txt` | Debug dump of the tokens produced by the lexer |
| `output/output.tex` | Generated LaTeX document |

To test the current implementation with the provided sample inputs, run:

```bash
make clean
make

for f in TestPro.md Test_parser_step1.md sample_inputs/*.md; do
    echo "Testing $f"
    ./output/main.exe "$f" || exit 1
done
```

---

## Project Structure

| File | Description |
| ---- | ----------- |
| `Makefile` | Build script - `make` to compile, `make clean` to remove outputs |
| `main.c` | Entry point - reads input file, runs lexer and parser, writes `tokens.txt` and `output.tex` |
| `lexer.c` / `lexer.h` | Tokenizer - scans raw Markdown characters and produces a flat stream of typed tokens |
| `parser.h` | Public parser interface - defines AST node types, parser structs, and public parser functions |
| `parser_internal.h` | Internal parser interface - shares helper functions between parser modules |
| `parser.c` | Core parser - handles parser utilities, AST helpers, and block-level parsing |
| `parser_inline.c` | Inline parser - handles text, emphasis, links, images, inline code, and inline math |
| `parser_latex.c` | LaTeX emitter - walks the AST and writes the final LaTeX document |
| `template.c` | Blank C file template for adding new modules |
| `TestPro.md` | Extensive Markdown test file covering supported constructs |
| `Test.md` | Minimal quick test file |
| `Test_parser_step1.md` | Parser development test file |
| `sample_inputs/` | Additional Markdown test inputs |
| `sample_outputs/` | Generated LaTeX samples for test inputs |
| `output/` | Compiled executables, `tokens.txt` debug dump, and `output.tex` LaTeX result |
| `Admin/` | Project proposal, notes, and documentation |

---

## Architecture Overview

The compiler follows a **three-stage pipeline**:

```
Raw Markdown File
       ↓
   ┌─────────────────┐
   │    LEXER        │ – Converts characters to tokens
   └─────────────────┘
       ↓
  TokenList (tokens.txt)
       ↓
   ┌─────────────────┐
   │    PARSER       │ – Builds Abstract Syntax Tree (AST)
   └─────────────────┘
       ↓
  ASTNode Tree (in-memory)
       ↓
   ┌─────────────────┐
   │  LaTeX EMITTER  │ – Generates output.tex from AST
   └─────────────────┘
       ↓
  output/output.tex
```

Each stage is independent:
- **Lexer** doesn't know about syntax, just recognizes special characters
- **Parser** doesn't know about LaTeX, just builds a tree from tokens
- **LaTeX Emitter** walks the tree and outputs LaTeX commands

---

Each stage is independent:
- **Lexer** doesn't know about syntax, just recognizes special characters
- **Parser** doesn't know about LaTeX, just builds a tree from tokens
- **LaTeX Emitter** walks the tree and outputs LaTeX commands