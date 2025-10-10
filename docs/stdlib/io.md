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
| [`stdin : File`](#stdin--file) | variable | Standard input stream. |
| [`stdout : File`](#stdout--file) | variable | Standard output stream. |
| [`stderr : File`](#stderr--file) | variable | Standard error stream. |
| [`print`](#print) | function | Prints arguments to standard output without a newline. |
| [`println`](#println) | function | Prints arguments to standard output followed by a newline. |
| [`printf`](#printf) | function | Prints formatted output to standard output. |
| [`input`](#input) | function | Reads input from standard input, optionally displaying a prompt. |
| [`File`](#file) | class | Represents an open file handle. |
| [`File::init`](#fileinit) | method | Opens a file with the specified mode. |
| [`File::close`](#fileclose) | method | Closes the file handle. |
| [`File::read`](#fileread) | method | Reads the file contents. |
| [`File::write`](#filewrite) | method | Writes content to the file. |
| [`File::<<`](#file) | operator | Writes a value to the file (using its string representation). |

---

### `stdin : File` <a name="stdin--file"></a>

Standard input stream.

### `stdout : File` <a name="stdout--file"></a>

Standard output stream.

### `stderr : File` <a name="stderr--file"></a>

Standard error stream.

### `print` <a name="print"></a>

```xylia
print(args: any[])
```

Prints arguments to standard output without a newline.

### `println` <a name="println"></a>

```xylia
println(args: any[])
```

Prints arguments to standard output followed by a newline.

### `printf` <a name="printf"></a>

```xylia
printf(args: any[])
```

Prints formatted output to standard output.
Requires at least one argument.

### `input` <a name="input"></a>

```xylia
input(prompt?: string) -> string
```

Reads input from standard input, optionally displaying a prompt.

### `File` <a name="file"></a>

Represents an open file handle.

#### `File::init` <a name="fileinit"></a>

```xylia
init(path: string, mode: string) -> File
```

Opens a file with the specified mode.

#### `File::close` <a name="fileclose"></a>

```xylia
close()
```

Closes the file handle.

#### `File::read` <a name="fileread"></a>

```xylia
read() -> string
```

Reads the file contents.

#### `File::write` <a name="filewrite"></a>

```xylia
write(content: string)
```

Writes content to the file.

#### `File::<<` <a name="file"></a>

```xylia
<<(value: any)
```

Writes a value to the file (using its string representation).

