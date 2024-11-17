# Random Art generator

Inspired by <https://github.com/tsoding/randomart>

Now it's a one-evening implementation.

A further optimized version will be added.
Also, shader generation, live update GUI with Time parameter will be added.
## Requirements

C++ std20 compatible compiler.
Tested with clang on macOS.
To use other compilers, modify `build/build.cpp`.

## Quick start

Open `build/` dirrectory.

First build:

```console
clang++ -Wall -std=c++17 build.cpp -o build
```

Further run with simple:

```console
./build
```

## Examples 

Chosen ppm format is raw, images are large, and GitHub doesn’t know how to render it.

Download and check it yourself.

- [Image 1](build/output/1731845589.ppm)
- [Image 2](build/output/1731845715.ppm)
- [Image 3](build/output/1731845961.ppm)
- [Image 4](build/output/1731845968.ppm)
- [Image 5](build/output/1731846570.ppm)


## Known issues
Compilation with `-O1/2/3` flags corrupts randomness somehow.
