# Explain the code

Use these notes to understand and modify the implementation. They are not a replacement for inspecting the source or checking the course's rules about assistance and attribution.

## 1. Where does rendering start?

`main.cpp` creates a GLFW window and current OpenGL context, then loads functions through GLAD. `run()` creates the scene and UI while that context is alive. Each frame polls events, updates the camera and scene time, clears color/depth buffers, draws the scene, draws the UI and swaps buffers.

The scene objects leave scope before the window/context is destroyed, so their destructors can safely release GPU resources. Shader compile/link failures produce a readable error instead of silently displaying an empty window.

## 2. What is in a vertex?

`Vertex` contains a 3D position, a 3D normal and a 2D UV coordinate. A VBO stores these records. An EBO stores indices that refer to them. A VAO records the attribute formats and buffer associations. The renderer uses indexed triangle draws.

A mesh does not automatically know how to move, how to be lit or how to look textured. Those operations are provided by matrices, shaders and material textures.

## 3. What is the transformation order?

For column vectors:

```text
clipPosition = Projection * View * Model * localPosition
Model = Translation * Rz * Ry * Rx * Scale
```

The rightmost multiplication acts first. The vertex is scaled, rotated about X, Y and Z in that order, then translated. `Math.hpp` stores matrices column-major and `Shader::set` uploads them without transposing.

The perspective matrix follows OpenGL's conventional normalized device depth interval of -1 to +1. The CPU tests check the near/far-plane mapping.

## 4. Why not transform a normal like a position?

Translation should not change a direction. More subtly, multiplying a normal directly by a non-uniformly scaled model matrix can make it stop being perpendicular to the surface. `reef.vert` uses:

```glsl
normalize(transpose(inverse(mat3(model))) * localNormal)
```

Scale controls are clamped to positive values so this inverse remains well-defined. Kelp's simple wave deformation also applies the corresponding local normal correction.

## 5. What is Blinn-Phong here?

The fragment shader uses normalized surface normal `N`, direction toward the light `L`, view direction `V`, and halfway vector `H = normalize(L + V)`.

```text
ambient  = baseColor * ambientColor
diffuse  = baseColor * lightColor * max(dot(N,L),0)
specular = specularColor * max(dot(N,H),0)^shininess
```

The implementation suppresses the specular term when the light is behind the surface. It is a local illumination model: there is no shadow-map pass or physically based material system.

## 6. How are the coral models made?

Branching coral joins tapered cylinders and small rounded tips. A recursive rule produces shorter, narrower child branches. Tube coral builds four rings per tube: outside bottom, outside lip, inside lip and inside bottom. Plate coral combines flattened, slightly distorted parametric spheres.

All mesh generation happens on the CPU during scene creation. Coral growth is not simulated every frame. The resulting geometry is uploaded once and reused.

## 7. Is placement really Poisson-disc sampling?

`scatter()` uses a simple Poisson-disc-style dart-throwing method: generate a random point, reject it in reserved space, then reject it if it is within the minimum distance of an accepted seed. Stop at the requested count or attempt limit.

This enforces seed spacing, but it is not an optimized Bridson sampler and does not test the full geometry bounds. Differently scaled coral meshes can still overlap. Its O(n^2) comparison pattern is acceptable for this small prototype, but a spatial grid would be a sensible extension.

## 8. How does instancing help?

One `Instance` contains a model matrix and tint. Matrix columns occupy vertex attributes 3-6, with tint at 7. `glVertexAttribDivisor(...,1)` makes those attributes advance per instance instead of per vertex.

A single `glDrawElementsInstanced` call reuses the mesh for an entire batch. The vertex shader selects either the per-instance matrix/tint or uniform values for an individually drawn object. Fish deliberately use separate hierarchical draws to keep parent-child modeling visible in the code.

## 9. What are procedural textures and normal maps?

`makeMaterial()` fills a 256 x 256 RGB diffuse array and a height array using periodic functions. Differences between neighboring height samples produce approximate X/Y slopes. A normalized vector `(-dx * strength, -dy * strength, 1)` becomes a tangent-space normal encoded into RGB.

Both arrays are uploaded to real OpenGL textures and mipmaps are generated. A normal map changes the shading normal, not the vertex position or silhouette. `mappedNormal()` reconstructs tangent/bitangent directions from fragment derivatives of world position and UV; near-degenerate UV derivatives fall back to the geometric normal.

This optional derivative-based step is the least elementary part of the shader. The N comparison makes its contribution observable without making it essential to the basic scene.

## 10. Are the caustics physically accurate?

No. `makeCaustics()` creates a repeating cellular bright-line texture. The fragment shader projects it using world XZ coordinates and samples two moving layers. Upward-facing surfaces receive the strongest contribution.

This imitates the moving bright patterns seen underwater, but the program does not trace refracted light through a water surface. The caustics are not a screen-space algorithm, shadow map, fluid simulation or ray tracer.

## 11. How does fish hierarchy work?

A `Node` has a local matrix and optional mesh/material, plus child nodes. Recursive traversal computes:

```text
childWorld = parentWorld * childLocal
```

The swimming root moves on an ellipse and turns toward its path tangent. Its tail pivot translates behind the body and rotates sinusoidally. The tail mesh inherits both the root movement and pivot rotation. Changing the root moves the whole fish; changing the pivot moves only the tail subtree.

## 12. What does time control?

The application normally measures elapsed time with `glfwGetTime`. Movement is multiplied by delta time. Normal interactive updates clamp a very long frame to avoid a large jump after a pause or stall. The simulation clock drives fish, leaf sway, moving light, caustics and bubble rise.

P stops that clock, not the event loop. `--fixed-dt` is a separate deterministic capture/testing option; a fixed-timestep video must not be presented as evidence of the live frame rate.

## 13. Why depth testing and special bubble blending?

Depth testing chooses the closest visible opaque surface at each pixel rather than relying on draw order. The depth buffer is cleared every frame. Kelp and fins are two-sided, so face culling is intentionally disabled.

Bubbles are circular point sprites with a bright rim and transparent center. They are drawn after opaque geometry, with depth testing enabled but depth writes disabled. Additive blending combines their light contribution without requiring back-to-front sorting for this specific effect. Depth writes are restored afterward.

## 14. What would be reasonable next improvements?

Measure before optimizing. Potential extensions include a spatial grid for placement, shared low-detail eye meshes, less expensive normal transforms for instance batches, a more compact coral branch mesh, frustum culling, a clearer fish path layout and a simple shadow map only after the existing renderer is stable.

These are possible future changes, not implemented features. Avoid describing them as completed work in the report.
