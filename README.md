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

#### Requirements

- GCC (any version supporting ANSI C / C89)
- A terminal (Command Prompt, PowerShell, bash)

#### Install GCC on Windows

Download and install [MinGW-w64](https://www.mingw-w64.org/) or use the GCC bundled with VSCode's C/C++ extension.

Verify your installation:

```bash
gcc --version
```

---

## Compiling

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
// Minimal test
output/main.exe Test.md

// Every single markdown character
output/main.exe TestPro.md

```

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
