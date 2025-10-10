# Module: `math`

> **Header:** `math`  
> **Import with:** `let math = import("math");`

---

## Overview

The `math` module provides common mathematical constants and functions
for numerical operations, trigonometry, logarithms, and rounding.

---

## Contents

| Symbol | Type | Description |
|:--------|:------|:-------------|
| [`PI = 3.14159265358979323846`](#pi--314159265358979323846) | variable |  |
| [`E = 2.7182818284590452354`](#e--27182818284590452354) | variable |  |
| [`NAN = 0 / 0`](#nan--0--0) | variable |  |
| [`INF = 1 / 0`](#inf--1--0) | variable |  |
| [`abs`](#abs) | function | Returns the absolute value of `x`. |
| [`min`](#min) | function | Returns the minimum value from a list of arguments. |
| [`max`](#max) | function | Returns the maximum value from a list of arguments. |
| [`clamp`](#clamp) | function | Clamps `x` between `lo` and `hi`. |
| [`sin`](#sin) | function | Returns the sine of `x` (radians). |
| [`cos`](#cos) | function | Returns the cosine of `x` (radians). |
| [`tan`](#tan) | function | Returns the tangent of `x` (radians). |
| [`asin`](#asin) | function | Returns the arcsine of `x` in radians. |
| [`acos`](#acos) | function | Returns the arccosine of `x` in radians. |
| [`atan`](#atan) | function | Returns the arctangent of `x` in radians. |
| [`atan2`](#atan2) | function | Returns the arctangent of `y/x` in radians, considering the quadrant. |
| [`sqrt`](#sqrt) | function | Returns the square root of `x`. |
| [`pow`](#pow) | function | Returns `x` raised to the power of `y`. |
| [`log`](#log) | function | Returns the natural logarithm of `x`. |
| [`exp`](#exp) | function | Returns e raised to the power `x`. |
| [`floor`](#floor) | function | Returns the largest integer ≤ `x`. |
| [`ceil`](#ceil) | function | Returns the smallest integer ≥ `x`. |
| [`round`](#round) | function | Rounds `x` to the nearest integer. |

---

### `PI = 3.14159265358979323846` <a name="pi--314159265358979323846"></a>

### `E = 2.7182818284590452354` <a name="e--27182818284590452354"></a>

### `NAN = 0 / 0` <a name="nan--0--0"></a>

### `INF = 1 / 0` <a name="inf--1--0"></a>

### `abs` <a name="abs"></a>

```xylia
abs(x)
```

Returns the absolute value of `x`.

### `min` <a name="min"></a>

```xylia
min(args[])
```

Returns the minimum value from a list of arguments.

### `max` <a name="max"></a>

```xylia
max(args[])
```

Returns the maximum value from a list of arguments.

### `clamp` <a name="clamp"></a>

```xylia
clamp(x, lo, hi)
```

Clamps `x` between `lo` and `hi`.

### `sin` <a name="sin"></a>

```xylia
sin(x)
```

Returns the sine of `x` (radians).

### `cos` <a name="cos"></a>

```xylia
cos(x)
```

Returns the cosine of `x` (radians).

### `tan` <a name="tan"></a>

```xylia
tan(x)
```

Returns the tangent of `x` (radians).

### `asin` <a name="asin"></a>

```xylia
asin(x)
```

Returns the arcsine of `x` in radians.

### `acos` <a name="acos"></a>

```xylia
acos(x)
```

Returns the arccosine of `x` in radians.

### `atan` <a name="atan"></a>

```xylia
atan(x)
```

Returns the arctangent of `x` in radians.

### `atan2` <a name="atan2"></a>

```xylia
atan2(y, x)
```

Returns the arctangent of `y/x` in radians, considering the quadrant.

### `sqrt` <a name="sqrt"></a>

```xylia
sqrt(x)
```

Returns the square root of `x`.

### `pow` <a name="pow"></a>

```xylia
pow(x, y)
```

Returns `x` raised to the power of `y`.

### `log` <a name="log"></a>

```xylia
log(x)
```

Returns the natural logarithm of `x`.

### `exp` <a name="exp"></a>

```xylia
exp(x)
```

Returns e raised to the power `x`.

### `floor` <a name="floor"></a>

```xylia
floor(x)
```

Returns the largest integer ≤ `x`.

### `ceil` <a name="ceil"></a>

```xylia
ceil(x)
```

Returns the smallest integer ≥ `x`.

### `round` <a name="round"></a>

```xylia
round(x)
```

Rounds `x` to the nearest integer.

