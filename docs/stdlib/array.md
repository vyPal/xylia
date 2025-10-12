# array

## Table of Contents

- [Classes](#classes)
  - [Array](#Array)

## Classes

## Array

The `Array` class provides a simple wrapper around builtin arrays.
It adds size tracking and a few utility methods for resizing and
converting arrays to string form.

### Methods

### init

```xylia
func init(size: number) -> Array
```

Initializes a new `Array` with the given size.
The internal data is a builtin array of that size.

**Parameters:**

- `size` (`number`)

**Returns:**

`Array` 

### size

```xylia
func size() -> number
```

Returns the current size of the array.

**Returns:**

`number` 

### to_string

```xylia
func to_string() -> string
```

Returns a string representation of the array’s contents.

**Returns:**

`string` 

### resize

```xylia
func resize(new_size: number)
```

Resizes the array to `new_size`.
The contents are preserved up to the new size if possible.

**Parameters:**

- `new_size` (`number`)

### operator []

```xylia
func operator [](index: number) -> Any
```

Returns the element at the given index.
Raises an error if the index is out of range.

**Parameters:**

- `index` (`number`)

**Returns:**

`Any` 

### operator []=

```xylia
func operator []=(index: number, value: Any)
```

Sets the element at the given index to `value`.
Raises an error if the index is out of range.

**Parameters:**

- `index` (`number`)
- `value` (`Any`)

