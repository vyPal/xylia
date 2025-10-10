# Module: `array`

> **Header:** `array`  
> **Import with:** `let array = import("array");`

---

## Overview

The `array` module provides a simple wrapper around the builtin array type.
It offers a higher-level interface for fixed-size collections of values, as well as
utilities for resizing and converting to string representations.

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`Array`](#array) | class | A wrapper type for builtin arrays, providing common operations and size management. |
| [`Array::init`](#array::init) | method | Initializes a new array with the given size. |
| [`Array::size`](#array::size) | method | Returns the current size of the array. |
| [`Array::to_string`](#array::to_string) | method | Returns a string representation of the array contents. |
| [`Array::resize`](#array::resize) | method | Resizes the array to `new_size`, truncating or extending it as needed. |
| [`Array::[]`](#array::[]) | operator | Accesses the value at the specified `index`. |
| [`Array::[]=`](#array::[]=) | operator | Sets the element at the specified `index` to `value`. |

---

### `Array`

A wrapper type for builtin arrays, providing common operations and size management.

### `Array::init`

**Signature:** `init(size: number) -> Array`  

Initializes a new array with the given size.

### `Array::size`

**Signature:** `size() -> number`  

Returns the current size of the array.

### `Array::to_string`

**Signature:** `to_string() -> string`  

Returns a string representation of the array contents.

### `Array::resize`

**Signature:** `resize(new_size: number)`  

Resizes the array to `new_size`, truncating or extending it as needed.

### `Array::[]`

**Signature:** `[](index: number) -> any`  

Accesses the value at the specified `index`.

### `Array::[]=`

**Signature:** `[]=(index: number, value: any)`  

Sets the element at the specified `index` to `value`.

