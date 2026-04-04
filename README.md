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

## Project Structure

| File                    | Description                                                                              |
| ----------------------- | ---------------------------------------------------------------------------------------- |
| `Makefile`              | Build script - `make` to compile, `make clean` to remove outputs                         |
| `main.c`                | Entry point - reads input file, runs lexer then parser, writes tokens.txt and output.tex |
| `lexer.c` / `lexer.h`   | Tokenizer - scans raw Markdown characters and produces a flat stream of typed tokens     |
| `parser.c` / `parser.h` | Parser - walks the token stream and emits a complete LaTeX document                      |
| `template.c`            | Blank C file template for adding new modules                                             |
| `TestPro.md`            | Extensive Markdown test file covering all supported constructs                           |
| `Test.md`               | Minimal quick test file                                                                  |
| `output/`               | Compiled executables, `tokens.txt` debug dump, and `output.tex` LaTeX result             |
| `Admin/`                | Project proposal, notes, and documentation                                               |

---

## Setting Up

#### Requirements

- GCC (any version supporting ANSI C / C89)
- A terminal (Command Prompt, PowerShell, bash)

#### Install GCC on Windows

Download and install [MinGW-w64](https://www.mingw-w64.org/) or use the GCC bundled with VSCode's C/C++ extension.

Verify your installation:

```bash
gcc --version
```

#### Install `make`

> **Note:** The installation method for `make` differs depending on your operating system (OS).

Anything just ask Chat/Google

---

## Compiling

cd into project directory 
```bash
make
```

Or manually:

```bash
gcc -Wall -Werror main.c lexer.c -o output/main.exe
```

To delete build outputs:

```bash
make clean
```

---

## Running

Build first, then run with your Markdown file:

```bash
make
output/main.exe yourfile.md
```

Run with the provided test files:

```bash
make
output/main.exe Test.md      # minimal test
output/main.exe TestPro.md   # full coverage test
```

Outputs are written to the `output/` directory:

| File                | Contents                                     |
| ------------------- | -------------------------------------------- |
| `output/tokens.txt` | Debug dump of every token the lexer produced |
| `output/output.tex` | The converted LaTeX document                 |

---

## Lexing/Tokenisation

### push() function

```
Every time the lexer recognises something meaningful
it will append one finished token into the TokenList.

E.g input: # Hi

Starting state:
text_buf = []   len=0   line=1   col=1   tokens.count=0

i = 0, # is identified, yes its special, nothing to flush,
push # into TokenList :

tokens[0] = { TOK_HASH, "#", line=1, col=1 }
count=1

```

### flush_text() function

```
Everytime it hits a meaningful character, calls this to emit everything accumulated so far
While scanning "Hello **world**":

read 'H' → buf = ['H'],           len=1
read 'e' → buf = ['H','e'],       len=2
read 'l' → buf = ['H','e','l'],   len=3
read 'l' → buf = [...,'l'],       len=4
read 'o' → buf = [...,'o'],       len=5

hit '*' (special!) → flush_text()
buf[5] = '\0'  →  buf = "Hello"
push TOK_TEXT "Hello"
len = 0        →  buffer reset

now handle '*' as its own token
```

---

## Parser

The parser converts tokens into a structured Abstract Syntax Tree (AST).

### Design

Two-stage parsing:

1. Block-level parsing

Detects structure of each line (what kind of block it is):
- Heading
- Paragraph
- List

```
ASTNode* parse_block(Parser *p)
{
    Token *t = peek(p);

    if(t->type == TOK_HASH)
        return parse_heading(p);

    if(t->type == TOK_DASH)
        return parse_list(p);

    return parse_paragraph(p);
}
```

supporting blocks:
```
ASTNode* parse_heading(Parser *p)
ASTNode* parse_paragraph(Parser *p)
ASTNode* parse_list(Parser *p)
```

2. Inline-level parsing

Handles formatting inside blocks:
- Text
- Bold

```
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
        add_child(parent, text);
        return;
    }

    // fallback
    Token *unknown = advance(p);
    ASTNode *node = create_node(NODE_TEXT);
    strcpy(node->value, unknown->value);
    add_child(parent, node);
}
```

supporting blocks:
```
ASTNode* parse_bold(Parser *p)
ASTNode* parse_text(Parser *p)
```

### AST Design

Each node contains:
- type (e.g. HEADING, TEXT, BOLD)
- value (for text)
- level (for headings)
- children[]

Node Types:
- NODE_DOCUMENT
- NODE_HEADING
- NODE_PARAGRAPH
- NODE_LIST
- NODE_ITEM
- NODE_TEXT
- NODE_BOLD

### Text merging

```
static void add_child(ASTNode *parent, ASTNode *child)
```

Adjacent TEXT nodes are automatically merged to keep the AST clean.
```
"Hello" + " " + "world" → "Hello world"
```

### Space handling

```
static void add_child(ASTNode *parent, ASTNode *child)
```

- Trim leading spaces (first node only)
- Prevent duplicate spaces during merge
- Trim trailing spaces

### Parser safety

```
ASTNode* parse_document(Parser *p)
```

Detects infinite loops and forces token advancement if stuck.

### Example Flow

Input:
```
# Document Title

## Headings

# Heading Level 1
## Heading Level 2
### Heading Level 3

## Paragraphs

This is a simple paragraph.

This is another paragraph separated by a blank line.

## Unordered Lists

- Item one
- Item two
- Item three

- Item with **bold** text
- Another **bold** example

## Mixed

# Heading

This is a paragraph with **bold text**.

- List item one
- List item two
```
AST:
```
DOCUMENT
  HEADING (level=1)
    TEXT: Document Title
  PARAGRAPH
    TEXT:
  HEADING (level=2)
    TEXT: Headings
  PARAGRAPH
    TEXT:
  HEADING (level=1)
    TEXT: Heading Level 1
  HEADING (level=2)
    TEXT: Heading Level 2
  HEADING (level=3)
    TEXT: Heading Level 3
  PARAGRAPH
    TEXT:
  HEADING (level=2)
    TEXT: Paragraphs
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT: This is a simple paragraph.
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT: This is another paragraph separated by a blank line.
  PARAGRAPH
    TEXT:
  HEADING (level=2)
    TEXT: Unordered Lists
  PARAGRAPH
    TEXT:
  LIST
    ITEM
      TEXT: Item one
    ITEM
      TEXT: Item two
    ITEM
      TEXT: Item three
  PARAGRAPH
    TEXT:
  LIST
    ITEM
      TEXT: Item with
      BOLD
        TEXT: bold
      TEXT:  text
    ITEM
      TEXT: Another
      BOLD
        TEXT: bold
      TEXT:  example
  PARAGRAPH
    TEXT:
  HEADING (level=2)
    TEXT: Mixed
  PARAGRAPH
    TEXT:
  HEADING (level=1)
    TEXT: Heading
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT: This is a paragraph with
    BOLD
      TEXT: bold text
    TEXT: .
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT:
  LIST
    ITEM
      TEXT: List item one
  PARAGRAPH
    TEXT:
  LIST
    ITEM
      TEXT: List item one
  PARAGRAPH
    TEXT:
  LIST
    ITEM
  PARAGRAPH
    TEXT:
  LIST
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT:
  PARAGRAPH
    TEXT:
  LIST
  PARAGRAPH
    TEXT:
  PARAGRAPH
  PARAGRAPH
    TEXT:
  LIST
    ITEM
      TEXT: List item one
    ITEM
      TEXT: List item two
```

---

## main.c Improvements 

### Safer File Reading

- Uses binary mode ("rb") to ensure consistent cross-platform behavior
- Validates file opening (fopen) to prevent crashes
- Checks memory allocation (malloc) before use
- Verifies full file read (fread) to avoid partial or corrupted input
- Allocates precise buffer size (size + 1) instead of arbitrary padding

```
f = fopen(path, "rb");
if (!f) {
    printf("Error: cannot open file %s\n", path);
    exit(1);
}
```

### Command-line Argument Validation

Input arguments are validated before execution, preventing segmentation faults when no file is provided.

```
if (argc < 2) {
    printf("Usage: %s <input.md>\n", argv[0]);
    return 1;
}
```

### Safe Output File Handling

Output file handling now avoids undefined behavior:
- Checks if fopen succeeds before writing
- Prevents writing to a NULL file pointer
- Ensures graceful failure with error messages

```
out = fopen("output/tokens.txt", "w");
if (!out) {
    printf("Error: cannot open file %s\n", "output/tokens.txt");
    exit(1);
}
```

---