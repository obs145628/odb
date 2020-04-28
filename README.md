# odb

OBS Debugger.  
Generic debugger for virtual machines

# Environment

I only tested my programs on the following environment:

- Ubuntu 18.04 x64
- GCC 7.5
- python 3.6.9
- cmake 3.10.2

# Build

```
git submodule update --init
mkdir _build
cd _build
cmake ..
make
```

# Testing

```
make check
```

python required

# Add a VM support

Every VM need to implement an interface to work with ODB.  
Interface can be done in C/C++.  
Examples can be found at `tests/mockvms/`.  
I also use it for some of my projects:
- [chip8-toolchain](https://github.com/obs145628/chip8-toolchain)
