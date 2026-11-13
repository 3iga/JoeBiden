#include "Interface.hpp"
#include <imgui.h>
#include <algorithm>
#include "ProjectInfo.hpp"

void Ui::fieldGuide(Scene& scene,const std::array<PhotoView,SpeciesCount>& photos) {
    if(!scene.showFishInfo||scene.guideSpecies<0||scene.guideSpecies>=SpeciesCount)return;
    auto size=ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({20,195},ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({410,std::max(300.0f,size.y-245)},ImGuiCond_Appearing);
    if(ImGui::Begin("Fish field guide",&scene.showFishInfo)) {
        int id=scene.guideSpecies;const auto& info=SpeciesList[std::size_t(id)];const auto& photo=photos[std::size_t(id)];
        ImGui::TextColored({.54f,.92f,.81f,1},"%s",info.name);ImGui::TextDisabled("%s",info.scientific);
        ImGui::Separator();
        if(photo.textureId) {
            float w=ImGui::GetContentRegionAvail().x,h=w*float(photo.height)/float(std::max(1,photo.width));
            if(h>220){w*=220/h;h=220;}
            ImGui::Image(static_cast<ImTextureID>(photo.textureId),{w,h});
            ImGui::TextDisabled("Real photograph / Wikimedia Commons");
        } else {
            ImGui::TextWrapped("Real photo is not installed. Run setup_assets.bat, then restart the program.");
            if(!photo.error.empty())ImGui::TextDisabled("%s",photo.error.c_str());
        }
        ImGui::SeparatorText("Species reference");
        ImGui::TextWrapped("%s",info.description);ImGui::Spacing();ImGui::TextWrapped("%s",info.fact);
        ImGui::Text("Source-reported maximum: %.1f cm",info.maximumLengthCm);
        ImGui::TextWrapped("See the cited source for measurement convention (total vs. standard length).");
        if(ImGui::Button("Copy biological source URL"))ImGui::SetClipboardText(info.source);
        if(ImGui::CollapsingHeader("Source and photo attribution")) {
            ImGui::TextWrapped("Biology: %s",info.source);
            ImGui::TextWrapped("%s",photo.credit.empty()?"Photo credits are saved alongside each installed photograph.":photo.credit.c_str());
            if(ImGui::Button("Copy photo credit"))ImGui::SetClipboardText(photo.credit.c_str());
        }
        ImGui::SeparatorText("Simulation specimen");
        if(scene.selectedFish>=0&&scene.selectedFish<int(scene.fish.size())&&scene.fish[std::size_t(scene.selectedFish)].species==id) {
            const auto& f=scene.fish[std::size_t(scene.selectedFish)];
            ImGui::Text("Specimen #%u / variant %d",f.id,f.variant+1);
            ImGui::Text("Generated length: %.1f cm",f.lengthCm);
            ImGui::Text("Illustrative mass: %.3f kg",f.massKg);
            ImGui::Text("Current speed: %.2f scene units/s",length(f.velocity));
        } else ImGui::TextWrapped("Click a visible fish to inspect a live specimen. Browsing the guide does not count as a discovery.");
        ImGui::TextWrapped("Steering: %s",behaviourName(info.behaviour));
        ImGui::TextWrapped("Mass, speed, turning and size variation are game parameters, not biological measurements. Small fish are visually enlarged.");
        ImGui::Separator();ImGui::Text("Scanned species: %d / %d",scene.discoveries(),SpeciesCount);
    }
    ImGui::End();
}
InputFrame Ui::build(Scene& s,const RenderStats& stats,const std::array<PhotoView,SpeciesCount>& photos,bool captured,float fps,float ms) {
    InputFrame request;if(!s.settings.ui)return request;
    auto& settings=s.settings;auto display=ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({18,18},ImGuiCond_Always);
    ImGui::SetNextWindowSize({360,0},ImGuiCond_Always);ImGui::SetNextWindowBgAlpha(.86f);
    const ImGuiWindowFlags hud=ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoNav;
    if(ImGui::Begin("Dive telemetry",nullptr,hud)) {
        ImGui::TextColored({.59f,.95f,.85f,1}, "%s", ProjectTitle);
        ImGui::TextWrapped("%s", TeamNames);
        ImGui::TextDisabled("%s", ProjectTechnique);
        ImGui::Text("Caustics [C]: %s | Pause [P]: %s", settings.caustics ? "ON" : "OFF", settings.paused ? "ON" : "OFF");
        ImGui::Separator();ImGui::Text("%.0f FPS  |  %.1f ms",fps,ms);
        ImGui::Text("Drawn: %d corals / %d fish",stats.corals,stats.fish);
        ImGui::Text("%d draws / %dK triangles",stats.draws,stats.triangles/1000);
        ImGui::TextDisabled("World X %.1f / Z %.1f",s.worldX(),s.worldZ());
        ImGui::Text("Survey: %d / %d | %.0f s", s.discoveries(), SpeciesCount, s.survey.remaining);
        ImGui::TextDisabled("F1 help | F5 reset survey | F9 record FPS");
        ImGui::TextColored({.97f,.82f,.45f,1},"%s",captured?"DIVE MODE - TAB releases the cursor":"CLICK A FISH TO INSPECT IT");
    }
    ImGui::End();
    ImGui::SetNextWindowPos({std::max(440.0f,display.x-358),18},ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({340,std::max(390.0f,display.y-80)},ImGuiCond_FirstUseEver);
    if(ImGui::Begin("Reef controls")) {
        if(ImGui::BeginTabBar("Controls")) {
            if (ImGui::BeginTabItem("Dive")) {
                ImGui::SeparatorText("Reef survey");
                ImGui::TextWrapped("Goal: scan all eight living reef species in three minutes. Press Tab to release the mouse, then click visible fish. The world stays explorable after the timer ends.");
                if (s.survey.state == SurveyState::Ready) {
                    if (ImGui::Button("Start survey [Enter]")) request.press(Action::StartSurvey);
                } else if (s.survey.state == SurveyState::Active) {
                    ImGui::Text("Time remaining: %.0f seconds", s.survey.remaining);
                    ImGui::ProgressBar(float(s.discoveries()) / SpeciesCount);
                } else {
                    ImGui::TextColored({1, .8f, .3f, 1}, "%s", s.survey.state == SurveyState::Complete ? "Survey complete!" : "Time is up. Try again.");
                    if (ImGui::Button("Start again [Enter]")) request.press(Action::StartSurvey);
                }
                if (ImGui::Button("Reset survey [F5]")) request.press(Action::RestartSurvey);
                ImGui::TextWrapped("Use P to pause, F2 for a coral close-up, and C to compare caustics. Scans count only during an active survey.");
                ImGui::SeparatorText("Experimental encounter");
                ImGui::TextWrapped("Megalodon is extinct. This optional animal is a fictional, stylized reconstruction, not a living reef species.");
                ImGui::TextWrapped("Natural chance: about 0.01%% per unpaused minute (mean wait 10,000 minutes). A manual spawn does not increase the survey score.");
                if (ImGui::Button("Spawn Megalodon")) request.press(Action::SpawnMegalodon);
                ImGui::SameLine();
                if (ImGui::Button("Remove")) request.press(Action::DismissMegalodon);
                if (s.megalodon.active) ImGui::Text("Encounter: %s", s.megalodon.manuallySpawned ? "manual" : "natural");
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Light")) {
                ImGui::SeparatorText("Day and night");
                ImGui::Checkbox("Automatic day-night cycle",&settings.dayCycle);
                ImGui::SliderFloat("Hour",&settings.hour,0,23.99f,"%.2f");
                ImGui::SliderFloat("Day length (s)",&settings.daySeconds,20,600,"%.0f");
                ImGui::SliderFloat("Light azimuth", &settings.lightAzimuth, -180, 180, "%.0f degrees");
                if(ImGui::Button("Noon"))settings.hour=12;
                ImGui::SameLine();
                if(ImGui::Button("Dusk"))settings.hour=18;
                ImGui::SameLine();
                if(ImGui::Button("Midnight"))settings.hour=0;
                ImGui::SeparatorText("Underwater visibility");
                ImGui::Checkbox("Distance fog [F]",&settings.fog);
                ImGui::SliderFloat("Visibility",&settings.visibility,24,96,"%.0f units");
                ImGui::SliderFloat("Fog starts",&settings.fogStart,0,settings.visibility-3,"%.0f units");
                ImGui::TextWrapped("Beyond the visibility limit, objects are culled. Fog reaches full opacity at that limit.");
                ImGui::SeparatorText("Shader comparisons");
                ImGui::Checkbox("Blinn-Phong [B]",&settings.lighting);ImGui::Checkbox("Caustics [C]",&settings.caustics);
                ImGui::Checkbox("Diffuse textures [T]",&settings.textures);ImGui::Checkbox("Normal maps [N]",&settings.normalMaps);
                ImGui::Checkbox("Bubbles",&settings.bubbles);ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("World")) {
                ImGui::Text("Seed: %u", settings.seed);
                if(ImGui::Button("New reef [R]"))request.press(Action::NewSeed);
                const char* qualities[]={"Low","Medium","High"};ImGui::Combo("Density",&settings.quality,qualities,3);
                ImGui::Text("Loaded chunks: %d",int(s.chunks.size()));ImGui::Text("Pending generation: %d",s.pendingChunks);
                ImGui::Text("Visible tiles: %d / GPU tiles: %d",stats.chunksDrawn,stats.gpuChunks);
                ImGui::Text("Objects culled: %d",stats.objectsCulled);
                ImGui::TextWrapped("New terrain, corals and rocks load as you swim. Old tiles leave memory. Returning to a tile reproduces its static layout.");
                ImGui::SeparatorText("Movement");
                ImGui::SliderFloat("Camera speed",&settings.swimSpeed,1,20,"%.1f");
                ImGui::SliderFloat("Fish speed scale",&settings.fishSpeed,.05f,3,"%.2fx");
                ImGui::Checkbox("Pause simulation [P]",&settings.paused);
                if(ImGui::Button("Return to origin [Home]"))request.press(Action::Home);
                if(ImGui::Button("Guided camera [G]"))request.press(Action::Tour);
                ImGui::Checkbox("Vertical sync [V]",&settings.vsync);ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Lab")) {
                ImGui::TextWrapped("Featured coral: translation, rotation and non-uniform scale.");
                ImGui::DragFloat3("Position",&s.featured.position.x,.05f,-30,30);
                ImGui::DragFloat3("Rotation (rad)",&s.featured.rotation.x,.02f,-Pi,Pi);
                ImGui::DragFloat3("Scale",&s.featured.size.x,.01f,.2f,3);
                if(ImGui::Button("Reset coral [Backspace]"))request.press(Action::ResetCoral);
                if(ImGui::Button("Focus coral [F2]"))request.press(Action::Focus);
                ImGui::Checkbox("Wireframe [M]",&settings.wireframe);
                ImGui::TextWrapped("Keyboard: 1 move / 2 rotate / 3 scale. J/L = X, K/I = Y, U/O = Z.");
                ImGui::SeparatorText("Techniques");
                ImGui::TextWrapped("MVP transforms; inverse-transpose normals; instancing; hierarchical fish; attraction-point coral growth; deterministic chunk streaming; GPU ID picking.");
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Log")) {
                ImGui::Text("Discovered: %d / %d",s.discoveries(),SpeciesCount);
                ImGui::TextWrapped("Press Tab, then click a fish in the scene. Coral and rocks block picking, just as they block visibility.");
                for(int i=0;i<SpeciesCount;++i) {
                    std::string label=std::string(s.discovered[std::size_t(i)]?"[found] ":"[  ?  ] ")+SpeciesList[std::size_t(i)].name;
                    if(ImGui::Selectable(label.c_str(),s.guideSpecies==i)){s.guideSpecies=i;s.showFishInfo=true;}
                }
                ImGui::TextWrapped("Photos and descriptions are references. The moving 3D fish are stylized procedural models.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::Separator();if(ImGui::Button("Controls / help"))s.showHelp=!s.showHelp;
    }
    ImGui::End();
    fieldGuide(s,photos);
    if(s.showHelp) {
        ImGui::SetNextWindowSize({480,330},ImGuiCond_Appearing);
        if(ImGui::Begin("Controls and scope",&s.showHelp)) {
            ImGui::TextWrapped("WASD: swim | Space / Ctrl: up / down | Shift: faster | Tab: release mouse | Mouse wheel: FOV | Home: origin | F2: focus coral | G: camera tour | F1: help | F5: reset survey | Enter: start | F9: FPS CSV | H: interface | F12: BMP capture | Esc: exit.");
            ImGui::Separator();ImGui::TextWrapped("Pause freezes fish, the clock and animated effects. The camera and streaming remain responsive.");
            ImGui::TextWrapped("This is a graphics simulation, not a marine biology or fluid-physics model. Fish steering uses random targets, separation, limited grouping and obstacle repulsion.");
            ImGui::TextWrapped("The performance overlay measures the complete frame, not GPU time. Capture runs are not real-time benchmarks.");
        }
        ImGui::End();
    }
    if (s.showMegalodonInfo) {
        ImGui::SetNextWindowSize({430, 260}, ImGuiCond_Appearing);
        if (ImGui::Begin("Experimental Megalodon", &s.showMegalodonInfo)) {
            ImGui::Text("Otodus megalodon (extinct)");
            ImGui::Separator();
            ImGui::TextWrapped("This is a fictional encounter in the modern reef. Its 3D shape, mass, scale and movement are artistic simulation choices, not fossil measurements.");
            ImGui::TextWrapped("No photograph of a living Megalodon exists. The field-guide requirement for real animal photos applies to the eight living reef species, not this optional extinct animal.");
            if (ImGui::Button("Copy museum source")) ImGui::SetClipboardText("https://australian.museum/learn/animals/fishes/otodus-megalodon/");
        }
        ImGui::End();
    }
    if(captured) {
        auto* draw=ImGui::GetForegroundDrawList();ImVec2 c{display.x*.5f,display.y*.5f};
        draw->AddLine({c.x-5,c.y},{c.x+5,c.y},IM_COL32(180,240,224,180));
        draw->AddLine({c.x,c.y-5},{c.x,c.y+5},IM_COL32(180,240,224,180));
    }
    return request;
}
