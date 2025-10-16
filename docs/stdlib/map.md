# map

## Table of Contents

- [Functions](#functions)
  - [insert](#insert)
  - [delete](#delete)
  - [get](#get)
  - [operator []](#operator [])
  - [operator []=](#operator []=)
- [Classes](#classes)
  - [Map](#Map)

## Functions

### `insert`

```xylia
func insert(key: Any, value: Any)
```

**Parameters:**

- `key` (`Any`)
- `value` (`Any`)

### `delete`

```xylia
func delete(key: Any) -> bool
```

**Parameters:**

- `key` (`Any`)

**Returns:** `bool` 

### `get`

```xylia
func get(key: Any) -> Any
```

**Parameters:**

- `key` (`Any`)

**Returns:** `Any` 

### `operator []`

```xylia
func operator [](key: Any) -> Any
```

**Parameters:**

- `key` (`Any`)

**Returns:** `Any` 

### `operator []=`

```xylia
func operator []=(key: Any, value: Any)
```

**Parameters:**

- `key` (`Any`)
- `value` (`Any`)

## Classes

## Map

### Methods

### `Map::init`

```xylia
func Map::init() -> Map
```

**Returns:** `Map` 

### `Map::get_index`

```xylia
func Map::get_index(key: Any) -> number
```

**Parameters:**

- `key` (`Any`)

**Returns:** `number` 

### `Map::resize_table`

```xylia
func Map::resize_table(new_capacity: number)
```

**Parameters:**

- `new_capacity` (`number`)

