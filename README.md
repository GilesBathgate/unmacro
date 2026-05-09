# unmacro
A syntax-aware refactoring tool for de-macroing legacy C/C++.

## Build Dependencies
To build this tool on Ubuntu (Noble/24.04 or later), you will need the following packages:
```bash
sudo apt-get update
sudo apt-get install -y libclang-18-dev libclang-cpp18-dev llvm-18-dev libzstd-dev cmake g++
```

## Building
```bash
mkdir -p build
cd build
cmake ..
make
# Create C++ symlink
ln -s unmacro unmacro++
```

## Usage
```bash
./build/unmacro --macro-name=MY_MACRO [options] <source_file> -- [compiler_flags]
```

### Options
- `--macro-name=<string>`: The specific macro to remove (Required).
- `--inplace`: Modify the file directly.
- `--remove-extra-statements`: Remove empty `if`, `for`, `while`, and `do` statements that become empty after macro removal.

### C++ Usage
For C++ files or headers, you can use the `unmacro++` symlink, which automatically adds the `-xc++` flag:
```bash
./build/unmacro++ --macro-name=MY_TRACE file.cpp -- -std=c++17
```

Alternatively, if using the base `unmacro` tool, ensure you pass the correct include paths and flags after the `--` separator:
```bash
./build/unmacro --macro-name=MY_TRACE file.cpp -- -std=c++17 -I/path/to/includes
```

**Note:** The tool automatically detects C++ headers (`.h` files) and applies `-xc++` if a C++ standard flag (e.g., `-std=c++17`) is provided.
