# SHELL

This project implements a minimalist `*nix` shell interpreter in C, providing an interactive command prompt and basic scripting capabilities. The shell supports core functionalities needed for command execution and scripting in a straightforward and clear design.

---

## Features

- **Interactive Command Prompt:** Displays a colorized prompt that shows the current username and working directory.
- **Command Tokenization:** Parses input lines into tokens with support for quoted strings and escaped characters.
- **Variable Expansion:** Expands environment variables prefixed with `$` inside commands.
- **Builtin Commands:** Supports essential built in commands such as `cd` (change directory), `exit` (exit shell), and `export` (environment variable assignment).
- **Input/Output Redirection:** Handles file input `<`, output truncation `>`, and output append `>>`.
- **Command Chaining:** Supports sequential command execution using `;`, and conditional execution with logical AND `&&` and logical OR `||`.
- **Simple Conditional Expressions:** Evaluates POSIX style test expressions enclosed in `[ ... ]` for flow control within command chains.
- **Signal Handling:** Graceful handling of interrupts (e.g., Ctrl+C) to prevent shell exit and maintain prompt continuity.

---

## Overview

The shell continuously reads input lines either interactively or from a script, tokenizes them, performs variable expansions, and interprets command sequences. Commands are executed either as builtins or as external programs via fork/exec, with optional I/O redirection applied.

Conditionals embedded inside `[ ... ]` expressions are parsed and evaluated to zero/non-zero status codes to enable command chaining logic. The shell supports typical file attribute checks and comparison operators.

The implementation emphasizes clarity and simplicity, making it suitable for educational exploration of shell fundamentals, command parsing, and process control.

---

## Limitations

- **No Multi Line or Complex Control Structures:** The shell does not currently support multi-line syntax constructs like `if-then-else-fi`, loops (`for`, `while`), or functions.
- **No Job Control:** Background execution (`&`), job suspension, or job monitoring features are absent.
- **Limited Parsing:** Tokenization handles basic quoting and escaping but lacks advanced features like here documents, command substitution, or brace expansion.
- **Minimal Error Reporting:** Syntax and execution errors are reported plainly without detailed diagnostics.
- **No Shell Builtin Command Environment:** Does not preserve shell specific states such as local variables, advanced aliases, or shell options.
- **No Shell Script Files:** While it can execute commands from files via standard input, no dedicated script processing or file sourcing is implemented.
- **No Command History or Line Editing:** The prompt does not support command history navigation or advanced line editing.

---

## Possible Improvements

- **Multi Line Script Parsing:** Add support for structured shell scripting constructs, including conditional blocks (`if`, `elif`, `else`), loops, and case statements.
- **Job Control and Background Processes:** Implement command backgrounding, job suspension, and signal handling for job control.
- **Advanced Parsing Features:** Incorporate here-documents, command substitution, arithmetic expansion, and brace expansion.
- **Improved Error Handling:** Enhance diagnostics with descriptive error messages, line/column info, and recovery strategies.
- **Built-in Shell Features:** Extend built in command set, support shell variables, aliases, functions, and shell option flags.
- **Script File Support:** Allow direct execution of shell script files with parsing and environment management.
- **User Interaction Enhancements:** Add command history, readline support, auto completion, and syntax highlighting for improved usability.
- **Performance Optimizations:** Optimize memory management and system calls for faster shell responsiveness.
- **Portability and Standards Compliance:** Improve compatibility with POSIX shell standards and across different `*nix` systems.

---


