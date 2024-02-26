# Installation

```bash
git clone https://github.com/fezjo/WTIde.git
cd WTIde
git submodule update --init --recursive
bash patches/patch.sh apply
```

# Build

## Dependencies:

- [clang](https://clang.llvm.org/)
- [cmake](https://cmake.org/)

## Process

```bash
cd lib/wtstar/src
make
make
cd ../../..

# for linux
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++)
cmake --build .
# run aplication with ./WTIde

# for web
mkdir build_web
cd build_web
emcmake cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DEMSCRIPTEN=on
cmake --build .
emrun ./WTIde.html
```

# Update

```bash
git pull
git submodule update --init --recursive
bash patches/patch.sh apply
```
