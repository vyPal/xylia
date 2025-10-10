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
| [`Array::init`](#arrayinit) | method | Initializes a new array with the given size. |
| [`Array::size`](#arraysize) | method | Returns the current size of the array. |
| [`Array::to_string`](#arrayto_string) | method | Returns a string representation of the array contents. |
| [`Array::resize`](#arrayresize) | method | Resizes the array to `new_size`, truncating or extending it as needed. |
| [`Array::[]`](#array) | operator | Accesses the value at the specified `index`. |
| [`Array::[]=`](#array) | operator | Sets the element at the specified `index` to `value`. |

---

### `Array` <a name="array"></a>

A wrapper type for builtin arrays, providing common operations and size management.

#### `Array::init` <a name="arrayinit"></a>

```xylia
init(size: number) -> Array
```

Initializes a new array with the given size.

#### `Array::size` <a name="arraysize"></a>

```xylia
size() -> number
```

Returns the current size of the array.

#### `Array::to_string` <a name="arrayto_string"></a>

```xylia
to_string() -> string
```

Returns a string representation of the array contents.

#### `Array::resize` <a name="arrayresize"></a>

```xylia
resize(new_size: number)
```

Resizes the array to `new_size`, truncating or extending it as needed.

#### `Array::[]` <a name="array"></a>

```xylia
[](index: number) -> any
```

Accesses the value at the specified `index`.

#### `Array::[]=` <a name="array"></a>

```xylia
[]=(index: number, value: any)
```

Sets the element at the specified `index` to `value`.

