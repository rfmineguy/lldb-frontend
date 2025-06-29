# Build

```bash
$ git clone https://github.com/rfmineguy/lldb-frontend.git
```

For all platforms you require an LLVM installation with an LLVMConfig.cmake at `${LLVM_ROOT}/lib/cmake/llvm` and the library `lldb` available as either Static (preferred) or shared

## Macos

```bash
$ export LLVM_ROOT=$(brew --prefix llvm)
$ cmake -B build -GNinja -DLLVM_DIR="$LLVM_ROOT/lib/cmake/llvm" -DCMAKE_BUILD_TYPE=Debug
$ cmake --build build
```
The executable resides at `build/lldb-frontend`

## Windows

The official releases of LLVM include everything we need, however you'll need to get *the windows tarball*, extract to a sane location and set the environment variable `LLVM_ROOT`

```cmd
$ set LLVM_ROOT=C:\Path\To\Extracted\LLVMTarball
$ cmake -B build -GNinja -DLLVM_DIR=%LLVM_ROOT%\lib\cmake\llvm -DCMAKE_BUILD_TYPE=Debug
$ cmake --build build
```

## Linux

`Dependencies`
```bash
$ sudo apt install -y llvm-dev libwayland-dev libxkbcommon-dev libx11-xcb-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev liblldb-dev libgl1-mesa-dev
```

```bash
$ cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Debug
$ cmake --build build
```