#include "ui.h"

#include <imgui.h>

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

#include "hittable/line_segment.h"
#include "hittable/object_list.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "scene.h"

Ui::Ui(SDL_Window* window)
  : window(window)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
  ImGui_ImplOpenGL3_Init();
}

void
Ui::run(Scene& scene) const
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  if (ImGui::Begin("Configuration ")) {
    if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& light_list = dynamic_cast<ObjectList&>(scene.get_lights()).list;
      uint32_t i = 0;
      std::vector<std::vector<std::unique_ptr<Object>>::iterator> remove_index;
      for (auto itr = light_list.begin(); itr < light_list.end(); ++itr) {
        auto& light = *itr;
        if (light) {
          ImGui::PushID(i);
          if (auto point = dynamic_cast<Point*>(light.get())) {
            ImGui::Text("%u. Point Light", i + 1);
            ImGui::InputFloat3("position",
                               reinterpret_cast<float*>(&point->position));
            ImGui::InputScalar("mat_id", ImGuiDataType_U16, &point->mat_id);
          } else if (auto line_segment =
                       dynamic_cast<LineSegment*>(light.get())) {
            ImGui::Text("%u. Line Light", i + 1);
            ImGui::InputFloat3(
              "start", reinterpret_cast<float*>(&line_segment->position[0]));
            ImGui::InputFloat3(
              "end", reinterpret_cast<float*>(&line_segment->position[1]));
            ImGui::InputScalar(
              "mat_id", ImGuiDataType_U16, &line_segment->mat_id);
          } else {
            ImGui::Text("%u. Light(%s)", i + 1, typeid(*light).name());
          }
          ++i;
          if (ImGui::Button("Remove")) {
            remove_index.push_back(itr);
          }
          ImGui::PopID();
        }
      }
      for (auto itr : remove_index) {
        light_list.erase(itr);
      }

      if (ImGui::Button("New Line")) {
        const vec3 position[2] = {
          vec3{ -1, 0, 0 },
          vec3{ 1, 0, 0 },
        };
        light_list.emplace_back(new LineSegment(position, 0));
      }
      if (ImGui::Button("New Point")) {
        const vec3 position = { 0, 0, 0 };
        light_list.emplace_back(new Point(position, 0));
      }
      if (ImGui::Button("New Sphere")) {
        const vec3 position = { 0, 0, 0 };
        light_list.emplace_back(new Sphere(position, 1, 0));
      }
    }
  }
  ImGui::End();

  // Rendering
  ImGui::Render();
}

void
Ui::draw() const
{
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
Ui::process_event(const SDL_Event& event)
{
  ImGui_ImplSDL2_ProcessEvent(&event);
}
