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

Run with the built-in test string:

```bash
output/main.exe
```

Run with your own Markdown file:

```bash
output/main.exe yourfile.md
```

Run with the provided test files:

```bash

// Simple
output/main.exe Test.md

// Pro Version - Every single markdown
output/main.exe TestPro.md

```

---
