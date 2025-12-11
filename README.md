# Bonsai Renderer
[![CI Tests](https://github.com/nemjit001/bonsai-renderer/actions/workflows/ci_tests.yml/badge.svg?branch=main)](https://github.com/nemjit001/bonsai-renderer/actions/workflows/ci_tests.yml)

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

## Dependencies

Bonsai aims to have minimal dependencies, including only what is needed. Applications can choose to pull in additional
dependencies if they so require.

A complete list of shared and per-platform dependencies is given below.

### Shared dependencies

- DirectX Shader Compiler, used for runtime shader compilation and reflection.
- GoogleTest, used as unit testing framework.
- ImGui, used as application Debug GUI library.
- SDL3, used for platform and window management.
- SPDLog, used for high quality logging.

### Vulkan dependencies

- SPIRV-Reflect, used for SPIR-V shader reflection.
- VMA, used for Vulkan allocation management.
- Volk, used for Vulkan function loading at runtime.

## License

Bonsai is licensed under the MIT license.
