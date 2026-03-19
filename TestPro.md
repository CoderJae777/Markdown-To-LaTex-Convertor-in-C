# Document Title

## 1. Headings

# Heading Level 1
## Heading Level 2
### Heading Level 3
#### Heading Level 4
##### Heading Level 5
###### Heading Level 6

## 2. Inline Formatting

This is **bold text** in a sentence.
This is *italic text* in a sentence.
This is ***bold and italic*** in a sentence.
This is `inline code` in a sentence.
This is **bold with *nested italic* inside** it.
This is *italic with **nested bold** inside* it.
This has **multiple** bold **words** in one line.
This has *multiple* italic *words* in one line.

## 3. Paragraphs

This is the first paragraph. It has multiple sentences. All on the same block of text with no blank line separating them.

This is the second paragraph. A blank line above separates it from the first. Paragraphs are the most common block element in Markdown.

This is a third paragraph. It comes after another blank line. Each paragraph becomes its own block node in the AST.

## 4. Unordered Lists

- Item one
- Item two
- Item three

- Item with **bold** text
- Item with *italic* text
- Item with `inline code`

- Top level item A
  - Nested item A1
  - Nested item A2
    - Deeply nested item A2a
    - Deeply nested item A2b
  - Nested item A3
- Top level item B
  - Nested item B1
- Top level item C

## 5. Ordered Lists

1. First item
2. Second item
3. Third item

1. First item with **bold**
2. Second item with *italic*
3. Third item with `code`

1. Top level step one
   1. Sub-step one
   2. Sub-step two
      1. Sub-sub-step one
      2. Sub-sub-step two
   3. Sub-step three
2. Top level step two
3. Top level step three

## 6. Mixed Lists

- Unordered item one
- Unordered item two
  1. Ordered sub-item one
  2. Ordered sub-item two
- Unordered item three

## 7. Blockquotes

> This is a simple blockquote.

> This is a blockquote with **bold** and *italic* inside it.

> First line of a multi-line blockquote.
> Second line of the same blockquote.
> Third line of the same blockquote.

> Outer blockquote line one.
> Outer blockquote line two.

## 8. Code

Inline code: `x = 5 + y`

Inline code with symbols: `printf("%s", "hello")`

Inline code with backtick content: ``use `backtick` inside``

```
plain code block with no language
line two of code block
line three of code block
```

```c
/* C code block */
#include <stdio.h>

int main(void) {
    printf("Hello, world!\n");
    return 0;
}
```

```python
# Python code block
def greet(name):
    return f"Hello, {name}"

print(greet("world"))
```

## 9. Links and Images

[Simple link](http://example.com)

[Link with **bold** label](http://example.com)

[Link with *italic* label](http://example.com/page)

![Simple image](images/photo.png)

![Image with alt text](images/diagram.png)

## 10. Inline Math

The equation $x^2 + y^2 = z^2$ is Pythagoras theorem.

Euler's identity is $e^{i\pi} + 1 = 0$.

The quadratic formula is $x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$.

## 11. Block Math

$$
x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}
$$

$$
\sum_{i=1}^{n} i = \frac{n(n+1)}{2}
$$

$$
\int_{0}^{\infty} e^{-x^2} dx = \frac{\sqrt{\pi}}{2}
$$

## 12. Tables

| Name    | Age | City      |
|---------|-----|-----------|
| Alice   | 30  | Singapore |
| Bob     | 25  | Tokyo     |
| Charlie | 35  | London    |

| Left aligned | Center aligned | Right aligned |
|:-------------|:--------------:|--------------:|
| cell 1       | cell 2         | cell 3        |
| cell 4       | cell 5         | cell 6        |

| Feature      | Supported | Notes              |
|--------------|-----------|---------------------|
| **Bold**     | Yes       | `\textbf{}`        |
| *Italic*     | Yes       | `\textit{}`        |
| `Code`       | Yes       | `\texttt{}`        |
| Tables       | Yes       | `tabular` env      |
| Images       | Partial   | no size control    |

## 13. Horizontal Rules

Above the rule.

---

Below the rule.

Another paragraph.

---

Another section after a rule.

## 14. Edge Cases

### Empty formatting markers

A line with a lone star * in the middle.
A line with a lone underscore _ in the middle.
A line with a lone backtick ` in the middle.

### Special characters in text

Price is $50 and discount is 10%.
Use C & C++ for systems programming.
The range is 1 < x < 10.
The answer is 42 > 0.

### Punctuation around formatting

**bold** at the start of a line.
A sentence ending with **bold**.
(*italic inside parentheses*)
A comma after **bold**, then more text.
A period after *italic*.

### Consecutive formatting

**bold** *italic* `code` in a row.

### Long paragraph

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.

## 15. Combined Elements

### A realistic section

The **lexer** is the first phase of a compiler. It reads a raw character stream and produces a flat list of *tokens*. Each token has:

- A **type** (e.g. `HASH`, `STAR`, `TEXT`)
- A **value** (the matched text)
- A **line** and **column** for error reporting

The lexer does not understand structure. That is the job of the **parser**, which consumes the token stream and builds an *Abstract Syntax Tree* (AST).

For more details see the [compiler architecture](http://example.com/arch).

The key formula driving our tokenizer complexity is $O(n)$ where $n$ is the number of input characters.

| Phase   | Input       | Output     |
|---------|-------------|------------|
| Lexer   | `char *`    | `Token[]`  |
| Parser  | `Token[]`   | `Node *`   |
| Emitter | `Node *`    | `char *`   |
