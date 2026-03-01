# Notes
---

1) Topic and Problem Statement

LaTeX is widely used for academic and technical writing, while Markdown is preferred for lightweight documentation and publishing workflows.

Manual conversion from LaTeX to Markdown is time-consuming and error-prone, especially for nested structures, malformed environments, and inconsistent syntax.

This project proposes a C-based LaTeX-to-Markdown converter that parses a formally defined subset of LaTeX and produces structured Markdown output. The system will include a robust custom lexer and parser capable of detecting and recovering from malformed input.

The supported subset will include:
- Sectioning commands (\section, \subsection)
- Inline formatting (bold, italic)
- Nested lists
- Basic tables
- Inline and block math delimiters
- Environment tracking using a stack-based state machine

### 4) Related Work and Differentiation *might remove*

OpenAI’s [Prism](https://openai.com/prism/) is a **LaTeX-native scientific writing workspace** that integrates GPT-5.2 for drafting, revising, collaboration, and publication preparation inside a cloud editor. It is designed to support the LaTeX authoring workflow rather than perform deterministic format conversion between markup languages. 

Our project targets a different problem: **reliable, parser-driven document conversion** between LaTeX and Markdown for portability and documentation reuse. Specifically, we focus on a constrained, formally defined subset and provide:

- deterministic conversion behavior (same input + options => same output),
- custom lexer/parser implementation in ANSI C,
- explicit state-machine design,
- robust diagnostics and recovery for malformed input,
- CLI-based batch processing suitable for local/CI workflows.

In short, Prism improves writing within a LaTeX workspace, while our tool addresses **structured cross-format translation** and parser robustness, an engineering gap Prism does not primarily aim to solve