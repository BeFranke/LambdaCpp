## LambdaCpp, a c++ framework for interpreting lambda calculus

## How to use

### with tests

Note: If the project is built with test-support, [googletest](https://github.com/google/googletest) is automatically cloned by the CMake-script.

clone this repository, cd into it and run

```bash
mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=DEBUG ..
```

To run the tests after this, use

```bash
make test
```

### without tests

clone this repository, cd into it and run

```bash
mkdir build && cd build && cmake ..
```

### Run the example application


In both cases, you can build and run the REPL with:

```bash
make && ./REPL
```

from the build-folder


## Requirements

This project uses the C++14 standard and was built with:

+ g++ 9.3.0
+ cmake 3.16.3
+ googletest 1.10.0


## Directory structure

+ coverage: test-coverage report, to view open "index.html"
+ src: all source files
..+ src/lib: all library files, i.e. everything except the example application
+ test: unit tests
