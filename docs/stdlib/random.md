# random

## Table of Contents

- [Functions](#functions)
  - [random](#random)
  - [randomseed](#randomseed)
  - [choice](#choice)

## Functions

### random

```xylia
func random(args: any) -> any
```

Returns a random number or integer depending on the arguments provided.
- `random()` → float in range `[0, 1)`
- `random(n)` → integer in range `[1, n]`
- `random(a, b)` → integer in range `[a, b]`
Raises an error if too many arguments are passed.
**Examples:**
```xylia
random()        -- 0.73124
random(10)      -- 7
random(5, 15)   -- 12
```

**Parameters:**

- `args` (`any`)

**Returns:**

`any` 

### randomseed

```xylia
func randomseed(seed: number)
```

Seeds the random number generator with the given `seed`.
This allows deterministic random sequences for testing or reproducibility.
**Example:**
```xylia
randomseed(42)
print(random())   -- always the same value for seed 42
```

**Parameters:**

- `seed` (`number`)

### choice

```xylia
func choice(args: any) -> any
```

Returns a random element from a list, vector, or from the provided arguments.
If one argument is given and it is a sequence (like a `vector` or `list`),
a random element is chosen from that sequence.
If multiple arguments are passed, one of them is chosen randomly.
**Examples:**
```xylia
choice([1, 2, 3, 4])       -- 2
choice("a", "b", "c")      -- "b"
choice(Vector(0:10))       -- random element from vector
```

**Parameters:**

- `args` (`any`)

**Returns:**

`any` 

