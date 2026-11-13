#include "Renderer.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


void Renderer::initializeUI(GLFWwindow* window,float scale) {
    IMGUI_CHECKVERSION();ImGui::CreateContext();
    auto& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename=nullptr; // No stale window positions from an older version.
    ImGui::StyleColorsDark();auto& style=ImGui::GetStyle();
    style.WindowRounding=8;style.FrameRounding=4;style.GrabRounding=4;
    style.WindowPadding={14,12};style.ItemSpacing={9,8};
    style.Colors[ImGuiCol_WindowBg]={.018f,.055f,.072f,.96f};
    style.Colors[ImGuiCol_Header]={.08f,.32f,.35f,1};
    style.Colors[ImGuiCol_HeaderHovered]={.13f,.44f,.46f,1};
    style.Colors[ImGuiCol_Button]={.075f,.28f,.32f,1};
    style.Colors[ImGuiCol_ButtonHovered]={.12f,.42f,.43f,1};
    style.Colors[ImGuiCol_CheckMark]={.37f,.90f,.77f,1};
    style.Colors[ImGuiCol_SliderGrab]={.37f,.83f,.74f,1};
    style.ScaleAllSizes(clamp(scale,1,2));io.FontGlobalScale=clamp(scale,1,2);
    if(!ImGui_ImplGlfw_InitForOpenGL(window,true)){ImGui::DestroyContext();throw std::runtime_error("ImGui GLFW backend failed");}
    if(!ImGui_ImplOpenGL3_Init("#version 330")) {ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();throw std::runtime_error("ImGui OpenGL3 backend failed");}
    uiInitialized_=true;
}
void Renderer::shutdownUI() {
    if(uiInitialized_) {ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();uiInitialized_=false;}
}
void Renderer::beginUI() {if(uiInitialized_){ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();}}
void Renderer::endUI() {if(uiInitialized_){ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());}}
bool Renderer::wantsMouse() const {return uiInitialized_&&ImGui::GetIO().WantCaptureMouse;}
bool Renderer::wantsKeyboard() const {return uiInitialized_&&ImGui::GetIO().WantCaptureKeyboard;}
