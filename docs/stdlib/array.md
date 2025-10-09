# Module: `array`

> **Header:** `"array"`  
> **Imports with:** `let array = import("array");`

---

## Overview

The `array` module provides a wrapper around Xylia's built-in array type.  
While Xylia includes a low-level `__builtin___array` type, this module offers a safer, higher-level `Array` class with convenience methods and operator overloads.

Arrays, vectors, and files are built on internal data structures provided by the runtime.  
It's generally recommended to use these wrapper classes instead of directly manipulating built-ins.

---

## Contents

| Symbol                  | Type  | Description                           |
| :---------------------- | :---- | :------------------------------------ |
| [`Array`](#array-class) | class | Wrapper around the builtin array type |

---

## Types

### `Array` class

**Kind:** class  
**Description:**  
Represents a dynamically allocated contiguous collection of elements.  
Provides methods for resizing, indexing, and string conversion.

---

#### **Constructor**

```xylia
func init(size)
````

Creates a new array with the specified initial size.

| Parameter | Type  | Description                    |
| :-------- | :---- | :----------------------------- |
| `size`    | `int` | Number of elements to allocate |

**Example:**

```xylia
let array = import("array");
let a = array::Array(4);
```

---

#### **Methods**

| Method                        | Signature                    | Description                                                         |
| :---------------------------- | :--------------------------- | :------------------------------------------------------------------ |
| [`size()`](#size)             | `func size() -> int`         | Returns the current number of elements                              |
| [`to_string()`](#to_string)   | `func to_string() -> string` | Returns a string representation of the array                        |
| [`resize(new_size)`](#resize) | `func resize(new_size: int)` | Changes the array size, preserving existing elements where possible |

---

##### `size()`

```xylia
func size() -> int
```

Returns the number of elements in the array.

**Example:**

```xylia
let array = import("array");
let a = array::Array(3);
io::println(a.size());  # prints: 3
```

---

##### `to_string()`

```xylia
func to_string() -> string
```

Returns a string representation of the array contents.
This is primarily intended for debugging and display.

**Example:**

```xylia
let array = import("array");
let a = array::Array(3);
io::println(a.to_string());  # prints something like: <nil, nil, nil>
```

---

##### `resize(new_size)`

```xylia
func resize(new_size: int)
```

Changes the size of the array.
If the new size is larger, new elements are nil-initialized.
If smaller, elements beyond the new size are discarded.

| Parameter  | Type  | Description                        |
| :--------- | :---- | :--------------------------------- |
| `new_size` | `int` | The desired new number of elements |

**Example:**

```xylia
let array = import("array");
let a = array::Array(2);
a.resize(5);
io::println(a.size());  # prints: 5
```

---

#### **Operators**

| Operator | Description               |
| :------- | :------------------------ |
| `[]`     | Accesses element at index |
| `[]=`    | Assigns element at index  |

##### `operator [] (index)`

```xylia
operator [] (index: int) -> any
```

Returns the element at the given index.

##### `operator []= (index, value)`

```xylia
operator []= (index: int, value: any)
```

Assigns a new value at the given index.

**Example:**

```xylia
let array = import("array");
let a = array::Array(3);
a[0] = 42;
io::println(a[0]);  # prints: 42
```

---

## Examples

```xylia
let io = import("io");
let array = import("array");

let nums = array::Array(3);
nums[0] = 10;
nums[1] = 20;
nums[2] = 30;

io::println("Array size: " + nums.size());
io::println("Contents: " + nums.to_string());

nums.resize(5);
io::println("Resized: " + nums.to_string());
```

**Output:**

```
Array size: 3
Contents: <10, 20, 30>
Resized: <10, 20, 30, nil, nil>
```

---

## See Also

* [vector](./vector.md)
* [map](./map.md)
* [utility](./utility.md)
