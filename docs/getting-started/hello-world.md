# Hello, World!

Let's write and run your first Xylia program.

---

## ✏️ Creating the File

Create a new file named `main.xyl` with the following contents:

```xylia
let io = import("io");

io::println("Hello, World!");
````

---

## ▶️ Running the Program

You can execute your program using the **Xylia interpreter** or **REPL**.

### Run directly:

```bash
./build/xylia main.xyl
```

### Or launch the REPL and paste the code:

```
& ./build/xylia
>>> let io = import("io");
>>> io::println("Hello, World!");
Hello, World!
```

---

## 💡 Explanation

* `import("io")` loads Xylia's standard I/O module.
* `io::println` prints text followed by a newline.
* Semicolons (`;`) are required at the end of expressions.

---

## 🧭 What's Next?

Now that you've run your first program, you can explore:

* [Language Syntax](../language/syntax.md)
* [Functions](../language/functions.md)
* [Standard Library: IO](../stdlib/io.md)
