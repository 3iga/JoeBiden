# Course and proposal alignment

Basis: the uploaded `Luminous_Reef_Proposal new.pptx`, together with the Final Project Policy, grading rubric and course-plan screenshots supplied in this conversation. This is an implementation map, not an amended course specification.

## Course topics

| Course-plan topic | Demonstration in this project | Source |
|---|---|---|
| Week 1: 2D Programming in OGL | Triangle-based on-screen text and clickable panels | UI.cpp, ui.vert, ui.frag |
| Week 2: OpenGL Shaders | Separate GLSL vertex/fragment programs | shaders/ |
| Week 3: Interaction, Windows, Menus and Animation; Geometry | GLFW window, keyboard/mouse input, buttons, time-based animation and generated meshes | main.cpp, Geometry.cpp |
| Week 4: Transformations and 3D Programming; Classical Viewing | Scale, Euler rotation, translation, view and perspective matrices | Math.hpp, main.cpp, reef.vert |
| Week 5: Viewing, Lighting, Materials and Shading | Free-swim camera and Blinn-Phong ambient/diffuse/specular lighting | Scene.hpp, reef.frag |
| Week 6: Curves and Surfaces | Parametric spheres/tubes, sinusoidal seabed, curved leaf ribbons | Geometry.cpp, Procedural.hpp |
| Week 7: Pixels and Buffers | VAO/VBO/EBO, depth buffer, texture pixels, pixel readback for BMP capture | Graphics.cpp, Scene.cpp |
| Week 8: Texture Mapping | UV coordinates, diffuse textures, normal textures and world-projected caustics | Graphics.cpp, reef.frag |
| Week 9: Modeling and Hierarchy | Fish root -> body/fins/eyes/tail pivot -> tail; recursive traversal | Scene.cpp, Node in Scene.hpp |
| Week 10: Procedural Methods | Recursive coral branches, generated surfaces/textures and seeded placement | Geometry.cpp, Graphics.cpp, Procedural.hpp |

No compute shaders, ray tracing, tessellation pipeline, physics engine or machine-learning library is used. Instancing, normal maps and caustics are retained because they are specifically described in the proposal. The optional normal-map tangent reconstruction uses fragment derivatives; understand this part before emphasizing it in a defense.

## Proposal features

The implementation covers the central promises: a custom modern-OpenGL scene, model/view/projection transforms, a first-person camera, generated coral/rock placement, Blinn-Phong lighting, animated projected caustics, diffuse/normal textures, fog, instancing and a controllable demo.

Fish behavior simulation, a simulated water surface, multiplayer and a full day-night cycle remain out of scope, as stated on proposal slide 5. Fish motion here is a deterministic path animation. The caustics follow slide 8's animated-texture description; they are not the screen-space/refraction techniques mentioned as background inspiration on slide 4.

### Deliberate implementation differences to describe accurately

- **API version:** the preferred context is OpenGL 4.1 Core, with a 3.3 Core fallback. GLSL 330 keeps every used feature inside a consistent baseline. Do not claim that the demo requires advanced OpenGL 4.x features.
- **Math:** the preliminary GLM choice is replaced by a small explicit `Math.hpp`, exposing the transformation math for study and explanation.
- **Texture loading:** stb_image is unnecessary because textures are generated in memory and uploaded with `glTexImage2D`. These are real GPU textures, not an absence of texturing.
- **Placement:** the sampler is minimum-distance rejection/dart-throwing, a simple Poisson-disc-style approach. It is not a grid-accelerated Bridson implementation or a full noise library.
- **Reproducibility:** startup uses seed 1337 rather than a new random seed every run. R generates a different layout, and `--seed` reproduces a chosen one.
- **Fixed demonstration object:** one coral and its support/selection marker are deliberately anchored for the transform exercise. The remaining coral field is generated. Do not describe every object as randomly placed.
- **Extra interaction:** the transform panel, focused view, comparison toggles, density control and screenshot capture make the course concepts demonstrable. They are not a full scene editor.

## Grading evidence and remaining deliverables

| Rubric | Evidence available in the demo | What the team still needs to do |
|---|---|---|
| Technical depth & correctness: 35% | Modern shaders, depth testing, complete MVP, inverse-transpose normals, hierarchy, instancing, procedural generation | Explain the implementation and run the acceptance checklist on the target PC |
| Visual quality & performance: 20% | Colored reef, procedural materials, animated caustics, fog, bubbles, density presets and frame statistics | Record actual Release-mode measurements, GPU/CPU and resolution; identify bottlenecks |
| Interactivity / UX: 15% | Camera controls, clickable toggles, help overlay, transform editing, focus/reset and pause | Rehearse controls and verify real keyboard/mouse behavior |
| Code quality & documentation: 10% | Separated source modules, build configuration, source map, comments, tests and troubleshooting | Review/adapt the code, retain dependency attribution, publish the buildable repository |
| Report quality: 15% | Explanation notes and an honest testing record to draw on | Write the report using the provided course template, add actual results and citations |
| Presentation / demo: 5% | Live-demo script and an optional fixed-timestep recording command | Deliver/rehearse a 3-6 minute presentation with a backup video |

These features support the rubric; they do not guarantee a particular grade. The archive is not the final written report and does not replace the presentation deck.

## Team-size detail to confirm

The policy screenshot says teams are *typically* 2-3 students. The proposal names five team members. That is not an explicit prohibition, but confirm the group arrangement with the lecturer before final submission.
