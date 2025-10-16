# Xylia Programming Language

Whether you're just getting started or diving into the internals, this documentation will guide you through everything from writing your first program to mastering the standard library.

---

## 🚀 Getting Started

If you're new to Xylia, start here:

- [Installation](./getting-started/installation.md) — how to build Xylia from source
- [Hello, World!](./getting-started/hello-world.md) — your first Xylia program

---

## 🧠 Language Guide

Learn the core language features step by step:

- [Syntax](./language/syntax.md) — keywords, literals, operators, and expressions
- [Control Flow](./language/control-flow.md) — if, loops, and flow constructs
- [Functions](./language/functions.md) — defining and calling functions
- [Modules](./language/modules.md) — modularizing your code
- [Types](./language/types.md) — primitive and composite types
- [Classes](./language/classes.md) — defining structured types

---

## 📚 Standard Library

Xylia comes with a growing standard library for common tasks:

- [Array](./stdlib/array.md)
- [IO](./stdlib/io.md)
- [Map](./stdlib/map.md)
- [Math](./stdlib/math.md)
- [Random](./stdlib/random.md)
- [Strings](./stdlib/strings.md)
- [Test](./stdlib/test.md)
- [Time](./stdlib/time.md)
- [Utility](./stdlib/utility.md)
- [Vector](./stdlib/vector.md)

---

## 🧰 Tooling & Development

- Use the built-in **REPL** for interactive experimentation.
- Xylia is designed to be easily embedded and extended.
- The build system uses CMake and Ninja for fast incremental builds.

---

## 🌟 Example

```xylia
let io = import("io");

io::println("Hello, Xylia!");
````

Run it:

```bash
./build/xylia main.xyl
```

---

## 🤝 Contributing

Xylia is an open-source project. Contributions are welcome — whether it's improving docs, adding new standard library features, or hacking on the compiler.

---

## 📎 Useful Links

* [Getting Started](./getting-started/installation.md)
* [Language Reference](./language/syntax.md)
* [Standard Library](./stdlib/index.md)

---

*Xylia — A language designed for clarity and control.*
