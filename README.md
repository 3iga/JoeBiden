# Luminous Reef — TODO upgrade

A Windows-only **C++17 / OpenGL 3.3 Core / GLFW / GLAD / GLM / Dear ImGui** project. The only supported configuration is **x64 Debug**. No CMake, game engine, Linux/macOS target, Release target, or command-line feature flags are provided.

## Build and run (Windows 10/11)

1. Install Visual Studio with **Desktop development with C++**, MSVC x64 tools and a Windows SDK. CMake is not needed.
2. Extract this entire folder to a normal writable path, for example `C:\Projects\LuminousReef`.
3. Double-click **`run.bat`**. The scripts configure MSVC with `vcvarsall.bat x64`, compile the included Windows GLFW source, build the Debug application with `cl`.
4. Alternatively open **`LuminousReef.sln`**, select **Debug | x64**, Build, then Ctrl+F5. The VS project calls the same batch build.

Executable: **`bin\Debug\LuminousReef.exe`**. Keep its `shaders` and `assets` subfolders with it. Shader and asset paths are resolved from the executable, not the current directory. No separate GLFW DLL is used.

## What to do first

The reef, a nearby wrecked liner and animated caustics are visible immediately. The Dive panel offers a three-minute survey: press **Enter**, release the mouse with **Tab**, and click visible fish to discover all eight species. The same species counts once; browsing the Log does not score. Timeout and completion both allow another attempt without restarting the process. **F5** returns to the ready state. Free exploration continues after the timer ends.

Use **C** to compare caustics. In **Light**, compare Noon, Dusk and Midnight; disable the automatic cycle for a fixed hour. In **World**, change density or swim beyond the old demo boundaries. The generated seafloor now alternates between broad sediment plains, rolling slopes, high seamounts, winding trenches and deep volcanic-style craters. Coral is confined to separated reef patches; kelp and seagrass form their own beds. Small debris collects in occasional patches, while wrecks and ruins are spaced hundreds of metres apart. Click a wreck to open the **Seafloor field guide**. It separates the fictional in-game placement from facts about a real historical reference, including its loss, site, discovery and depth. Use **Open source online** to visit its reference pages. The experimental **Spawn Megalodon** button creates one stylized, explicitly fictional extinct-animal encounter. It is separate from the eight-species objective.

## Controls

| Input | Action |
|---|---|
| W/A/S/D, mouse | Swim and look; click open water or Tab to capture/release the cursor |
| Space / left Ctrl / left Shift | Up / down / faster swimming |
| Mouse wheel | Field of view; UI scrolling does not also zoom the camera |
| Enter / F5 / P | Start or replay survey / reset survey / pause simulation |
| F1 / H | Help / hide or show interface |
| Home / F2 / G | Return to origin / focus coral / guided camera |
| C / B / T / N / F | Caustics / lighting / textures / normal maps / fog comparison |
| R / Z / M / V | Next seed / density / wireframe / vertical sync |
| 1 / 2 / 3 | Featured coral translation / rotation / scale mode |
| J–L / K–I / U–O | Decrease–increase X / Y / Z transform component |
| Backspace | Reset featured coral transform |
| Left-click a visible fish or wreck | Inspect a species or open the seafloor field guide; fish score only during a survey |
| F9 / F12 | Record measured FPS sample to CSV / save a BMP screenshot |
| Esc | Exit |

Keyboard game controls always process input, including when the cursor is on an ImGui window. Mouse capture filtering is deliberately retained to prevent clicks passing through menus. Pause freezes simulation time and animation; camera motion and streaming stay usable. If text editing is active in a numeric slider, the same keys still control the game by design.

## Implementation map

- `Renderer.cpp/.hpp`: OpenGL resources, rendering, material uniforms, instancing, culling, integer-ID/depth picking; `RendererUI.cpp` integrates the actual ImGui backends.
- `Update.cpp/.hpp`: camera movement, fish steering, streaming/rebasing, day/night, survey state machine and rare encounters. No OpenGL calls.
- `Scene.hpp/.cpp`, `World.hpp`: plain scene data, seeded chunk streaming, habitat patches, large seamounts, deep canyons and craters, vegetation, debris and spaced wreck sites.
- `Geometry.cpp/.hpp`: attraction-point coral branches, fish and shark parts, procedural wrecks, seagrass, debris, ruins and terrain.
- `Graphics.cpp/.hpp`: explicit-return shader loading, GPU buffer/texture helpers and checked BMP I/O. All scene draw submissions are in Renderer.
- `MathSupport.hpp`: aliases and small composition wrappers around **real GLM**, not a replacement math implementation.
- `Interface.cpp/.hpp`: all on-screen controls, help and species information in Dear ImGui.
- `Species.cpp`: eight paraphrased museum/aquarium references and clearly distinguished simulation parameters.
- `ProjectInfo.hpp`: displayed team names and project title from the supplied proposal. Check these against the official topic sheet.
- `include/`: all external dependency source/headers and the locally built GLFW library.
- `assets/`: photo manifest, installed images, credits and metadata.
