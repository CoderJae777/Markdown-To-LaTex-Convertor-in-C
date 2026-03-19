# 50.051 Programming Language Concepts - Parser Project

A command-line compiler written in ANSI C that converts Markdown (`.md`) files into LaTeX (`.tex`) files. Built for the SUTD module 50.051 Programming Language Concepts.

_Done by:_
| Name | Student ID |
| ------------------- | ---------- |
| Goh Yi Shen | 1006852 |
| Joshua Chua Tze Ern | 1006627 |
| Chew Ming Hui | 1006892 |
| Anika Ajay Handigol | 1006952 |
| Jun Jin | 1004435 |

---

## Project Structure

| File                  | Description                                                                          |
| --------------------- | ------------------------------------------------------------------------------------ |
| `main.c`              | Entry point — reads input file or test string, orchestrates lexer → parser → emitter |
| `lexer.c` / `lexer.h` | Tokenizer — scans raw Markdown characters and produces a flat stream of typed tokens |
| `template.c`          | Blank C file template for adding new modules                                         |
| `test.md`             | Extensive Markdown test file covering all supported constructs                       |
| `output/`             | Directory where compiled executables are placed                                      |
| `Admin/`              | Project proposal, notes, and documentation                                           |

---

## Setting Up

### Requirements

- GCC (any version supporting ANSI C / C89)
- A terminal (Command Prompt, PowerShell, bash)

### Install GCC on Windows

Download and install [MinGW-w64](https://www.mingw-w64.org/) or use the GCC bundled with VSCode's C/C++ extension.

Verify your installation:

```bash
gcc --version
```

---

## Building

```bash
gcc -Wall -Werror -ansi -pedantic main.c lexer.c -o output/main.exe
```

---

## Running

Run with your own Markdown file:

```bash
output/main.exe yourfile.md
```

Run with the provided test file:

```bash
output/main.exe test.md
```

---

## Lexing/Tokenisation

### Components

| File | Role |
| --- | --- |
| `main.c` → `read_file()` | Reads `.md` file into a single char buffer |
| `lexer.c` → `lex()` | Scans buffer, fills TokenList via `push()` |
| `lexer.c` → `print_tokens()` | Writes TokenList to `output/tokens.txt` |

---

### Data Flow

```text
test.md (on disk)
      │
      ▼
read_file()
  fopen()
  fseek() + ftell()  →  measure file size
  rewind()
  malloc(size + 3)   →  allocate buffer
  fread()            →  copy file into buffer
  buf[size] = '\0'   →  null terminate
  fclose()
      │
      │  char *file_src
      │  "# Hi\nSome **bold**\n"
      ▼
lex(file_src, &tokens)
  for each character src[i]:
  │
  ├── is_special(c)?
  │
  │   NO  →  append c to text_buf
  │           continue to next char
  │
  │   YES →  flush_text()
  │            if text_buf not empty:
  │              push(TOK_TEXT, text_buf)
  │              reset text_buf
  │
  │          then match c:
  │            '#'   →  push(TOK_HASH)
  │            '**'  →  push(TOK_DOUBLE_STAR)
  │            '*'   →  push(TOK_STAR)
  │            '`'   →  push(TOK_BACKTICK)
  │            '\n'  →  push(TOK_NEWLINE)
  │            ' '   →  push(TOK_TEXT " ")
  │            ...
  │
  push(TOK_EOF)
      │
      │  TokenList tokens (now full)
      ▼
fopen("output/tokens.txt", "w")
      │
      ▼
print_tokens(&tokens, out)
  loop i = 0 to count:
    token_type_name(t->type)
    fprintf(out, ...)
      │
      ▼
output/tokens.txt written
```

---

### `push()` internals

```text
push(list, TOK_HASH, "#", line=1, col=1)
      │
      ├── guard: count >= MAX_TOKENS? → return
      │
      ├── t = &list->tokens[count]   ← point at next empty slot
      │   count++
      │
      ├── t->type  = TOK_HASH
      │   t->line  = 1
      │   t->col   = 1
      │   t->value = "#"
      │
      └── slot filled, count advanced
```

---

### TokenList state over time

```text
Start:       count=0  [        ][        ][        ][        ]
After #:     count=1  [HASH    ][        ][        ][        ]
After " ":   count=2  [HASH    ][TEXT " "][        ][        ]
After "Hi":  count=3  [HASH    ][TEXT " "][TEXT "Hi"][        ]
After \n:    count=4  [HASH    ][TEXT " "][TEXT "Hi"][NEWLINE ]
After EOF:   count=5  [HASH    ][TEXT " "][TEXT "Hi"][NEWLINE ][EOF]
```
