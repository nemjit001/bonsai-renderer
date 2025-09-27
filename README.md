# Bonsai Renderer

High performance render engine written in C++17.

## Code structure

Bonsai consists of several layers that build out its functionality, each layer builds on top of previous layers.
High level layers may only depend on lower-level layers.

Bonsai's API layers in order from low-level to high-level:

- **core** contains core code that other layers can build on. It is only dependent on external libraries.
- **platform** contains platform specific code such as window handling and filesystem access.
- **world** contains the renderer world representation.
- **rendering** contains the Bonsai rendering abstractions that wrap low level graphics APIs.

## External dependencies

All of Bonsai's dependencies are included as git submodules in the repository and pinned at a specific
version.

- GoogleTest (v1.17.0)
- SDL3 (v3.2.x)
- SPDLog (v1.15.3)
- Volk (v1.4.321)

## License

Bonsai is licensed under the MIT license.
