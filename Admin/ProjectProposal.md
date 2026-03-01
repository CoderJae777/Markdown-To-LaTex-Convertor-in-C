## Project Proposal (Group 7): LaTeX-to-Markdown Converter

### Team Members

- Goh Yi Shen, 1006852
- Joshua Chua Tze Ern, 1006627
- Chew Ming Hui, 1006892
- Anika Ajay Handigol, 1006952
- Jun Jin, 1004435

---

### 1) Topic and Problem Statement

LaTeX is widely used for academic and technical publication workflows, while Markdown is preferred for fast drafting (notes, READMEs, documentation).

However, converting LaTeX into Markdown manually is time-consuming and error-prone, especially for complex structures (tables, equations, nested lists) and malformed input.

Our project proposes a C-based LaTeX-to-Markdown converter that parses a restricted LaTeX subset and emits structured Markdown output with robust diagnostics for invalid syntax. The system will also process embedded image references and copy referenced image files in binary mode to the output directory.

---

### 2) Expected Inputs and Outputs

#### Input files

- **Primary text input:** `.tex` LaTeX file

#### Output files

- **Primary text output:** `.md` Markdown file
- **Diagnostics report text output:** `.report.txt` (line/column, error code, recovery action)
- **Binary image copies** (if applicable)

---

### 3) Deliverables

1. C codebase (`.c` / `.h`)
2. Custom lexer + parser with explicit state-machine logic
3. LaTeX-to-Markdown emitter
4. Error handling + recovery system
5. CLI executable and Makefile
6. File I/O handling for both text and binary data
7. README with build/run instructions