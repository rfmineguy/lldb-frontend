# Build
## Macos
- for Macos, it is recommended to use the llvm from brew (i.e. `brew install llvm`)

```bash
$ git clone https://github.com/rfmineguy/lldb-frontend.git
$ cmake -B build -GNinja "-DLLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm"
$ cmake --build build
```
The executable resides at `build/lldb-frontend`

## Windows

WIP

## Linux

WIP
