# Installation

Xylia can be built from source using CMake and Ninja.  
This guide will walk you through the setup and build process.

---

## 🧩 Prerequisites

Before you begin, make sure the following dependencies are installed on your system:

- **CMake** ≥ 3.20  
- **Ninja** build system  
- **libffi** development library  
- **git**

On most Linux distributions you can install these using your package manager:

```bash
# Example for Debian/Ubuntu
sudo apt install git cmake ninja-build libffi-dev

# Example for Arch Linux
sudo pacman -S git cmake ninja libffi
````

---

## 📥 Cloning the Repository

Xylia uses Git submodules (for [replxx](https://github.com/AmokHuginnsson/replxx)),
so make sure to clone recursively:

```bash
git clone --recurse-submodules https://github.com/vh8t/xylia.git
cd xylia
```

If you accidentally cloned without submodules, you can initialize them later:

```bash
git submodule update --init --recursive
```

---

## 🛠️ Building Xylia

You can either use the provided **build script**, or run CMake manually.

### Option 1: Using the build script

```bash
./build.sh
```

This script automatically configures and builds Xylia using Ninja.

### Option 2: Manual build steps

```bash
# Create a build directory
mkdir build

# Configure the project
cmake -S . -B ./build -G "Ninja"

# Compile the binary
ninja -C ./build
```

---

## 📦 Result

After a successful build, the Xylia executable will be located at:

```bash
./build/xylia
```

You can run it directly:

```bash
./build/xylia
```

---

## 🧪 Verifying the REPL

You should see the interactive prompt:

```
& xylia
>>> let io = import("io");
>>> io::println("Hello, Xylia!");
Hello, Xylia!
```

---

## ✅ Next Steps

* [Hello World](./hello-world.md) — write and run your first program
* [Language Basics](../language/syntax.md) — learn about syntax and semantics
