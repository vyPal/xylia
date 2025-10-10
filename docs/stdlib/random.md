# Module: `random`

> **Header:** `random`  
> **Import with:** `let random = import("random");`

---

## Overview

The `random` module provides functions for generating pseudo-random numbers,
seeding the random generator, and selecting random elements from collections.

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`random`](#random) | function | Generates a random number. Usage: |
| [`randomseed`](#randomseed) | function | Seeds the random number generator with the given `seed`. |
| [`choice`](#choice) | function | Selects a random element from a collection or arguments list.   |

---

### `random` <a name="random"></a>

```xylia
random(args: any[])
```

Generates a random number. Usage:
- `random()` → float between 0 and 1
- `random(n: number)` → integer between 1 and n
- `random(a: number, b: number)` → integer between a and b

### `randomseed` <a name="randomseed"></a>

```xylia
randomseed(seed: number)
```

Seeds the random number generator with the given `seed`.

### `choice` <a name="choice"></a>

```xylia
choice(args: any[])
```

Selects a random element from a collection or arguments list.  
- If given a vector or list: returns a random element  
- Otherwise: returns a random argument from the list

