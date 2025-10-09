# Module: `io`

> **Header:** `"io"`  
> **Imports with:** `let io = import("io");`

---

## Overview

The `io` module provides facilities for input and output operations, including console printing, formatted output, reading user input, and file handling.

It defines:
- Global printing functions such as [`print`](#print), [`println`](#println), and [`printf`](#printf)
- Stream-oriented functions (`fprint`, `fprintln`, `fprintf`)
- Interactive input via [`input`](#input)
- A [`File`](#file-class) class for file I/O
- Predefined standard streams: [`stdin`](#stdin), [`stdout`](#stdout), and [`stderr`](#stderr)

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`print`](#print) | function | Prints arguments to standard output without newline |
| [`println`](#println) | function | Prints arguments followed by a newline |
| [`printf`](#printf) | function | Formatted print to standard output |
| [`input`](#input) | function | Reads a line from standard input |
| [`fprint`](#fprint) | function | Prints to a specified file or stream |
| [`fprintln`](#fprintln) | function | Prints with newline to a specified stream |
| [`fprintf`](#fprintf) | function | Formatted print to a specified stream |
| [`File`](#file-class) | class | Wrapper for file handles |
| [`stdin`](#stdin) | variable | Standard input stream |
| [`stdout`](#stdout) | variable | Standard output stream |
| [`stderr`](#stderr) | variable | Standard error stream |

---

## Functions

### `print(args[])`

```xylia
func print(args[])
````

Prints all given arguments separated by space to standard output without adding a newline.

**Example:**

```xylia
let io = import("io");
io::print("Hello,", "World");
```

---

### `println(args[])`

```xylia
func println(args[])
```

Prints all arguments separated by space followed by a newline.

**Example:**

```xylia
let io = import("io");
io::println("Hello, World!");
```

---

### `printf(args[])`

```xylia
func printf(args[])
```

Formatted print using a format string followed by values.

**Constraints:**

* Must be called with at least one argument (the format string).

**Example:**

```xylia
let io = import("io");
io::printf("Value: {}\n", 42);
```

---

### `input(prompt[])`

```xylia
func input(prompt[])
```

Reads a single line of text from standard input.
If a prompt string is provided, it is printed before reading.

**Parameters:**

| Name     | Type                | Description                  |
| :------- | :------------------ | :--------------------------- |
| `prompt` | `string` (optional) | Text to display before input |

**Example:**

```xylia
let io = import("io");
let name = io::input("Enter your name: ");
io::println("Hello,", name);
```

---

### `fprint(stream, args[])`

```xylia
func fprint(stream, args[])
```

Prints arguments separated by space to the specified file or stream without adding a newline.

If `stream` is an instance of `File`, its internal handle is used.
If `stream` is a raw file handle, it is used directly.

**Example:**

```xylia
let io = import("io");
io::fprint(io::stdout, "Hello, ");
io::fprint(io::stdout, "World!");
```

---

### `fprintln(stream, args[])`

```xylia
func fprintln(stream, args[])
```

Prints arguments separated by space followed by a newline to the specified stream.

**Example:**

```xylia
let io = import("io");
io::fprintln(io::stderr, "Error:", "Something went wrong!");
```

---

### `fprintf(stream, args[])`

```xylia
func fprintf(stream, args[])
```

Formatted print to a specific stream or file.
Requires at least one argument (the format string).

**Example:**

```xylia
let io = import("io");
io::fprintf(io::stdout, "Count: {}\n", 10);
```

---

## Types

### `File` class

**Kind:** class
**Description:**
Encapsulates a file handle and provides methods for reading, writing, and closing files.

Files can be opened by path and mode, or wrapped around an existing file handle.

---

#### **Constructor**

```xylia
func init(args[])
```

Initializes a `File` instance.

| Usage              | Description                                                        |
| :----------------- | :----------------------------------------------------------------- |
| `File(file)`       | Wraps an existing file handle                                      |
| `File(path, mode)` | Opens a new file at `path` using `mode` (e.g. `"r"`, `"w"`, `"a"`) |

**Example:**

```xylia
let io = import("io");

let file = io::File("output.txt", "w");
file.write("Hello from Xylia!\n");
file.close();
```

---

#### **Methods**

| Method                               | Signature                          | Description                                   |
| :----------------------------------- | :--------------------------------- | :-------------------------------------------- |
| [`close()`](#close)                  | `func close()`                     | Closes the file handle                        |
| [`read()`](#read)                    | `func read() -> string`            | Reads entire contents of the file             |
| [`write(content)`](#write)           | `func write(content: string)`      | Writes text to the file                       |
| [`operator << (other)`](#operator--) | `operator << (other: any) -> File` | Writes any value to the file and returns self |

---

##### `close()`

Closes the file if open.

```xylia
file.close();
```

---

##### `read()`

Reads the entire file into a string.
Raises an assertion if the file is already closed.

```xylia
let content = file.read();
```

---

##### `write(content)`

Writes a string to the file.
Raises an assertion if the file is closed.

```xylia
file.write("Some text");
```

---

##### `operator << (other)`

Overloaded output operator.
Writes either `other.to_string()` if available, or the stringified value of `other`.

Returns the file instance for chaining.

**Example:**

```xylia
file << "Hello, " << "World!\n";
```

---

## Standard Streams

### `stdin`

```xylia
let stdin = File(__builtin___stdin());
```

Standard input stream — used by [`input()`](#input).
Can be used directly to read raw input:

```xylia
let io = import("io");
let data = io::stdin.read();
```

---

### `stdout`

```xylia
let stdout = File(__builtin___stdout());
```

Standard output stream — used by [`print()`](#print) and [`println()`](#println).
You can write directly to it:

```xylia
io::stdout.write("Hello via stdout\n");
```

---

### `stderr`

```xylia
let stderr = File(__builtin___stderr());
```

Standard error stream — used for diagnostics and errors.

```xylia
io::stderr.write("Warning: something went wrong!\n");
```

---

## Examples

```xylia
let io = import("io");

# Simple printing
io::println("Hello, Xylia!");

# Formatted output
io::printf("Number: {}\n", 42);

# User input
let name = io::input("Your name: ");
io::println("Hi,", name);

# File I/O
let file = io::File("example.txt", "w");
file.write("Xylia loves files.\n");
file.close();

# Using operator <<
let out = io::File("chain.txt", "w");
out << "Hello" << " " << "World!\n";
out.close();
```

**Output:**

```
Hello, Xylia!
Number: 42
Your name: Samuel
Hi, Samuel
```

---

## See Also

* [strings](./strings.md)
* [utility](./utility.md)
* [test](./test.md)
