#include "Renderer.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

bool Renderer::initializeUI(GLFWwindow* window, float scale) {
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext()) {
        std::cerr << "ERROR: ImGui context creation failed\n";
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    // No keyboard-navigation mode: keyboard always controls the dive, as requested.
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 7;
    style.FrameRounding = 4;
    style.WindowPadding = {12, 10};
    style.ItemSpacing = {8, 7};
    style.Colors[ImGuiCol_WindowBg] = {.018f, .055f, .072f, .96f};
    style.Colors[ImGuiCol_Header] = {.08f, .32f, .35f, 1};
    style.Colors[ImGuiCol_Button] = {.075f, .28f, .32f, 1};
    style.Colors[ImGuiCol_ButtonHovered] = {.12f, .42f, .43f, 1};
    style.Colors[ImGuiCol_CheckMark] = {.37f, .90f, .77f, 1};
    style.Colors[ImGuiCol_SliderGrab] = {.37f, .83f, .74f, 1};
    style.ScaleAllSizes(clamp(scale, 1, 1.5f));
    io.FontGlobalScale = clamp(scale, 1, 1.5f);
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "ERROR: ImGui GLFW backend failed\n";
        ImGui::DestroyContext();
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "ERROR: ImGui OpenGL backend failed\n";
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }
    uiInitialized_ = true;
    return true;
}
void Renderer::shutdownUI() {
    if (!uiInitialized_) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    uiInitialized_ = false;
}
void Renderer::beginUI(bool captured) {
    if (!uiInitialized_) return;
    if (captured) ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    else ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void Renderer::endUI() {
    if (!uiInitialized_) return;
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
bool Renderer::wantsMouse() const {
// Kept deliberately: a click on a slider must not also scan a fish behind it.
return uiInitialized_ && ImGui::GetIO().WantCaptureMouse;
}
