# Third-party notices and generated-code provenance

## Libraries

- **GLFW 3.4**: original Windows/core/null source subset and headers included under `include/glfw`; zlib/libpng license at `include/glfw/LICENSE.md`. Source: https://github.com/glfw/glfw/tree/3.4 . The subset was recovered from the user's earlier repository archive. Non-Windows implementations, upstream CMake files, examples and unrelated assets were omitted; included source code retains upstream notices.
- **GLAD**: the generated `deps/glad/gl.h` shipped by GLFW 3.4 is copied to `include/glad/gl.h`. Its generator and OpenGL-registry notices remain in the file. Source: https://raw.githubusercontent.com/glfw/glfw/3.4/deps/glad/gl.h . Implementation is instantiated exactly once in `src/glad_impl.c`.
- **GLM 1.0.1**: downloaded under `include/glm` with upstream `copying.txt`. MIT license. Source: https://github.com/g-truc/glm/tree/1.0.1 . GLM is the production math implementation; no local math replacement is shipped.
- **Dear ImGui v1.91.9b**: core and GLFW/OpenGL3 backends downloaded under `include/imgui` with `LICENSE.txt`. MIT license. Source: https://github.com/ocornut/imgui/tree/v1.91.9b . Its embedded default font is used; no external font file is distributed.

These upstream libraries are not student-written code. Keep notices and license files when copying or distributing them. Fixed versions support reproducible setup; they are not a claim to be the newest available releases.

## Assets and facts

Geometry, material textures and caustic pixels are generated in code. Real fish photos are separately installed from Wikimedia Commons with a file-specific creator, source and license record. Do not redistribute downloaded images without the associated credits/licensing terms. Species descriptions are short paraphrases of the aquarium/museum pages listed in `docs/REFERENCES.md` and the UI. Numerical masses, scene-unit speeds and model scaling are explicitly simulation choices, not sourced biological measurements.
