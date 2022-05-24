# Installation

```
git clone https://github.com/fezjo/WTIde.git
cd WTIde
git submodule update --init --recursive
bash patches/patch.sh apply
```

# Build

```
cd lib/wtstar/src
make
make
cd ../../..

mkdir build
cd build
cmake ..
cmake --build .
# run aplication with ./WTIde
```