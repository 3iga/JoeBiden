# Testing record and limitations

## Environment

Tests were performed in a Linux container with GCC, CMake/Ninja and Mesa software OpenGL. The renderer reported:

```text
OpenGL: 4.5 (Core Profile) Mesa 25.0.7-2
GPU: llvmpipe (LLVM 19.1.7, 256 bits)
```

The container did not have the real GLFW development package and could not download external source archives. To exercise the application, a **separate, local-only EGL adapter** supplied window/context and input-test bindings. OpenGL rendering and shader execution used Mesa; they were not mocked. The five project C++ translation units were compiled against a local validation header that declared the used OpenGL entry points.

The production archive still uses **actual GLFW and its bundled GLAD loader**, fetched by CMake. The EGL adapter and validation headers are **not shipped** in the project and are not represented as GLFW or GLAD.

## Completed checks

| Check | Result |
|---|---|
| CPU transform, view, projection, hierarchy and procedural-placement tests | Passed |
| Compilation of all five project C++ source files with C++17 and GCC warning flags | Passed; final compile produced no warnings |
| GLSL compilation and linking for reef, background, bubbles and UI | Passed in a real Mesa Core context |
| Default, low-density and high-density rendering | Passed with no observed OpenGL errors |
| Caustics, normal maps, diffuse texturing, lighting, fog and wireframe variants | Passed; each produced a different rendered image |
| Different procedural seed | Passed; rendered layout changed |
| Injected key-handler logic for reseeding, density, pause and transform modes | Passed through the local adapter |
| F2 focus view | Passed; focused renderer screenshot captured |
| BMP screenshot encoding/orientation | Inspected visually; correct |
| 288-frame fixed-timestep guided tour | Rendered and captured with no observed OpenGL errors |
| CMake target configuration, shader-copy ordering, compilation/linking and CTest | Passed in a throwaway project copy using the local test adapter as the dependency target |
| Launch of that CMake-built test application from a different working directory | Passed; executable-adjacent shaders were found |

The image-comparison smoke test used 800 x 450, a fixed timestep and four frames per variant. `renderer_test_results.json` contains its recorded output. Its short-run timings are **not performance benchmarks**.

The guided preview consists of 288 frames at a simulated 1/24-second step, encoded into a 12-second MP4. Capturing those frames at 1280 x 720 on the software renderer took about 24.6 seconds, including BMP writes. The encoded 24 FPS must **not** be described as the measured real-time application frame rate. It is an offline visual sample captured from the renderer, not an AI-generated illustration or a Windows screen recording.

## Not established by these tests

The real Windows/MSVC build, production GLFW download/build, actual GLAD loading, real GLFW event/cursor behavior, Windows high-DPI handling, display resizing/minimization, requested MSAA support, vsync behavior, hardware-GPU performance and the 4.1-to-3.3 GLFW context fallback were **not tested here**.

The CMake structure check used a substitute dependency target and does not prove that the official GLFW source builds on every machine. The event-logic check used injected input, not a real physical mouse or keyboard.

No Windows executable is included. The source is arranged for Visual Studio/CMake, but the team must build and run it on the presentation/TA machine before submitting. Use `tests/MANUAL_ACCEPTANCE.md` to complete that validation. Do not turn these renderer tests into claims of a completed Windows acceptance test or a guaranteed FPS.
