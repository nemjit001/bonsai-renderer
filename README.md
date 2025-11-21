# Bonsai Renderer
[![CI Tests](https://github.com/nemjit001/bonsai-renderer/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/nemjit001/bonsai-renderer/actions/workflows/cmake-multi-platform.yml)

Flexible application / rendering framework with a focus on performance.
Bonsai aims to be a flexible engine-like software package that allows implementing
custom rendering applications.

## Supported platforms

Currently Bonsai supports 64-bit Windows and Linux. Cross-platform compatability is kept
in mind during development, ensuring similar performance across the supported platforms.

## Building Bonsai

Bonsai should build out of the box when using CMake 3.25 or higher. All dependencies are
included as Git submodules, making the build as straightforward as these two commands:

```
$ cmake . --preset <preset name>
```
```
$ cmake --build --preset <preset name>
```

## License

Bonsai is licensed under the MIT license.
