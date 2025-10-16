# io

## Table of Contents

- [Functions](#functions)
  - [print](#print)
  - [println](#println)
  - [printf](#printf)
  - [input](#input)
  - [read](#read)
  - [write](#write)
  - [operator <<](#operator <<)
  - [fprint](#fprint)
  - [fprintln](#fprintln)
  - [fprintf](#fprintf)
- [Variables](#variables)
  - [stdin](#stdin)
  - [stdout](#stdout)
  - [stderr](#stderr)
- [Classes](#classes)
  - [File](#File)

## Functions

### `print`

```xylia
func print(args: Any)
```

**Parameters:**

- `args` (`Any`)

### `println`

```xylia
func println(args: Any)
```

**Parameters:**

- `args` (`Any`)

### `printf`

```xylia
func printf(args: Any)
```

**Parameters:**

- `args` (`Any`)

### `input`

```xylia
func input(prompt: Any) -> string
```

**Parameters:**

- `prompt` (`Any`)

**Returns:** `string` 

### `read`

```xylia
func read() -> string
```

**Returns:** `string` 

### `write`

```xylia
func write(content: string)
```

**Parameters:**

- `content` (`string`)

### `operator <<`

```xylia
func operator <<(other: Any)
```

**Parameters:**

- `other` (`Any`)

### `fprint`

```xylia
func fprint(stream: File, args: Any)
```

**Parameters:**

- `stream` (`File`)
- `args` (`Any`)

### `fprintln`

```xylia
func fprintln(stream: File, args: Any)
```

**Parameters:**

- `stream` (`File`)
- `args` (`Any`)

### `fprintf`

```xylia
func fprintf(stream: File, args: Any)
```

**Parameters:**

- `stream` (`File`)
- `args` (`Any`)

## Variables

### stdin

**Type:** `File`

### stdout

**Type:** `File`

### stderr

**Type:** `File`

## Classes

## File

### Methods

### `File::init`

```xylia
func File::init(args: Any) -> File
```

**Parameters:**

- `args` (`Any`)

**Returns:** `File` 

### `File::close`

```xylia
func File::close()
```

