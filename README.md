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
```

## Usage
```bash
./build/unmacro --macro-name=MY_MACRO <source_file> -- [compiler_flags]
```
Use `--inplace` to modify files directly.

### C++ Usage
When processing C++ files, you may need to specify the C++ standard and include paths:
```bash
./build/unmacro --macro-name=MY_TRACE file.cpp -- -std=c++17 -I/path/to/includes
```

**Note:** If the tool encounters errors parsing system headers or standard libraries, ensure you are passing the correct include paths and flags. For C++ files with complex dependencies, it is recommended to use a compilation database (`compile_commands.json`) or provide all necessary `-I` flags after the `--` separator.
