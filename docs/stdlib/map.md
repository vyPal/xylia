# Module: `map`

> **Header:** `map`  
> **Import with:** `let map = import("map");`

---

## Overview

The `map` module provides a hash map implementation with automatic resizing
and collision handling. It allows storing key-value pairs with efficient
lookup, insertion, and deletion.

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`Map`](#map) | class | A hash map type that wraps a low-level array with custom collision resolution. |
| [`Map::init`](#mapinit) | method | Initializes a new empty map. |
| [`Map::get_index`](#mapget_index) | method | Computes the hash index for the given key. |
| [`Map::resize_table`](#mapresize_table) | method | Resizes the internal table to the specified capacity. |
| [`Map::insert`](#mapinsert) | method | Inserts a key-value pair into the map, replacing the value if the key exists. |
| [`Map::delete`](#mapdelete) | method | Deletes a key from the map. Returns `true` if the key was removed, `false` if it was not found. |
| [`Map::get`](#mapget) | method | Retrieves the value associated with the key, or `nil` if the key does not exist. |
| [`Map::[]`](#map) | operator | Accesses the value corresponding to `key`. |
| [`Map::[]=`](#map) | operator | Sets the value for the given key in the map. |

---

### `Map` <a name="map"></a>

A hash map type that wraps a low-level array with custom collision resolution.

#### `Map::init` <a name="mapinit"></a>

```xylia
init() -> Map
```

Initializes a new empty map.

#### `Map::get_index` <a name="mapget_index"></a>

```xylia
get_index(key) -> number
```

Computes the hash index for the given key.

#### `Map::resize_table` <a name="mapresize_table"></a>

```xylia
resize_table(new_capacity: number)
```

Resizes the internal table to the specified capacity.

#### `Map::insert` <a name="mapinsert"></a>

```xylia
insert(key, value)
```

Inserts a key-value pair into the map, replacing the value if the key exists.

#### `Map::delete` <a name="mapdelete"></a>

```xylia
delete(key) -> bool
```

Deletes a key from the map. Returns `true` if the key was removed, `false` if it was not found.

#### `Map::get` <a name="mapget"></a>

```xylia
get(key) -> any
```

Retrieves the value associated with the key, or `nil` if the key does not exist.

#### `Map::[]` <a name="map"></a>

```xylia
[](key) -> any
```

Accesses the value corresponding to `key`.

#### `Map::[]=` <a name="map"></a>

```xylia
[]=(key, value)
```

Sets the value for the given key in the map.

