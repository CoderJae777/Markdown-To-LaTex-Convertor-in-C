# Comprehensive Markdown Converter Test Suite

This document systematically tests all features of the Markdown-to-LaTeX converter, organized by feature type with "Should Work" and "Shouldn't Work" subsections.

---

## 1. Headings

### 1.1 Should Work

# Heading Level 1
## Heading Level 2
### Heading Level 3
#### Heading Level 4
##### Heading Level 5
###### Heading Level 6

Multiple headings in sequence:
# First Section
## Subsection 1
### Sub-subsection 1
## Subsection 2
# Second Section

Headings with inline formatting:
## Section with **bold** title
### Section with *italic* title
#### Section with `code` in title

### 1.2 Shouldn't Work

####### Seven hashes (too many levels)
# Heading with no space after hash#
# Heading at end of line #
Heading without hash prefix

---

## 2. Inline Text Formatting

### 2.1 Should Work

**Bold text** using double asterisks
__Bold text__ using double underscores
*Italic text* using single asterisk
_Italic text_ using single underscore
***Bold and italic*** using triple asterisks
___Bold and italic___ using triple underscores

**Nested bold with *italic inside* bold**
*Nested italic with **bold inside** italic*
***Triple nested **deep** bold***

Bold at **start** of sentence
Sentence ending with **bold** here.
Multiple **bold** words **in** one line.
Multiple *italic* words *in* one line.

Mix of **bold**, *italic*, ***both***, and normal text.

Text with *asterisk and **nested bold** inside*.

### 2.2 Shouldn't Work

**Bold without closing asterisk
*Italic without closing asterisk
** **Empty bold markers
* *Empty italic markers
\*\*Escaped asterisks (if not supported)
**Bold *without proper nesting** and*
*Italic **without proper nesting* and**

---

## 3. Code

### 3.1 Should Work

Inline `code` in text
`Code with special chars: $#%&_`
`Multiple` pieces `of inline code`

**Bold with `inline code` inside**
*Italic with `inline code` inside*

```
Plain code block
line 2
line 3
```

```c
/* C code block */
#include <stdio.h>
int main() {
    printf("Hello, World!\n");
    return 0;
}
```

```python
# Python code block
def greet(name):
    return f"Hello, {name}"
print(greet("World"))
```

```
Code with **markdown symbols** that should be literal
Code with *asterisks* and _underscores_
```

### 3.2 Shouldn't Work

`Unclosed inline code
``Double backtick code`` (if not supported)

```
Unclosed code block

No closing ticks

Triple backtick inside code block:
```
nested```

---

## 4. Lists

### 4.1 Unordered Lists Should Work

- Item one
- Item two
- Item three

- Item with **bold**
- Item with *italic*
- Item with `code`

- First item
- Second item
  - Nested item 1
  - Nested item 2
    - Deeply nested
    - More deeply nested
  - Back to second level
- Third item

- Item with multiple **bold**, *italic*, and `code` inline

### 4.2 Ordered Lists Should Work

1. First item
2. Second item
3. Third item

1. Item with **bold**
2. Item with *italic*
3. Item with `code`

1. First step
2. Second step
   1. Sub-step 1
   2. Sub-step 2
      1. Sub-sub-step 1
      2. Sub-sub-step 2
   3. Sub-step 3
3. Third step

### 4.3 Mixed Lists Should Work

- Unordered item 1
- Unordered item 2
  1. Ordered sub-item 1
  2. Ordered sub-item 2
     - Unordered sub-sub-item
     - Another sub-sub-item
  3. Ordered sub-item 3
- Unordered item 3

1. Ordered item 1
   - Unordered sub-item 1
   - Unordered sub-item 2
2. Ordered item 2

### 4.4 Lists Shouldn't Work

- Item without proper indentation for nesting (only 1-2 spaces)
  - This might not nest correctly

Paragraph breaks in middle
- Item 1

- Item 2

List starting with wrong marker
* Item (asterisk instead of dash)
+ Item (plus instead of dash)

0. Zero-indexed ordered list (non-standard)
-1. Negative indexed list

---

## 5. Blockquotes

### 5.1 Should Work

> This is a blockquote

> This is a blockquote with **bold** and *italic*

> Multi-line blockquote
> Line 2
> Line 3

> Blockquote with `inline code`

> Nested idea
> > Inner blockquote
> > Still inner
> Back to outer

> - List inside blockquote
> - Item 2
> - Item 3

> 1. Ordered list inside blockquote
> 2. Item 2

> # Heading in blockquote

### 5.2 Shouldn't Work

>No space after angle bracket (might still work)

Blockquote after paragraph
> Quote without blank line

> Unclosed blockquote
Next line (should this be indented?)

---

## 6. Links and Images

### 6.1 Links Should Work

[Simple link](https://example.com)

[Link with **bold** text](https://example.com)

[Link with *italic* text](https://example.com/page)

[Link with `code` inside](https://example.com/code)

Multiple [link one](https://example1.com) and [link two](https://example2.com) in one line.

[Link to local file](./file.md)

[Link with special chars](https://example.com/path?q=1&t=2#section)

### 6.2 Images Should Work

![Simple image](./images/photo.png)

![Image with alt text](./images/diagram.png)

Multiple images: ![img1](./img1.png) and ![img2](./img2.png)

### 6.3 Links and Images Shouldn't Work

[Unclosed link text](https://example.com (missing closing bracket)

[Link without URL](no-url)

[Link with wrong bracket][https://example.com]

![Unclosed image alt text](./image.png

Bare URL: https://example.com (no link syntax)

---

## 7. Code Blocks (Fenced)

### 7.1 Should Work

```javascript
// JavaScript code
const x = 42;
console.log(`The answer is ${x}`);
```

```ruby
# Ruby code
def factorial(n)
  n <= 1 ? 1 : n * factorial(n - 1)
end
```

```bash
#!/bin/bash
echo "Hello from bash"
for i in {1..5}; do
  echo "Number: $i"
done
```

```
No language specified
Just plain text
Can contain anything
```

### 7.2 Code Blocks Shouldn't Work

`` Double backtick open (should use triple)

```
Unclosed code fence
No closing fence

Text continues here

```
Extra backticks on close

---

## 8. Math

### 8.1 Inline Math Should Work

The equation $x^2 + y^2 = z^2$ is Pythagorean theorem.

Euler's identity: $e^{i\pi} + 1 = 0$

Quadratic formula: $x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$

Greek letters: $\alpha + \beta = \gamma$

Matrix notation: $\begin{pmatrix} a & b \\ c & d \end{pmatrix}$

Multiple equations in one line: $a + b$ and $c * d$ and $e / f$

### 8.2 Block Math Should Work

$$
x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}
$$

$$
e^{i\pi} + 1 = 0
$$

$$
\sum_{i=1}^{n} i = \frac{n(n+1)}{2}
$$

$$
\int_{0}^{\infty} e^{-x^2} dx = \frac{\sqrt{\pi}}{2}
$$

$$
\begin{align}
a &= b + c \\
d &= e + f
\end{align}
$$

### 8.3 Math Shouldn't Work

$Unclosed inline math
Unclosed inline math$

$$ Unclosed block math

$$Unclosed block math

Inline math with newline: $x +
y$ (broken across lines)

---

## 9. Tables

### 9.1 Simple Tables Should Work

| Name | Age | City |
|------|-----|------|
| Alice | 30 | Singapore |
| Bob | 25 | Tokyo |
| Charlie | 35 | London |

### 9.2 Tables with Alignment Should Work

| Left | Center | Right |
|:-----|:-------:|------:|
| L1 | C1 | R1 |
| L2 | C2 | R2 |

### 9.3 Tables with Formatting Should Work

| Feature | Supported | Notes |
|---------|-----------|-------|
| **Bold** | Yes | \*\*text\*\* |
| *Italic* | Yes | \*text\* |
| `Code` | Yes | \`text\` |
| [Links](https://example.com) | Yes | [text](url) |
| Math | Partial | $x^2$ |

### 9.4 Complex Table Should Work

| Complex | Example | Description |
|---------|---------|-------------|
| With **bold**, *italic*, and `code` | Yes | All inline formatting |
| With $math$ inside | Yes | $x^2 + y^2$ |
| Multi-word cells | Yes | This cell has multiple words |

### 9.5 Tables Shouldn't Work

| Missing closing pipe
| Col1 | Col2 |
|------|------|

Separator with wrong format:
| Col1 | Col2 |
| ---- | ---- |  (extra spaces)

Table without separator:
| Col1 | Col2 |
| A1 | A2 |

| Uneven columns |
|--------|---------|
| Data | Extra |

---

## 10. Paragraphs and Spacing

### 10.1 Should Work

This is a paragraph.

This is a new paragraph (blank line above).

This paragraph
continues on multiple lines
but is still one paragraph.

Paragraph with **bold**, *italic*, `code`, and [link](https://example.com).

Paragraph ending with sentence. Another sentence follows.

### 10.2 Spacing Behavior

Paragraph 1.

Paragraph 2 (one blank line above).

Paragraph 3 (one blank line above).



Paragraph 4 (multiple blank lines above - behavior may vary).

### 10.3 Edge Cases

Single word paragraph.

Paragraph with single punctuation.

Paragraph   with   multiple   spaces   (collapse to single spaces).

Paragraph\nwith escaped newline (may not work as line break).

---

## 11. Special Characters and Escaping

### 11.1 Should Work (Basic special chars)

Price is $50.
Discount is 10%.
Range: 1 < x < 10.
Answer: x > 0.
Use C & C++ for programming.

Text with underscore_in_middle.
Text with dash-in-middle.

### 11.2 Escaped Characters (May Not Work)

This is \*not italic\*.
This is \*\*not bold\*\**.
This is \[not a link\](url).
This is \$not math\$.

Escaped underscore: \_text\_.

---

## 12. Combinations and Complex Structures

### 12.1 Heading with Code and Math

## Using $\LaTeX$ and `code` Together

Inline code `printf("%d", x)` with math $\sum x$.

### 12.2 List with Multiple Formatting

- Item with **bold** and *italic*
- Item with `code` and $x^2$
- Item with [link](https://example.com)
- Item with everything: **bold**, *italic*, `code`, $y = mx + b$, [link](https://example.com)

### 12.3 Blockquote with Formatting

> This blockquote has **bold**, *italic*, and `code`.
> 
> It also has math: $E = mc^2$
> 
> And a [link](https://example.com).

### 12.4 Table in Complex Context

Complex table with mixed content:

| Type | Example | Rendered |
|------|---------|----------|
| **Bold** | `**text**` | **bold** |
| *Italic* | `*text*` | *italic* |
| Code | `` `text` `` | `code` |
| Math | `$x^2$` | $x^2$ |
| [Link](https://example.com) | `[text](url)` | [link](https://example.com) |

### 12.5 Nested Structures

1. Ordered item 1
   - Unordered subitem
   - Another unordered subitem
     ```
     code block inside nested list
     ```
   - Back to unordered
2. Ordered item 2
   > Blockquote in list
   > Multi-line quote
3. Ordered item 3

---

## 13. Stress Tests and Edge Cases

### 13.1 Very Long Content

This is a very long paragraph that continues for many lines with multiple sentences to test how the converter handles lengthy text blocks without any special formatting or line breaks. It should all be treated as a single paragraph and should flow normally into the LaTeX output without issues. Line wrapping should be handled by the LaTeX compiler, not by the Markdown converter. This paragraph is intentionally long and repetitive to ensure that the converter doesn't have any issues with extended text.

### 13.2 Empty or Minimal Content

Empty list:
-

Minimal paragraph:
.

Single character:
x

### 13.3 Repeated Formatting

**bold** **bold** **bold**
*italic* *italic* *italic*
`code` `code` `code`

### 13.4 Deeply Nested Lists

- Level 1
  - Level 2
    - Level 3
      - Level 4
        - Level 5
          - Level 6 (may hit depth limits)

1. Level 1
   1. Level 2
      1. Level 3
         1. Level 4
            1. Level 5 (deeply nested)

### 13.5 Many Columns in Table

| Col1 | Col2 | Col3 | Col4 | Col5 | Col6 | Col7 | Col8 |
|------|------|------|------|------|------|------|------|
| A | B | C | D | E | F | G | H |
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |

### 13.6 Large Table

| Row | Data1 | Data2 | Data3 |
|-----|-------|-------|-------|
| 1 | Value | Value | Value |
| 2 | Value | Value | Value |
| 3 | Value | Value | Value |
| 4 | Value | Value | Value |
| 5 | Value | Value | Value |
| 10 | Value | Value | Value |
| 20 | Value | Value | Value |

---

## 14. Unicode and Special Symbols

### 14.1 Should Work (ASCII-safe content)

Copyright: (c)
Registered: (r)
Trademark: (tm)
Degree: Celsius is 25°C

Arrows: <- -> <-> =>

### 14.2 May Not Work (Unicode)

Mathematical symbols: ±, ×, ÷, ≈, ≠, ≤, ≥
Greek text outside math: α, β, γ, δ, ε
Other symbols: €, ¥, £, ©, ®, ™
Emoji: 😀 ✓ ✗ ⚠ ℹ

---

## 15. Document Structure

### 15.1 Typical Article Structure

# Main Title

## Introduction

This is the introduction section with some content.

## Background

Some background information here.

## Method

- Step 1: Do this
- Step 2: Do that
  - Sub-step 2a
  - Sub-step 2b

Code example:
```
example code
```

## Results

| Metric | Value |
|--------|-------|
| Accuracy | 95% |
| Precision | 92% |

Math results: $R^2 = 0.98$

## Discussion

Discussion of findings.

## Conclusion

Final thoughts and conclusion.

---

## 16. Known Limitations and Unsupported Features

### 16.1 Unsupported Markdown Syntax

- Horizontal rules (----, ****, ____)
- Strikethrough (~~text~~)
- Subscript and superscript (H~2~O, x^2^)
- Task lists (- [ ] Task, - [x] Done)
- Footnotes ([^1], [^1]: Note text)
- Definition lists (Term : Definition)
- HTML blocks (<div>, <table> tags)
- Raw HTML (<span style="color:red">text</span>)

### 16.2 Limited or Partial Support

- Escaped characters (may not work as expected)
- Nested formatting edge cases
- Very deep list nesting (may have depth limits)
- Unicode and emoji support

### 16.3 Behavior That May Vary

- Multiple consecutive blank lines
- Spaces inside formatting markers
- Mixed tab and space indentation
- Windows vs Unix line endings

---

## Summary

This comprehensive test file covers:
- ✓ 15 main feature sections
- ✓ ~100+ individual test cases
- ✓ Both should-work and shouldn't-work scenarios
- ✓ Edge cases and stress tests
- ✓ Known limitations and unsupported features

**Use this to verify converter behavior comprehensively and identify any issues or edge cases that need attention.**
