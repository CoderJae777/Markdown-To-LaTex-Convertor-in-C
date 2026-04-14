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

## Main Entry Point (`main.c`)

The `main.c` file orchestrates the entire compilation process:

### Step 1: Read Input File
```c
file_src = read_file(argv[1]);
```
- Opens file in binary mode (`"rb"`)
- Validates file exists (prevents crashes)
- Allocates exact buffer size needed
- Validates full file is read

**Safety checks:**
- `fopen` validation (file must exist and be readable)
- `malloc` validation (prevent null pointer dereference)
- `fread` validation (full file was read, not partial)

### Step 2: Lexical Analysis
```c
lex(file_src, &tokens);
```
- Calls the lexer to tokenize the input
- Stores tokens in `TokenList` structure
- Prints token count for debugging

### Step 3: Output Directory Setup
```c
ensure_output_dir();
```
- Creates `output/` directory if it doesn't exist
- Uses Unix permissions `0755` (rwxr-xr-x)

### Step 4: Debug Output
```c
print_tokens(&tokens, out);
```
- Writes all tokens to `output/tokens.txt`
- Useful for debugging the lexer stage

### Step 5: Parsing
```c
root = parse_document(&parser);
```
- Initializes parser with token list
- Builds complete AST from tokens
- Returns root `NODE_DOCUMENT` node

### Step 6: LaTeX Generation
```c
generate_latex(root, out);
```
- Walks the AST recursively
- Outputs LaTeX commands to `output/output.tex`

---

## Lexing/Tokenisation

The lexer (`lexer.c` / `lexer.h`) converts a stream of raw characters into a **flat list of typed tokens**.

### Token Types

Defined in `lexer.h`, the lexer recognizes these special characters:

| Token Type | Symbol | Use |
|---|---|---|
| `TOK_TEXT` | (any) | Plain text content |
| `TOK_HASH` | `#` | Heading marker |
| `TOK_DASH` | `-` | Unordered list marker |
| `TOK_STAR` | `*` | Single asterisk |
| `TOK_DOUBLE_STAR` | `**` | Bold formatting |
| `TOK_TRIPLE_STAR` | `***` | Bold + italic |
| `TOK_UNDERSCORE` | `_` | Single underscore |
| `TOK_DOUBLE_UNDERSCORE` | `__` | Italic via underscore |
| `TOK_BACKTICK` | `` ` `` | Inline code marker |
| `TOK_TRIPLE_BACKTICK` | ``` `` ` `` ``` | Code block marker |
| `TOK_LBRACKET` / `TOK_RBRACKET` | `[ ]` | Link/image text |
| `TOK_LPAREN` / `TOK_RPAREN` | `( )` | Link/image URL |
| `TOK_BANG` | `!` | Image prefix |
| `TOK_DOLLAR` / `TOK_DOUBLE_DOL` | `$` / `$$` | Math mode |
| `TOK_GT` | `>` | Blockquote marker |
| `TOK_PIPE` | `\|` | Table separator |
| `TOK_NEWLINE` | `\n` | Line break |
| `TOK_EOF` | (end) | End of file |

### Token Structure

Each token stores:
```c
typedef struct {
    int type;           // Which type of token (TOK_*)
    char value[256];    // The actual text (e.g., "# ", "bold text")
    int line;           // Source line number (for error reporting)
    int col;            // Source column number
} Token;
```

### Lexer Functions

#### `is_special(char c)`
Checks if a character is meaningful (part of Markdown syntax):
```c
return c == '#' || c == '*' || c == '_' || c == '`' || ...
```
- Returns 1 if special
- Returns 0 if plain text

#### `push(TokenList *list, int type, const char *value, int line, int col)`
Adds a completed token to the token list:
- Validates token count doesn't exceed `MAX_TOKENS` (8192)
- Writes to next empty slot in array
- Increments count
- Safely copies value with `strncpy` (prevents buffer overflow)
- Forces null-terminator at end

Example:
```
push(&tokens, TOK_HASH, "#", 1, 1);  // Add # token at line 1, col 1
```

#### `flush_text(TokenList *list, char *buf, int *len, int line, int col)`
Emits accumulated plain text as a `TOK_TEXT` token:
- Called when lexer hits a special character
- Accumulates plain text characters in buffer
- When buffer is non-empty, pushes it as one `TOK_TEXT` token
- Resets buffer for next text segment

Example with input "Hello **world**":
```
Read 'H','e','l','l','o' → buf="Hello", len=5
Hit '*' (special) → flush_text() 
  → push(TOK_TEXT, "Hello")
  → buf reset, len=0
Hit '*' again → flush_text() (buffer empty, skip)
Process '**' → push(TOK_DOUBLE_STAR, "**")
```

#### `lex(const char *src, TokenList *list)`
Main lexer loop:
- Scans source file character-by-character
- Tracks line/column for error reporting
- For each character:
  - If plain text: add to buffer
  - If special: flush buffer, then emit special token
- Handles multi-character operators (e.g., `**`, `***`)

State during lexing:
```c
char text_buf[MAX_TOKEN_VALUE];  // Accumulates text
int text_len;                    // Current length
int line, col;                   // Position tracking
```

Example walk-through for `# Hello`:
```
i=0, c='#' → is_special=1 → flush_text() (empty)
          → push(TOK_HASH, "#", 1, 1)
i=1, c=' ' → is_special=1 → flush_text() (empty)
          → add to text_buf (whitespace is normal text)
i=2-6, c='Hello' → is_special=0 → add to text_buf
          → text_buf="Hello", text_len=5
i=7, c='\n' → is_special=1 → flush_text()
          → push(TOK_TEXT, "Hello", 1, 3)
          → line++, col=1
```

### TokenList Structure

```c
typedef struct {
    Token tokens[MAX_TOKENS];  // Array of up to 8192 tokens
    int count;                 // Actual number of tokens
} TokenList;
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

## LaTeX Generation

The code generation stage walks the AST and emits LaTeX commands. The `generate_latex()` function implements a **visitor pattern**:

```c
void generate_latex(ASTNode *root, FILE *out)
{
    // For each node type, emit appropriate LaTeX
    switch(node->type) {
        case NODE_DOCUMENT:
            // Emit \documentclass, \begin{document}, etc.
            break;
        case NODE_HEADING:
            // Emit \section, \subsection, etc. based on level
            break;
        case NODE_PARAGRAPH:
            // Emit paragraph content + newline
            break;
        case NODE_BOLD:
            // Emit \textbf{...}
            break;
        // ... etc for all node types
    }
}
```

**Example:**

AST:
```
NODE_HEADING (level=1)
└── NODE_TEXT "Introduction"
```

Generated LaTeX:
```latex
\section{Introduction}
```

This stage doesn't understand Markdown — it only knows LaTeX syntax. If you wanted to emit HTML instead, you'd write a different `generate_html()` function that walks the same AST.

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

## Complete Data Flow Example

Here's a complete walk-through of the entire compilation pipeline:

### Input File: `example.md`
```markdown
# My Document

This is **bold** and regular text.

- Item 1
- Item 2
```

### Stage 1: Lexer Output (`output/tokens.txt`)
```
1: TOK_HASH         "#"
2: TOK_TEXT         "My Document"
3: TOK_NEWLINE      "\n"
4: TOK_NEWLINE      "\n"
5: TOK_TEXT         "This is "
6: TOK_DOUBLE_STAR  "**"
7: TOK_TEXT         "bold"
8: TOK_DOUBLE_STAR  "**"
9: TOK_TEXT         " and regular text."
10: TOK_NEWLINE     "\n"
11: TOK_NEWLINE     "\n"
12: TOK_DASH        "-"
13: TOK_TEXT        "Item 1"
14: TOK_NEWLINE     "\n"
15: TOK_DASH        "-"
16: TOK_TEXT        "Item 2"
17: TOK_NEWLINE     "\n"
18: TOK_EOF         ""
```

### Stage 2: Parser Output (In-memory AST)

```
NODE_DOCUMENT
├── NODE_HEADING (level=1)
│   └── NODE_TEXT "My Document"
├── NODE_PARAGRAPH
│   ├── NODE_TEXT "This is "
│   ├── NODE_BOLD
│   │   └── NODE_TEXT "bold"
│   └── NODE_TEXT " and regular text."
└── NODE_LIST (ordered=0)
    ├── NODE_ITEM
    │   └── NODE_TEXT "Item 1"
    └── NODE_ITEM
        └── NODE_TEXT "Item 2"
```

### Stage 3: LaTeX Generator Output (`output/output.tex`)
```latex
\documentclass{article}
\usepackage{amssymb}
\begin{document}

\section*{My Document}

This is \textbf{bold} and regular text.

\begin{itemize}
\item Item 1
\item Item 2
\end{itemize}

\end{document}
```

---

## How to Extend the Compiler

### Adding a New Markdown Feature

To add support for a new Markdown construct (e.g., strikethrough `~~text~~`):

1. **Lexer**: Add token type in `lexer.h`
   ```c
   #define TOK_TILDE 23
   #define TOK_DOUBLE_TILDE 24
   ```

2. **Lexer**: Detect in `lexer.c` `lex()` function
   ```c
   else if (c == '~' && next == '~') {
       push(list, TOK_DOUBLE_TILDE, "~~", line, col);
       i += 1;
       col += 2;
   }
   ```

3. **Parser**: Add node type in `parser.h`
   ```c
   typedef enum {
       // ... existing types ...
       NODE_STRIKETHROUGH
   } NodeType;
   ```

4. **Parser**: Add parsing function in `parser.c`
   ```c
   ASTNode *parse_strikethrough(Parser *p)
   {
       advance(p);  // consume ~~
       ASTNode *node = create_node(NODE_STRIKETHROUGH);
       // Parse content until closing ~~
       while (peek(p)->type != TOK_DOUBLE_TILDE) {
           parse_inline(p, node);
       }
       advance(p);  // consume closing ~~
       return node;
   }
   ```

5. **Generator**: Add LaTeX output in generator function
   ```c
   case NODE_STRIKETHROUGH:
       fprintf(out, "\\sout{");
       for (int i = 0; i < node->child_count; i++)
           generate_latex(node->children[i], out);
       fprintf(out, "}");
       break;
   ```

### Debugging

Use the token dump to verify lexer behavior:
```bash
make
output/main.exe TestFile.md
cat output/tokens.txt  # See all tokens
```

Enable AST printing (in `parser.c`):
```c
print_ast(root, 0);  // Prints tree structure before LaTeX generation
```

---

## Code Organization Summary

| File | Purpose | Key Functions |
|---|---|---|
| `main.c` | Entry point and orchestration | `main()`, `read_file()`, `ensure_output_dir()` |
| `lexer.h` / `lexer.c` | Character → Token conversion | `lex()`, `push()`, `flush_text()`, `is_special()` |
| `parser.h` / `parser.c` | Token → AST conversion | `parse_document()`, `parse_block()`, `parse_inline()`, `add_child()` |
| `Makefile` | Build automation | `make`, `make clean` |

**Data types:**
- `Token`: Single lexeme with type, value, line, col
- `TokenList`: Array of tokens (max 8192)
- `ASTNode`: Tree node with type, value, children
- `Parser`: Position tracker in token stream

**Key Patterns:**
- **Visitor Pattern** (generate_latex): Walks AST and emits output
- **Recursive Descent** (parser): Hand-written recursive parser
- **Two-Phase Parsing**: Separate block and inline levels
- **Safety-First Design**: Validates all file I/O, memory allocation, buffer overflows

---

---