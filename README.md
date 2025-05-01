[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/1R7dNcsG)
﻿<!--toc:start-->

- [Programming Assignment: Sloc Project](#programming-assignment-sloc-project)
  - [Introduction](#introduction)
  - [Input](#input)
  - [Interface](#interface)
  - [Execution](#execution)
    - [Single line of code, comments, and blank lines](#single-line-of-code-comments-and-blank-lines)
    - [Special case 1: comment marks inside other comments.](#special-case-1-comment-marks-inside-other-comments)
    - [Special case 2: comment marks in literal](#special-case-2-comment-marks-in-literal)
    - [Special case 3: line that yields a sloc and a comment](#special-case-3-line-that-yields-a-sloc-and-a-comment)
    - [Special case 4: doc comments](#special-case-4-doc-comments)
  - [System Modeling](#system-modeling)
    - [The list of source files](#the-list-of-source-files)
    - [The `FileInfo` class/struct](#the-fileinfo-classstruct)
    - [Helper functions](#helper-functions)
  - [Output](#output)
    - [Error output](#error-output)
  - [Assignment Grading](#assignment-grading)
  - [Extra credits](#extra-credits)
- [Authorship and Collaboration Policy](#authorship-and-collaboration-policy)
- [Work Submission](#work-submission)
<!--toc:end-->

# Programming Assignment: Sloc Project

## Introduction

Counting the **lines of code** (LOC) of a programming project is a standard _software metric_
used to measure the size of a computer program.
This information may be used, for instance, to objectively measure the programming effort
done by a team of developers, as well as to estimate software documentation practices by
calculating the percentage of comments in a given project.

In this programming assignment you should design and develop a terminal application that
counts how many lines of code, comments, and blank lines were found
in one or more C/C++ source files provided as input, and prints out this information
to the standard output.

The application should provide interaction via a CLI (command line interface), and may receive
multiples C/C++ source file as input and/or multiple directories containing one or more source files.

Below you will find an example of expected output when running the application on a single C++ source file.

```
$ ./sloc src/main.cpp
Files processed: 1
------------------------------------------------------------------------------------------------------------------
Filename            Language      Comments        Doc Comments    Blank           Code            # of lines
------------------------------------------------------------------------------------------------------------------
src/main.cpp        C++           1 (3.6%)        9 (32.1%)       3 (10.7%)       15 (53.6%)      28
------------------------------------------------------------------------------------------------------------------
```

This project may help you develop your object-oriented programming skills since you will need
to model this application with classes. This assignment also relies heavily on [strings](https://en.cppreference.com/w/cpp/string/basic_string) and [stream](https://en.cppreference.com/w/cpp/io/basic_ostringstream) manipulation, [file streams](https://en.cppreference.com/w/cpp/io/basic_fstream), the processing and parsing of command line arguments. Of course, you are encouraged to choose convenient data structures such as **lists** (`vector` or `array`), as well as **dictionary**, to help improve your solution and make the problem easier to solve.

## Input

As stated before, the main data input for the program are C/C++ source files and/or _directories_ containing C/C++ source files.
Feel free to add support to other languages if you so desire (extra feature).

Other input information are the options to select which type of sorting order you will use to present the data table, or if the user wants to investigate the directories provided recursively.

## Interface

If you run `sloc` with no arguments, or with the flag `--help` or `-h` the program should display the _usage_ message to help the client to understand how to use the program.

```
$ ./build/sloc -h
Welcome to sloc cpp, version 1.0, (c) DIMAp/UFRN.

NAME
  sloc - single line of code counter.

SYNOPSIS
  sloc [-h | --help] [-r] [(-s | -S) f|t|c|b|s|a] <file | directory>

EXAMPLES
  sloc main.cpp sloc.cpp
     Counts loc, comments, blanks of the source files 'main.cpp' and 'sloc.cpp'

  sloc source
     Counts loc, comments, blanks of all C/C++ source files inside 'source'

  sloc -r -s c source
     Counts loc, comments, blanks of all C/C++ source files recursively inside 'source'
     and sort the result in ascending order by # of comment lines.

DESCRIPTION
  Sloc counts the individual number **lines of code** (LOC), comments, and blank
  lines found in a list of files or directories passed as the last argument
  (after options).
  After the counting process is concluded the program prints out to the standard
  output a table summarizing the information gathered, by each source file and/or
  directory provided.
  It is possible to inform which fields sloc should use to sort the data by, as
  well as if the data should be presented in ascending/descending numeric order.

OPTIONS:
  -h/--help
            Display this information.

  -r
            Look for files recursively in the directory provided.

  -s f|t|c|d|b|s|a
            Sort table in ASCENDING order by (f)ilename, (t) filetype,
            (c)omments, (d)oc comments, (b)lank lines, (s)loc, or (a)ll.
            Default is to show files in ordem of appearance.

  -S f|t|c|d|b|s|a
            Sort table in DESCENDING order by (f)ilename, (t) filetype,
            (c)omments, (d)oc comments, (b)lank lines, (s)loc, or (a)ll.
            Default is to show files in ordem of appearance.
```

## Execution

After receiving and parsing the command-line arguments, your program may first try to create a list of valid C/C++ source files based on the input source. If the source is just a file name, you may need to _extract_ the file extension to see if it matches the accepted supported file extensions, listed below:

- `".cpp"`: a C/C++ program.
- `".hpp"`: a C++ header file.
- `".c"`: a C program.
- `".h"`: a C/C++ header file.

On the other hand, if your source is a directory name, you will need to access all the file names inside the source directory, check if they have the supported extensions listed above, and include them into the list of valid C/C++ source files. Files that do not have any of the extensions listed above should be ignored.

If the user has specified the option `-r`, your program should explore any directory provided **recursively**.
That means that all the source files found in the folder of any internal subfolder must be also considered for counting.

To learn how to handle files and directories, refer to the C++ [Filesystem library](https://en.cppreference.com/w/cpp/filesystem) and this [chapter](https://www.learncpp.com/cpp-tutorial/basic-file-io/).

After your program has created a list of valid C/C++ source files (that might have only one file, for instance), the next step is to open each of those files and begin the counting task (explained in the next section).

When the counting process is done and finished for the entire list of files, your application should print to the standard output a **table** presenting the result of the counting **per file** with a grand total at the bottom of the table.
The totalization line should be displayed **only** if your program processed more than one input file.

In summary, your program should:

1. Read the input source and create a list of valid files to process;
2. For each file in the list, count the individual lines of code, blank lines,
   regular comments and doc comments, store this information in a convenient data structure (which one? 🤔), and;
3. Display the result of the counting into a single table, with information associated with each individual file.

Note that the display of the information inside the table should be sorted in ascending order according to the field the user requested.
The possibilities are:

- option `-s f`: sort data in ascending order by file name.
- option `-s t`: sort data in ascending order by file type.
- option `-s c`: sort data in ascending order by number of comment lines.
- option `-s d`: sort data in ascending order by number of doc comment lines.
- option `-s b`: sort data in ascending order by number of blank lines.
- option `-s s`: sort data in ascending order by number of sloc lines.
- option `-s a`: sort data in ascending order by number of all lines.

To sort the table in reverse order the user must replace `-s` by `-S` and use the same arguments described above.

### Single line of code, comments, and blank lines

A **single line of code** (sloc) is a line inside a source file that contains any C/C++ piece of code. It may be a line with just a "`{`", or a line with a "`for`" loop, for example.

A **blank line** is a line that is not inside a comment block and has no code nor any type of comment marks.

Program comments are statements that you can include in the C/C++ code to help anyone read the source code.
C++ supports single-line and multi-line comments. All characters available inside any comment are ignored by C++ compiler.
C++ comments may start with "`/*`" and end with "`*/`". For example:

```cpp
/* This is a comment */

/*
 * C++ comments can also
 * span multiple lines
 * inside a block comment.
 *
*/
```

A comment can also start with "`//`", at the beginning of a line or after a C++ statement. All that comes after the "`//`" is considered a comment.

```cpp
#include <iostream>
int main(void) {
  std::cout << "Hello world!\n"; // This is a single end line comment.
  // This is a comment that occupies an entire line.
  int x{0}; // another example.
  //

  return 0;
}
```

### Special case 1: comment marks inside other comments.

Within a "`/*`" and "`*/`" block comment, the "`//`" characters have no special meaning and must be ignored. Similarly, within a single line "`//`" comment, the "`/*`" and "`*/`" have no special meaning.
Thus, it is possible to find one kind of comment within the other kind.

For example:

```cpp
/*
  Comment out a piece of code.
  This is lost but valid! ---> /*

  for(int i{0}; i<10; i++ )
    std::cout << "(" << i+1 << "): Hello world!\n"; // prints Hello World

  // This pair of '/' are ignored.
*/
```

Note, however, that C/C++ **does not support** nested comments.
Consider the code below.

```c++
/* this is a comment /* this is not accepted */ this is outside the comment.
  This should interpreted as a line of code.
*/ this is algo a line of code only.

  for(int i{0}; i<10; i++ )
    std::cout << "(" << i+1 << "): Hello world!\n"; // prints Hello World

  // This now is a single line comment.
*/ This is a line of code now.
```

Investigate this affirmation by creating several examples of nested comments to see which ones the compiler will reject as error.

### Special case 2: comment marks in literal

Additionally, any comment mark that appears **inside** a string literal must also be ignored.

```c++
#include <iostream>
int main(void) {
  std::cout << "A comment in C++ beging with a " << "/*"
            << " and ends with a " << "*/" << std::end;
  // Another example.
  if ( str == "//") // The double slash inside if must be ignored.
    std::cout << "Found double slash\n";

  return 0;
}
```

### Special case 3: line that yields a sloc and a comment

But your program must have special care with some situations that might happen involving block comments.
For example, the code below

```c++
/* some comment */ int x = 0;
```

should be counted **both** as a comment and as a line of code.
The same goes for the code below

```c++
int x = 0; // This is both a comment and a sloc.
```

Another situation to look out for is when a multi line block comment ends, and we still have a C/C++ statement on that line.

```c++
/* one
   two
   three */ int x = 0;
```

The code above should be counted as 3 lines of comment and 1 line of code.

### Special case 4: doc comments

Your program also counts the number of documentation comment lines associated
with `doxygen`. The documentation comments (aka **docblocks**) should be
counted separately from the regular comments described in this document. To
learn how to identify `doxygen` documentation comments, refer to [this
page](https://www.doxygen.nl/manual/docblocks.html).

## System Modeling

The first step is to identify which **entities** might help you model the problem.
Here goes some suggestions of entities involved in the problem.

### The list of source files

We may need to create a **list** of input source files and their types. Filling
in this list is your program's first task. This is done based on the input
source information collected via Command Line Argument. Recall that if the
client sends a single file this list will have only a single element; whereas,
if the client provides a directory, this list may end up with various entries,
one for each valid source code file found in that directory.

### The `FileInfo` class/struct

This class or struct represents the collection of information gathered from a
source file. They are the file name, file type (C, C++, C/C++ header or C++
header), number of blank lines, number of comment lines, number of doc comment lines,
number of single lines of code, and the total number of lines of the file.

To store the file information for each input file, we will need a **dynamic
list** of `FileInfo`. This will work as the program's internal database that
stores the counting result for each source file.

### Helper functions

To help you handle a line of code, you may need to create some useful
functions. You might need, for instance, a function to convert a string to
lowercase. This is useful if you want to support files that may come with
extension in upper case. Another useful functions are one that trims out any
leading white space in a string, and another that trims out any trailing white
spaces in a string.

## Output

The output of the program is a table with the counting information for each
category of line, per source file.
In case the program has processed more than one source file, the table must
present a summation (consolidation) of counts on the last line of the table.

The example below is from a directory with three source files.

```
$ ./sloc src/core
Files processed: 3
------------------------------------------------------------------------------------------------------------------
Filename            Language      Comments        Doc Comments    Blank           Code            # of lines
------------------------------------------------------------------------------------------------------------------
core/main_.cpp      C++           11 (16.4%)      32 (47.8%)      7 (10.4%)       17 (25.4%)      64
core/cowsay.h       C/C++ header  29 (13.9%)      64 (30.6%)      20 (9.6%)       96 (45.9%)      166
core/cowsay.cpp     C++           142 (27.3%)     23 (4.4%)       26 (5.0%)       330 (63.3%)     465
------------------------------------------------------------------------------------------------------------------
SUM                               182             119             53              443             695
------------------------------------------------------------------------------------------------------------------
```

Note that for each category of line you must show the number of lines **and**
the percentage that this quantity represents in relation to the total number of
lines of the source file. The percentage must be presented enclosed in
parentheses and must have one digit after the point.  
All the fields must be left aligned.
All the columns, but the first, must have a fixed width.
The first column must grow in width to accommodate the largest path+file name found in the list.

### Error output

There are, at least, three possible input errors that your program must address:

1. The client provides either a non-existing source file or a non-existing directory.

```
$ ./build/sloc file.cpp
  Sorry, unable to read "file.cpp".
$ ./build/sloc source
  Sorry, unable to read "source".
```

2. The client provides an existing file, but it is not a supported source file.

```
$ ./build/sloc file.tex
  Sorry, ".tex" files are not supported at this time.
```

3. The client provides an existing directory that does not contain any supported source file.

```
$ ./build/sloc src
  Sorry, unable to find any supported source file inside directory "src".
```

In any of these cases just described the program must print the proper error
message and exit the program **without** printing an empty table.

Other possible errors your team must deal with is the wrong use of the configuration options and their arguments.
For example, it is an error to provide an invalid sorting field, or to provide an invalid option.

If you identify another error situation while developing the project feel free to treat it accordingly.
The important point here is that your program **must not crash** in any circumstance.

## Assignment Grading

This assignment has a total of **120 points**. Bellow is a table with the
description of the item under consideration and the assigned grade _if the item
is totally correct_. Partial grade will be considered if the item is not fully
correct.

| Item                                                | Value (points) |
| --------------------------------------------------- | :------------: |
| Read one or multiple source file                    |       10       |
| Read a list of files from a directory               |       10       |
| Read a list of files recursively from a directory   |       8        |
| Show help as requested `-h`                         |       4        |
| Treat error situations                              |       10       |
| Show correct number of blank lines                  |       5        |
| Show correct number of comment lines                |       10       |
| Show correct number of doc comment lines            |       10       |
| Show correct number of code lines                   |       10       |
| Show correct number of total lines                  |       5        |
| Show the type of source file                        |       5        |
| Show percentages as requested                       |       10       |
| Show consolidation at the bottom of a table         |       5        |
| Table is formatted as requested                     |       8        |
| The project contains at least one struct or classes |       5        |
| Overall evaluation of the program                   |       5        |

The following situations may take credits out of your assignment, if they happen during the evaluation process:

| Item                                              | Value (points) |
| ------------------------------------------------- | :------------: |
| Compiling and/or runtime errors                   |       -5       |
| Missing code documentation in Doxygen style       |      -10       |
| Memory leak                                       |      -10       |
| [`author.md`](docs/author.md) file not filled out |      -10       |
| Program crashes while testing                     |       -5       |

The [`author.md`](docs/author.md) file should contain a brief description of the project, and how to run it. It also should describe possible errors, limitations, or issues found. Do not forget to include the author name!

## Extra credits

You may earn extra credits if:

- Your program support extra languages, **5 pts** per language (max **10 pts**).

# Authorship and Collaboration Policy

This is a pair assignment. You may work in a pair or alone. If you work as a
pair, comment both members' names atop every code file, and try to balance
evenly the workload.

Any team may be called for an interview. The purpose of the interview is
twofold: to confirm the authorship of the assignment and to identify the
workload assign to each member. During the interview, any team member should be
capable of explaining any piece of code, even if he or she has not written that
particular piece of code. After the interview, the assignment's credits may be
re-distributed to better reflect the true contribution of each team member.

The cooperation among students is strongly encouraged. It is accepted the open
discussion of ideas or development strategies. Notice, however, that this type
of interaction should not be understood as a free permission to copy and use
somebody else's code. This is may be interpreted as plagiarism.

Trabalhos plagiados ou realizados com IA generativa (ou qualquer variação) receberão nota **zero** automaticamente,
independente de quem seja o verdadeiro autor dos trabalhos infratores.

Any two (or more) programs deemed as plagiarism or created with generative AI
(or any variation thereof) will automatically receive **zero** credits,
regardless of the real authorship of the programs involved in the case. If your
project uses a (small) piece of code from someone else's, please provide proper
acknowledgment in the README file.

# Work Submission

Both team members must submit a single zip file containing the entire project
via SIGAA before the deadline. This should be done only via the proper link in
the SIGAA's virtual class.
**Submission via email shall be ignored!**

The same project must, also, be committed before the same deadline to the
proper GitHub repository created by the GitHub Classroom assignment.

Any commits made after the deadline will make your project be evaluated as a
**later submission**. This means that your project's grade will probably be
reduced, even if the project is completely correct.

Remember to remove all the executable files (i.e. the `build` folder) from your
project before handing in your work.

---

&copy; DIMAp/UFRN 2024-2025.
