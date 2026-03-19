## Proposal: Markdown &rarr; LaTex

### Tokenising (Lexing)

Scan character by character and split when theres anything **meaningful**

E.g.
Input: "# Heading"

```
Tokens:
HASH "#"
WHITESPACE " "
TEXT "Heading"
NEWLINE "\n"
```

---

### Parsing

Markdown only has 2 parsing phases:

1. Block Level - indentation, leading characters
2. Inline Level - everything else (left to right)

#### Parsing at Block Level

Parser looks at whole lines and their structure to group content into blocks

```
Tokens:
HASH "#"
WHITESPACE " "
TEXT "Heading"
NEWLINE "\n"
```

First token on this line is HASH
&rarr; this line is a heading
&rarr; count how many HASHes: 1 _// If it had seen ## it would be level 2, ### level 3, and so on._
&rarr; discard the WHITESPACE
&rarr; collect remaining TEXT as the heading content
&rarr; emit NODE_HEADING { level=1, text="Heading" }

#### Parsing at Inline Level

The inline parser then runs on the heading's text content "Heading" to check if there is any formatting inside it

"Heading"
──► scan left to right
──► no special characters found
──► emit NODE_TEXT { value="Heading" }

---

### Building the AST

Input:

```
# Hello **world**

- item one
- item two

NODE_DOCUMENT
├── NODE_HEADING (level=1)
│ ├── NODE_TEXT "Hello "
│ └── NODE_BOLD
│ └── NODE_TEXT "world"
└── NODE_LIST (ordered=0)
├── NODE_ITEM
│ └── NODE_TEXT "item one"
└── NODE_ITEM
└── NODE_TEXT "item two"
```

### Emitter (generating the laTex)

The emitter recurses depth-first through the AST, printing LaTeX opening tags before visiting children and closing tags after — building the output file left to right as it goes.

So weneed some sort of case switch database of all markdown to latex