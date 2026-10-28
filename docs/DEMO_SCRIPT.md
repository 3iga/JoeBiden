# Luminous Reef: 4 minute 30 second live demo

Start with a Release build at 1280 x 720, seed 1337, medium density. Keep the console available in case a driver or shader error needs explaining. This outline fits the policy's 3-6 minute range; allow time for the on-screen actions rather than reading every sentence without pausing.

## 0:00-0:30 — Introduce the scene

**Action:** show the overview, with the Render Lab visible.

**Say:**

“Luminous Reef is an interactive underwater scene built in C++ using GLFW, GLAD and modern OpenGL. Its main features are procedural coral generation and dynamic underwater lighting. The application generates its geometry and texture pixels directly, rather than using a game engine or imported models.”

## 0:30-1:00 — Camera and viewing

**Action:** click the scene; move briefly with W/A/S/D, use the mouse, then Space and Ctrl. Press Home to restore the overview and Tab to release the mouse if needed.

**Say:**

“The camera produces the view matrix. Keyboard movement uses frame delta time, while mouse movement changes yaw and pitch. The perspective matrix gives the scene depth. Every object is transformed from its local coordinates to world space, then camera space and clip space.”

## 1:00-1:45 — Show the transformation pipeline

**Action:** press F2 to focus the marked coral. Press 2 and hold I briefly to rotate about Y. Press 3 and hold L to scale X. Press 1 and hold I to translate upward. Press Backspace to restore the coral.

**Say:**

“This coral exposes translation, rotation and non-uniform scaling. The model matrix is written as translation times rotation times scale. With column vectors, the scale is applied first, then rotation and finally translation. Surface normals use an inverse-transpose matrix, which is important when scale is different on each axis.”

Do not leave the coral deformed or floating for the rest of the presentation.

## 1:45-2:30 — Lighting, caustics and textures

**Action:** toggle B off/on, C off/on, T off/on, then N off/on. Leave each effect off for long enough to see the difference. Point the camera toward the sand for the caustics comparison.

**Say:**

“Blinn-Phong combines ambient, diffuse and specular light. Our caustics are an animated texture projected in world-space XZ coordinates. Two scrolling layers imitate moving underwater light; this is a visual approximation, not a physical water simulation. Diffuse textures provide surface color variation, while normal maps change shading without adding mesh detail.”

Lighting must be enabled to see caustics. Texturing must be enabled to see normal mapping.

## 2:30-3:10 — Procedural generation and efficiency

**Action:** press Home. Press R once to generate a new layout. Press Z to compare density; stop on medium or low if the target machine slows down. Briefly toggle M to show triangles, then turn it off.

**Say:**

“Placement uses a seeded minimum-distance sampler. It rejects candidate coral positions that are too close to existing seeds, and gives accepted instances different transforms and colors. A chosen seed reproduces the scene. Repeated coral, rock and kelp meshes use instanced drawing, so additional copies do not require one separate draw call each.”

Mention only FPS measured on the current machine. Wireframe is a geometry inspection mode, not an optimized presentation mode.

## 3:10-3:45 — Hierarchy and animation

**Action:** show a passing fish. Press P to pause, then P to continue. Toggle F briefly to show the fog difference.

**Say:**

“Each fish has a root transform for its swimming path, with child transforms for its body, fins, eyes and tail. The tail rotates around a local pivot and inherits the root's movement. The paths are scripted rather than AI-based. Fog blends distant objects into the water color. The pause control freezes scene time while leaving the camera usable.”

## 3:45-4:30 — Close with scope and evidence

**Action:** press G for the guided tour; optionally H to hide the overlay. Restore the UI before questions. Use F12 for a screenshot if needed.

**Say:**

“The project combines the course's transformations, viewing, shading, texture mapping, buffers, hierarchy and procedural methods in one interactive scene. It deliberately avoids water physics, ray tracing and fish behavior simulation. Our final submission includes the source repository, build instructions, report and demo. Performance results should be stated with the actual hardware, resolution and settings used.”

Replace the final submission sentence with the team's actual submission status. Do not claim a completed report or repository before those exist.

## Before presenting

Rehearse on the actual presentation computer, not just the development machine. Complete the manual acceptance checklist. Record a backup video from that working build. The supplied 12-second preview is a visual sample, not the full assessed 3-6 minute presentation and not a benchmark.
