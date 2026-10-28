# Acceptance checklist for the actual Windows presentation machine

These checks remain for the team. A renderer test through a Linux EGL adapter does not validate the real Visual Studio, GLAD, GLFW, driver, mouse or display environment.

## Build and startup

- [ ] Extract a clean copy and configure/build x64 Release in Visual Studio.
- [ ] First dependency download succeeds, or the offline GLFW source directory works.
- [ ] Run `ctest` and confirm the CPU test passes.
- [ ] Start from Visual Studio and directly from the executable's folder.
- [ ] Console identifies the real GPU and an OpenGL Core version of at least 3.3.
- [ ] No shader compilation/linking or missing-file errors appear.

## Real interaction

- [ ] Click the reef, move with WASD, look with the mouse and release with Tab.
- [ ] Space/Ctrl change height; Shift changes speed; the wheel changes field of view.
- [ ] Home restores overview; F2 focuses the featured coral; G enables/disables the tour.
- [ ] Click every Render Lab button with the mouse released.
- [ ] Check the corresponding keyboard shortcuts independently.
- [ ] Hold the transform keys in move, rotate and scale modes; Backspace resets.
- [ ] Mouse capture does not jump after Tab, Home, F2 or refocusing the window.

## Rendering and state

- [ ] Lighting, caustics, textures, normal maps and fog each produce a visible comparison.
- [ ] Caustics need lighting; normal mapping needs texturing, as documented.
- [ ] P freezes scene animation but leaves manual camera movement available.
- [ ] M shows wireframe, then restores filled rendering.
- [ ] R changes the seed and layout; launching with the same seed reproduces it.
- [ ] Z cycles density and updates the actual object count.
- [ ] H hides/restores the overlay and marker.
- [ ] Resizing keeps the scene's aspect ratio and mouse/button hit locations correct.
- [ ] Minimize/restore, alt-tab and close the application without a crash.
- [ ] F12 saves a readable BMP with correct colors and orientation.
- [ ] A finite `--check-gl` run exits successfully.

## Performance and submission

- [ ] Record CPU, GPU, RAM, OS/driver, resolution, Release build and vsync setting.
- [ ] Compare low/medium/high density with the same camera, seed and effects.
- [ ] Do not use startup-only samples or disk-capture runs as final benchmarks.
- [ ] Test on the TA/presentation computer, including its Internet/offline dependency situation.
- [ ] Rehearse a 3-6 minute demo and record a backup from the working build.
- [ ] Complete the report in the required template and confirm the team-size arrangement.
