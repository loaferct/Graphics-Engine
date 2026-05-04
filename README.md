# Graphics Engine

A compact C++ OpenGL mesh renderer and learning project. This repository contains a small engine demonstrating shader-based rendering, model/texture loading, and a basic camera system using GLFW, GLM, and stb image.

## Features

- OpenGL renderer (modern pipeline)
- Shader management system
- Basic model/mesh handling and texture support
- Camera utilities for navigating the scene
- Included sample models and textures

## Requirements

- Linux (tested) or other platforms with OpenGL support
- C++17-compatible compiler (g++ recommended)
- CMake 3.16+
- OpenGL development headers

Third-party libraries used (bundled or provided via CMake):

- GLFW
- GLM
- glad
- stb (image loading, utilities)

## Build Instructions

1. Create a build directory and run CMake:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

2. Run the application (from the `build` directory):

```bash
./voxel_engine
```

Notes:

- The project is configured with CMake. If CMake reports missing packages, ensure system development packages for OpenGL and X11 (or your platform's windowing system) are installed.
- The workspace includes a prepared `build/` folder that contains a compiled binary when available.

## Project Layout

- `src/` — engine and application source files (main, camera, mesh, model, shader, textures)
- `include/` — public headers for engine modules
- `resources/shaders/` — GLSL vertex & fragment shaders
- `models/` — sample models and textures
- `external/`, `build/_deps/` — vendored third-party dependencies used during CMake configuration
- `ScreenShots/` — example screenshots

## Running from VS Code

You can use the provided C/C++ build tasks or configure the CMake Tools extension. The typical workflow:

- Open the folder in VS Code
- Configure CMake via the CMake Tools extension (or run the commands above in a terminal)
- Start the compiled binary from the `build/` folder or configure a debug target using the built executable

## Usage / Controls

Controls vary depending on the current demo scene implementation. Common controls (if implemented) include WASD movement and mouse look. If controls are missing or different, check `src/` for input handling.

## Contributing

Contributions are welcome. Suggested steps:

1. Open an issue describing the change or bug.
2. Create a branch for your work.
3. Send a pull request with a clear description and small, focused commits.

## License

No license is included in the repository root. If you intend to publish or share this project, add a `LICENSE` file (for example, MIT) and update this section.

## Next Steps and Ideas

- Add a small README section describing main controls (when confirmed in code)
- Add CI that builds with CMake and runs a smoke test
- Include more sample scenes, model formats, or a simple level file

---

If you want, I can: (a) detect and insert accurate runtime controls from the code, (b) add a `LICENSE` file (MIT), or (c) create a minimal CI workflow to build the project. Tell me which one to do next.
