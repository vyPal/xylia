# Module: `io`

> **Header:** `io`  
> **Import with:** `let io = import("io");`

---

## Overview

The `io` module provides functions and classes for input and output operations.
It includes console printing, formatted output, user input, and file handling.

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`stdin : File`](#stdin-:-file) | variable | Standard input stream. |
| [`stdout : File`](#stdout-:-file) | variable | Standard output stream. |
| [`stderr : File`](#stderr-:-file) | variable | Standard error stream. |
| [`print`](#print) | function | Prints arguments to standard output without a newline. |
| [`println`](#println) | function | Prints arguments to standard output followed by a newline. |
| [`printf`](#printf) | function | Prints formatted output to standard output. |
| [`input`](#input) | function | Reads input from standard input, optionally displaying a prompt. |
| [`File`](#file) | class | Represents an open file handle. |
| [`File::init`](#file-init) | method | Opens a file with the specified mode. |
| [`File::close`](#file-close) | method | Closes the file handle. |
| [`File::read`](#file-read) | method | Reads the file contents. |
| [`File::write`](#file-write) | method | Writes content to the file. |
| [`File::<<`](#file-<<) | operator | Writes a value to the file (using its string representation). |

---

### `stdin : File`

Standard input stream.

### `stdout : File`

Standard output stream.

### `stderr : File`

Standard error stream.

### `print`

**Signature:** `print(args: any[])`  

Prints arguments to standard output without a newline.

### `println`

**Signature:** `println(args: any[])`  

Prints arguments to standard output followed by a newline.

### `printf`

**Signature:** `printf(args: any[])`  

Prints formatted output to standard output.
Requires at least one argument.

### `input`

**Signature:** `input(prompt?: string) -> string`  

Reads input from standard input, optionally displaying a prompt.

### `File`

Represents an open file handle.

### File::`init`

**Signature:** `init(path: string, mode: string) -> File`  

Opens a file with the specified mode.

### File::`close`

**Signature:** `close()`  

Closes the file handle.

### File::`read`

**Signature:** `read() -> string`  

Reads the file contents.

### File::`write`

**Signature:** `write(content: string)`  

Writes content to the file.

### File::`<<`

**Signature:** `<<(value: any)`  

Writes a value to the file (using its string representation).

