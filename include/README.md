# Dependencies (all located under include)

GLFW 3.4 Windows source and its generated GLAD header were recovered from the user's earlier repository archive, with upstream notices retained. GLM 1.0.1 and Dear ImGui v1.91.9b are fetched by `setup_dependencies.bat`. No font files are redistributed.

Expected headers: `glm/glm.hpp`, `glad/gl.h`, `glfw/include/GLFW/glfw3.h`, `imgui/imgui.h`. The Debug-built GLFW static library is `glfw/lib/glfw3_debug.lib`.

See `docs/DEPENDENCIES_AND_ASSETS.md` for versions, official URLs, licenses, and the fully manual installation route. The download script does not run downloaded installers or change system settings.
