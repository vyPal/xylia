# vector

## Table of Contents

- [Functions](#functions)
  - [operator +](#operator +)
  - [operator ==](#operator ==)
- [Classes](#classes)
  - [Vector](#Vector)

## Functions

### `operator +`

```xylia
func operator +(other: Vector) -> Vector
```

**Parameters:**

- `other` (`Vector`)

**Returns:** `Vector` 

### `operator ==`

```xylia
func operator ==(other: Vector) -> Vector
```

**Parameters:**

- `other` (`Vector`)

**Returns:** `Vector` 

## Classes

## Vector

### Methods

### `Vector::init`

```xylia
func Vector::init(args: Any) -> Vector
```

**Parameters:**

- `args` (`Any`)

**Returns:** `Vector` 

### `Vector::size`

```xylia
func Vector::size() -> number
```

**Returns:** `number` 

### `Vector::push`

```xylia
func Vector::push(args: Any)
```

**Parameters:**

- `args` (`Any`)

### `Vector::pop`

```xylia
func Vector::pop() -> Any
```

**Returns:** `Any` 

### `Vector::insert`

```xylia
func Vector::insert(index: number, value: Any)
```

**Parameters:**

- `index` (`number`)
- `value` (`Any`)

### `Vector::remove`

```xylia
func Vector::remove(index: number) -> Any
```

**Parameters:**

- `index` (`number`)

**Returns:** `Any` 

### `Vector::to_string`

```xylia
func Vector::to_string() -> string
```

**Returns:** `string` 

### `Vector::operator []`

```xylia
func Vector::operator [](index: number) -> Any
```

**Parameters:**

- `index` (`number`)

**Returns:** `Any` 

### `Vector::operator []=`

```xylia
func Vector::operator []=(index: number, value: Any)
```

**Parameters:**

- `index` (`number`)
- `value` (`Any`)

### `Vector::operator [`

```xylia
func Vector::operator [(from: number, to: number) -> Vector
```

**Parameters:**

- `from` (`number`)
- `to` (`number`)

**Returns:** `Vector` 

### `Vector::operator [`

```xylia
func Vector::operator [(from: number, to: number, value: Any)
```

**Parameters:**

- `from` (`number`)
- `to` (`number`)
- `value` (`Any`)

