# Contributing to ADMC
This file documents contributing guidelines for ADMC project.

## Coding style
Those guidelines are based on [Qt Coding
Style](https://wiki.qt.io/Qt_Coding_Style) and [LLVM coding
standards](https://llvm.org/docs/CodingStandards.html).

It is possible that the code base does not currently comply with these
guidelines.  We are not looking for a massive PR that fixes everything at once
as it can be hard to review and merge.  However, all new contributions should
make a best effort to clean up and make the code base better than they left it.
Obviously, apply your best judgment.  Remember, the goal here is to make the
code base easier for humans to navigate and understand.  Always keep that in
mind when nudging others to
comply.[[source](https://github.com/docker/cli/blob/master/CONTRIBUTING.md#coding-style)]

**Note** that formatting style in ADMC is enforced by `.clang-format` (see
["Tools support"](#tools-support) section below.)

### Commenting
#### Comments in code
Comments are important for readability and maintainability.  When writing
comments, write them in English, aim to describe the intent and rationale, not
how the code works at a micro level.
[[source](https://llvm.org/docs/CodingStandards.html#commenting)]

How the code works must be clear from the code itself.  If you find yourself
writing in the comments **how** your code works, maybe it is the time to stop
and re-factor your code so it will be more clear.

In ADMC we're using [Doxygen](https://www.doxygen.nl/index.html) comments as
they can be automatically processed to produce developer documentation.

Public APIs must be documented, while comments in the internal code base are
welcomed, but not required.  The rule of thumb for writing comments is that they
must convey some helpful information about code, otherwise they are not needed
at all.

Example:
```cpp
/**
 * Divide one number by another.
 *
 * Asserts that "b" parameter is not 0.
 * (This is an optional long description of the function, e.g. with description
 *  of  some usage caveats.)
 *
 * @param a A value to be divided.
 * @param b The divisor.
 * @return The result of division.
 */
float divide(float a, float b) {
    assert(b != 0);
    return a / b;
}
```

#### File headers
Each file must have a copyright header.

A header example:
```cpp
/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
```

Optionally the header can contain useful file-level description when it provides
useful architectural context or some non-obvious details that are not apparent
from the file name.  In such case this information can be written right below
"ADMC - AD Management Center" line.  For example:

```cpp
/*
 * ADMC - AD Management Center
 *
 * Some useful infromation about the file can be written here.
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 *
 * [ ... the rest of the header ... ]
 */
```

Don't update your copyright date unless you made a change to this file.  Don't
touch other people copyright dates.  Update your copyright date with the change
you are going to commit.

### Spacing
#### Don't skimp on spaces
Judicious use of space can make code easier to read.  Leave a space after each
comma, and surround binary operators with spaces.

```cpp
// Bad for reading:
int a=2;
if(a<5){

}

// Good:
int a = 2;
if (a < 5) {

}
```

#### Indent code only with spaces, don't use tabs
Indentation must be set to 4 spaces.

#### Write your code to fit into 80 columns
It allows developers to have multiple files opened side-by-side on a modest
display. [[source](https://llvm.org/docs/CodingStandards.html#source-code-width)]

Also when a line length limit is enforced it also helps to limit the code
nesting levels.  Paraphrasing Linux kernel developers, if you need more than 3
levels of indentation so your code does not fit into 80 column limit, you’re
screwed anyway, and should fix your
program. [[source](https://www.kernel.org/doc/html/v4.10/process/coding-style.html#indentation)]

#### Don't add trailing whitespaces at the end of lines
Be careful not to add trailing whitespaces at the end of lines.

Examples:
```cpp
// Bad:
int a = 5;  
//        ^^ two trailng spaces here

// Good:
int a = 5;
//        ^^ no trailing spaces here.
```

### Names
#### Declare each variable on a separate line
Examples:
```cpp
// Bad:
int width, heigth;

// Good:
int width;
int height;
```

#### Avoid short or meaningless names, avoid abbreviations
The length of a name must be proportional to its scope.

Single character variable names are only okay for counters and temporaries,
where the purpose of the variable is obvious.  And even for counters we
recommend more meaningful names -- e.g. use `index` or `idx` instead of just `i`
as single-letter variables are harder to find.

Also avoid abbreviations where it is possible as they make code cryptic and hard
to understand.

Examples:
```cpp
// Bad:
int a;
int abracadabra;
short cntr;

// Good:
int width;
int height;
short counter;
```

#### Wait when declaring a variable until it is needed
Examples:
```cpp
// Bad:
void fun() {
    int a;
    // ... more code ...
    a = 5;
}

// Good:
void fun() {
    // ... some code ...
    int a = 5;
    // .. some code that uses "a" ...
}
```

#### Use underscores in names (except for class names)
If a variable name, constant name, function or method name consists of several
words, they must be separated with an underscore.  Exception: class constructor
names.

Examples:
```cpp
// Bad:
int a, b;
char *c, *d;
char itemDelimiter = ':';

// Good:
int height;
int width;
char item_delimiter = ':';
char *name_of_this;
char *name_of_that;
```

#### Constants names must be written in upper-case letters
Constants names must be written in upper-case letters, to visually distinguish
them from variables.  Each consecutive word in a constant name must be separated
from others with an underscore.

Examples:
```cpp
// Bad:
const int item_delimiter = ':'
int WINDOW_WIDTH = 1024;

 // Good:
int item_delimiter = ':';
const int WINDOW_WIDTH = 1024;
```

#### Class names always start with an upper-case letter
If a class name consists of several words, the [camel
 case](https://en.wikipedia.org/wiki/Camel_case) style must be used.

Acronyms in class names are camel-cased (e.g. `XmlStreamReader`, not
`XMLStreamReader`).

Examples:
```cpp
// Bad:
class Some_class {
};

class anotherClass {
};

 // Good:
class SomeClass {
};

class AnotherClass {
};

```

### Braces
#### Use attached braces
The opening brace goes on the same line as the start of the statement.  If the
closing brace is followed by another keyword, it goes into the same line as
well.

Examples:
```cpp
// Bad:
if (codec)
{
    // ... some code ...
}
else
{
    // ... some alternative code ...
}

// Good:
if (codec) {
    // ... some code ...
} else {
    // ... some alternative code ...
}
```

#### Use curly braces even in case of an one-line statement body
Rationale: [goto fail Apple
  issue](https://www.blackduck.com/blog/understanding-apple-goto-fail-vulnerability-2.html).

Examples:
```cpp
// Bad:
if (address.isEmpty())
    return false;

for (int idx = 0; idx < 10; idx++)
    qDebug("%i", idx)

// Good:
if (address.isEmpty()) {
    return false;
}

for (int idx = 0; idx < 10; idx++) {
    qDebug("%i", idx);
}
```

### Parenthesis
#### Use parentheses to group expressions
Examples:
```cpp
// Bad:
if (a && b || c) {
    // ...
}

// Good:
if ((a && b) || c) {
    // ...
}

// Bad:
int n = a + b & c;

// Good:
int n = (a + b) & c;
```

### Statements
#### Resource Acquisition Is Initialization (RAII)
It is recommended to follow [Resource Acquisition Is Initialization
(RAII)](https://en.cppreference.com/cpp/language/raii) guidelines for resource
management.

#### `switch` statement
- The case labels are in the same column as the switch.
- Every case must have a `break` (or `return`) statement at the end.  Every case
  must have a `break` (or `return`) statement at the end or use
  `Q_FALLTHROUGH()` to indicate that there's intentionally no break, except
  where there is no code between two case labels.

```cpp
switch (my_enum) {
case VALUE_1:
    do_something();
    break;
case VALUE_2:
case VALUE_3:
    do_something_else();
    Q_FALLTHROUGH();
default:
    default_handling();
    break;
}
```

### Tests and checks
#### Test observable and non-trivial internal logic
Tests help us to catch bugs before they bite a user.  The act of test writing
itself sometimes helps us to find bugs.

Also when we are fixing something, it is good to have a test which ensures that
the error we fixed will not surface again.

#### Check preconditions and assumptions with `assert`
Use `assert` macro to to check preconditions and assumptions, where it is
appropriate. [[source](https://llvm.org/docs/CodingStandards.html#assert-liberally)**

**Note** that `assert` macro must be reserved only for checks assumptions about
the internal state of the application, and it does not replace other validation
and checks in code.

Example:
```cpp
#include <cassert>

// ...

int handle_object(SomeObject* object) {
    assert(obj != NULL);
    // Do somthing with the object.
}
```

To further assist with debugging, make sure to put some kind of error message in
the assertion statement, which is printed if the assertion is tripped.  This
helps the poor debugger make sense of why an assertion is being made and
enforced, and hopefully what to do about it.  Here is one complete example:

Example:
```cpp
#include <cassert>

// ...

int handle_object(SomeObject* object) {
    assert((obj != NULL) && "Object is NULL!");
    // Do somthing with the object.
}

```

### Tools support
To format the code according the rules you can use `clangformat` target for
`make`:

```shell
$ make -C build clangformat
```

**Note** that we discourage intentional large-scale reformatting in feature
commits.  Make sure that you don't re-format files that are not affected by the
change you are committing.  Coding style changes of huge chunks of code have to
be committed separately.

## Writing commit messages
We recommend to use [Conventional Commits](https://www.conventionalcommits.org/)
for writing commit messages.

The format is well-documented and formalized, and such commit messages can be
parsed automatically by scripts/bots (e.g. for preparing release notes.)

- The length of the first line in the commit message SHOULD be around 50
  characters in length (including change type and scope), and MUST NOT be longer
  than 72 characters.  It helps to keep the Git history readable in all
  circumstances.
- Take your time and write detailed commit descriptions (in other words, the
  body of the message), unless the change you have made is trivial.  This allows
  a viewer to quickly understand what was changed and why, without the need to
  look inside the code.  The commit message body MUST be separated from the
  short description (the first line) by an empty line.  The length of each line
  in the commit message body MUST NOT be greater than 72 characters.
- While the Conventional Commits specification does not formalize the commit
  message body format, we suggest to use [GNU
  ChangeLog](https://www.gnu.org/prep/standards/html_node/Style-of-Change-Logs.html)
  format inside commit description body in the cases when it is useful to
  indicate what file was changed, and which part of it.

A commit message example:
```
feat(core): Add "foo-procedure"

A long description of the changes that has been made.  If it is too long
we must add line breaks.  This optionally follows by the list of files
that were changed in the commit in the GNU ChangeLog format.

* src/core/file.c (foo-procedure): New procedure.
* src/core/file.h: Export it.

Signed-off-by: Vasiliy I. Pupkin <vip@altlinux.org>
```
