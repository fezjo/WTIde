# Installation

```
git clone https://github.com/fezjo/WTIde.git
cd WTIde
git submodule update --init --recursive
bash patches/patch.sh apply
```


# Build

## Dependencies:
```
clang
cmake
```

## Process

```
cd lib/wtstar/src
make
make
cd ../../..

mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++)
cmake --build .
# run aplication with ./WTIde
```

# Update

```
git pull
git submodule update --init --recursive
bash patches/patch.sh apply
```
