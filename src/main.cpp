#ifndef _WIN32
#error Luminous Reef targets Windows 10/11 x64 only. Use build.bat.
#endif
#include <Windows.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "Renderer.hpp"
#include "Update.hpp"
#include "ProjectInfo.hpp"
#include <array>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

struct Input {
std::array<bool, GLFW_KEY_LAST + 1> pressed{};
bool click = false, captured = false, firstMouse = true;
double lastX = 0, lastY = 0, scroll = 0;
void clear() {
    pressed.fill(false);
    click = false;
    scroll = 0;
}
};

static void captureMouse(GLFWwindow* window, Input& input, bool capture) {
    input.captured = capture;
    input.firstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, capture ? GLFW_TRUE : GLFW_FALSE);
}

static std::filesystem::path executableFolder() {
    std::array<wchar_t, 32768> buffer{};
    DWORD count = GetModuleFileNameW(nullptr, buffer.data(), DWORD(buffer.size()));
    if (count == 0 || count >= buffer.size()) return {};
    return std::filesystem::path(std::wstring(buffer.data(), count)).parent_path();
}

static InputFrame readInput(GLFWwindow* window, Input& input, bool allowMouse) {
    InputFrame frame;
    const std::pair<int, Action> keys[] = {
    {GLFW_KEY_B, Action::Lighting}, {GLFW_KEY_C, Action::Caustics},
    {GLFW_KEY_T, Action::Textures}, {GLFW_KEY_N, Action::Normals},
    {GLFW_KEY_F, Action::Fog}, {GLFW_KEY_P, Action::Pause},
    {GLFW_KEY_M, Action::Wireframe}, {GLFW_KEY_H, Action::UI},
    {GLFW_KEY_G, Action::Tour}, {GLFW_KEY_V, Action::Vsync},
    {GLFW_KEY_R, Action::NewSeed}, {GLFW_KEY_Z, Action::Quality},
    {GLFW_KEY_HOME, Action::Home}, {GLFW_KEY_F2, Action::Focus},
    {GLFW_KEY_BACKSPACE, Action::ResetCoral}, {GLFW_KEY_1, Action::MoveMode},
    {GLFW_KEY_2, Action::RotateMode}, {GLFW_KEY_3, Action::ScaleMode},
    {GLFW_KEY_ENTER, Action::StartSurvey}, {GLFW_KEY_F5, Action::RestartSurvey},
    {GLFW_KEY_F1, Action::Help},
    {GLFW_KEY_F3, Action::VisualCompare}, {GLFW_KEY_F4, Action::PreviewVista},
    {GLFW_KEY_F6, Action::PreviewArch}, {GLFW_KEY_F7, Action::PreviewKelp}
    };
    for (const auto& key : keys) if (input.pressed[std::size_t(key.first)]) frame.press(key.second);
    auto down = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS ? 1.0f : 0.0f;
    };
    // Deliberately independent of ImGui::WantCaptureKeyboard.
    frame.move = {down(GLFW_KEY_D) - down(GLFW_KEY_A),
    down(GLFW_KEY_SPACE) - down(GLFW_KEY_LEFT_CONTROL),
    down(GLFW_KEY_W) - down(GLFW_KEY_S)};
    frame.transformAxis = {down(GLFW_KEY_L) - down(GLFW_KEY_J),
    down(GLFW_KEY_I) - down(GLFW_KEY_K),
    down(GLFW_KEY_O) - down(GLFW_KEY_U)};
    frame.fast = down(GLFW_KEY_LEFT_SHIFT) != 0;
    frame.scroll = allowMouse ? float(input.scroll) : 0;
    frame.looking = input.captured;
    if (input.captured) {
        double x = 0, y = 0;
        glfwGetCursorPos(window, &x, &y);
        if (input.firstMouse) {
            input.lastX = x;
            input.lastY = y;
            input.firstMouse = false;
        }
        frame.mouseDelta = {float(x - input.lastX), float(y - input.lastY)};
        input.lastX = x;
        input.lastY = y;
    }
    return frame;
}

static bool recordPerformance(const std::filesystem::path& folder, const Scene& scene,
const RenderStats& stats, float fps, int width, int height) {
    std::error_code error;
    std::filesystem::create_directories(folder, error);
    if (error) {
        std::cerr << "ERROR: benchmark folder: " << error.message() << '\n';
        return false;
    }
    auto path = folder / "fps_samples.csv";
    bool exists = std::filesystem::exists(path, error);
    std::ofstream file(path, std::ios::app);
    if (!file) {
        std::cerr << "ERROR: cannot open " << path << '\n';
        return false;
    }
    if (!exists) file << "unix_time,gpu,width,height,configuration,caustics,lighting,fog,normal_maps,quality,visibility,vsync,paused,fps,frame_ms,draws,triangles\n";
    file << std::time(nullptr) << ",\"" << glGetString(GL_RENDERER) << "\"," << width << ',' << height
    << ",Debug," << scene.settings.caustics << ',' << scene.settings.lighting << ','
    << scene.settings.fog << ',' << scene.settings.normalMaps << ',' << scene.settings.quality << ','
    << scene.settings.visibility << ',' << scene.settings.vsync << ',' << scene.settings.paused << ','
    << fps << ',' << (fps > 0 ? 1000 / fps : 0) << ',' << stats.draws << ',' << stats.triangles << '\n';
    if (!file) return false;
    std::cout << "Saved measured frame-rate sample: " << path << '\n';
    return true;
}

static int run(GLFWwindow* window, const std::filesystem::path& folder) {
    Input input;
    glfwSetWindowUserPointer(window, &input);
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        auto* in = static_cast<Input*>(glfwGetWindowUserPointer(w));
        if (key >= 0 && key <= GLFW_KEY_LAST && action == GLFW_PRESS) in->pressed[std::size_t(key)] = true;
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        static_cast<Input*>(glfwGetWindowUserPointer(w))->click = true;
    });
    glfwSetScrollCallback(window, [](GLFWwindow* w, double, double y) {
        static_cast<Input*>(glfwGetWindowUserPointer(w))->scroll += y;
    });
    glfwSetWindowFocusCallback(window, [](GLFWwindow* w, int focused) {
        auto* in = static_cast<Input*>(glfwGetWindowUserPointer(w));
        in->clear();
        if (!focused) captureMouse(w, *in, false);
    });

    Scene scene;
    Update update;
    update.initialize(scene);
    Renderer renderer(folder / "shaders", folder / "assets");
    if (!renderer.valid() || !Renderer::checkErrors() || !renderer.initializeUI(window)) return 1;
    Ui ui;
    glfwSwapInterval(1);
    bool previousVsync = true;
    double previous = glfwGetTime(), statisticsStart = previous;
    int statisticsFrames = 0, screenshotIndex = 0;
    float fps = 0, frameMs = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        double now = glfwGetTime();
        float dt = clamp(float(now - previous), 0, .1f);
        previous = now;
        int width = 0, height = 0, windowWidth = 0, windowHeight = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        if (width <= 0 || height <= 0 || windowWidth <= 0 || windowHeight <= 0) {
            input.clear();
            glfwWaitEventsTimeout(.1);
            previous = glfwGetTime();
            statisticsStart = previous;
            statisticsFrames = 0;
            continue;
        }
        if (input.pressed[GLFW_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (input.pressed[GLFW_KEY_TAB]) captureMouse(window, input, !input.captured);
        renderer.beginUI(input.captured);
        InputFrame requests = ui.build(scene, renderer.stats(), renderer.photos(), input.captured, fps, frameMs);
        InputFrame frame = readInput(window, input, input.captured || !renderer.wantsMouse());
        for (std::size_t i = 0; i < frame.action.size(); ++i) frame.action[i] = frame.action[i] || requests.action[i];
        update.step(scene, frame, dt);
        if (frame.pressed(Action::Home) || frame.pressed(Action::Focus) || frame.pressed(Action::PreviewVista) ||
            frame.pressed(Action::PreviewArch) || frame.pressed(Action::PreviewKelp)) input.firstMouse = true;
        if (previousVsync != scene.settings.vsync) {
            glfwSwapInterval(scene.settings.vsync ? 1 : 0);
            previousVsync = scene.settings.vsync;
        }
        renderer.render(scene, width, height);
        if (input.click && !input.captured && !renderer.wantsMouse()) {
            double x = 0, y = 0;
            glfwGetCursorPos(window, &x, &y);
            int px = int(x * double(width) / windowWidth);
            int py = height - 1 - int(y * double(height) / windowHeight);
            int picked = renderer.pick(scene, px, py);
            if (picked>=int(StructurePickBase))update.selectStructure(scene, static_cast<std::uint32_t>(picked));
            else if (picked >= 0) update.selectFish(scene, picked);
            else captureMouse(window, input, true);
        }
        renderer.endUI();
        if (input.pressed[GLFW_KEY_F12]) {
            auto path = folder / "screenshots" / ("reef_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(screenshotIndex++) + ".bmp");
            if (renderer.screenshot(path)) std::cout << "Saved screenshot: " << path << '\n';
        }
        if (input.pressed[GLFW_KEY_F9] && fps > 0)
        recordPerformance(folder / "benchmarks", scene, renderer.stats(), fps, width, height);
        if (!Renderer::checkErrors()) return 1;
        glfwSwapBuffers(window);
        input.clear();
        ++statisticsFrames;
        double elapsed = glfwGetTime() - statisticsStart;
        if (elapsed >= 1) {
            fps = float(statisticsFrames / elapsed);
            frameMs = 1000 / fps;
            statisticsFrames = 0;
            statisticsStart = glfwGetTime();
        }
    }
    return 0;
}

int main() {
    // No command-line configuration flags. Defaults live in Scene.hpp; comparison controls are in ImGui.
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW ERROR " << code << ": " << message << '\n';
    });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, ProjectTitle, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        std::cerr << "ERROR: GLAD failed to load OpenGL 3.3 functions\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeLimits(window, 960, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    std::cout << "Luminous Reef - Windows x64 Debug\nGPU: " << glGetString(GL_RENDERER)
    << "\nOpenGL: " << glGetString(GL_VERSION) << '\n';
    auto folder = executableFolder();
    int result = 1;
    if (folder.empty()) std::cerr << "ERROR: cannot locate the executable directory\n";
    else result = run(window, folder);
    // GPU resources are destroyed before the context below.
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
