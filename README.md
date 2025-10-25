# Bonsai Renderer
[![CI Tests](https://github.com/nemjit001/bonsai-renderer/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/nemjit001/bonsai-renderer/actions/workflows/cmake-multi-platform.yml)

High performance render engine written in C++17.

## Code structure

Bonsai consists of several layers that build out its functionality, each layer builds on top of previous layers.
High level layers may only depend on lower-level layers.

Bonsai's API layers in order from low-level to high-level:

- **platform** contains platform specific code such as window handling, logging, and filesystem access.
- **core** contains core code that other layers can build on.
- **render_graph** contains the render graph API that builds on top of the RHI implementation.
- **assets** contains the asset API for host-side asset loading and management.
- **world** contains the host-side render world representation.
- **components** contains entity components that define behaviour.

## External dependencies

All of Bonsai's dependencies are included as git submodules in the repository and pinned at a specific
version. Bonsai's dependencies are split between general dependencies and platform specific dependencies.

General dependencies:

- GLM (v1.0.1)
- GoogleTest (v1.17.0)
- NLohmann JSON (v3.12.0)
- SDL3 (v3.2.x)
- SPDLog (v1.15.3)
- Tiny Object Loader (v2.0.0rc13)

Vulkan dependencies:

- Vulkan Memory Allocator (v3.3.0)
- Volk (v1.4.321)

## License

Bonsai is licensed under the MIT license.
