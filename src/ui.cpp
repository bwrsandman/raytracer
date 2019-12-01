#include "ui.h"

#include <imgui.h>

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

#include "camera.h"
#include "hittable/line_segment.h"
#include "hittable/object_list.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "material.h"
#include "materials/dielectric.h"
#include "materials/emissive.h"
#include "materials/emissive_linear_drop_off.h"
#include "materials/emissive_quadratic_drop_off.h"
#include "materials/lambert.h"
#include "materials/metal.h"
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
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Use WASD to move the camera and\n"
                  "Arrow keys for Panning and Tilting.\n"
                  "The shift key increases speed and\n"
                  "the CTRL key decreases.");
      auto& camera = scene.get_camera();
      bool dirty = false;

      auto fov = camera.v_fov;
      ImGui::SliderFloat("Vertical Field of View", &camera.v_fov, 1, 179);
      dirty |= fov != camera.v_fov;

      auto aspect = camera.screen_aspect;
      ImGui::InputFloat("Aspect Ratio", &aspect);
      if (aspect != camera.screen_aspect && aspect > 0 &&
          aspect < std::numeric_limits<float>::infinity()) {
        dirty = true;
        camera.screen_aspect = aspect;
      }

      vec3 origin = camera.origin;
      ImGui::InputFloat3("Origin", reinterpret_cast<float*>(&camera.origin));
      dirty |= origin != camera.origin;

      if (dirty) {
        camera.calculate_camera();
      }
    }
    if (ImGui::CollapsingHeader("Materials")) {
      auto& material_list = scene.get_material_list();
      uint32_t i = 0;
      std::vector<std::vector<std::unique_ptr<Material>>::iterator>
        remove_index;
      for (auto itr = material_list.begin(); itr < material_list.end(); ++itr) {
        auto& light = *itr;
        if (light) {
          ImGui::PushID(i);
          if (auto mat = dynamic_cast<Dielectric*>(light.get())) {
            ImGui::Text("%u. Dielectric", i);
          } else if (auto mat = dynamic_cast<Emissive*>(light.get())) {
            ImGui::Text("%u. Emissive (No Drop Off)", i);
            ImGui::InputFloat3("albedo",
                               reinterpret_cast<float*>(&mat->albedo));
          } else if (auto mat =
                       dynamic_cast<EmissiveLinearDropOff*>(light.get())) {
            ImGui::Text("%u. Emissive (Linear Drop Off)", i);
            ImGui::InputFloat3("albedo",
                               reinterpret_cast<float*>(&mat->albedo));
            ImGui::InputFloat("drop-off factor",
                              reinterpret_cast<float*>(&mat->drop_off_factor));
          } else if (auto mat =
                       dynamic_cast<EmissiveQuadraticDropOff*>(light.get())) {
            ImGui::Text("%u. Emissive (Quadratic Drop Off)", i);
            ImGui::InputFloat3("albedo",
                               reinterpret_cast<float*>(&mat->albedo));
            ImGui::InputFloat("drop-off factor",
                              reinterpret_cast<float*>(&mat->drop_off_factor));
          } else if (auto mat = dynamic_cast<Lambert*>(light.get())) {
            ImGui::Text("%u. Lambert (Shadow Ray)", i);
            ImGui::InputFloat3("albedo",
                               reinterpret_cast<float*>(&mat->albedo));
          } else if (auto mat = dynamic_cast<Metal*>(light.get())) {
            ImGui::Text("%u. Metal", i);
            ImGui::InputFloat3("albedo",
                               reinterpret_cast<float*>(&mat->albedo));
          } else {
            ImGui::Text("%u. Material", i);
          }
          ImGui::PopID();
          ++i;
          if (ImGui::Button("Remove##materials")) {
            remove_index.push_back(itr);
          }
        }
      }
      for (auto itr : remove_index) {
        material_list.erase(itr);
      }

      if (ImGui::Button("New Dielectric")) {
        material_list.emplace_back(new Dielectric(1.0f, 1.0f));
      }
      if (ImGui::Button("Emissive (No Drop Off)")) {
        const vec3 albedo(1.0f, 1.0f, 1.0f);
        material_list.emplace_back(new Emissive(albedo));
      }
      if (ImGui::Button("Emissive (Linear Drop Off)")) {
        const vec3 albedo(1.0f, 1.0f, 1.0f);
        material_list.emplace_back(new EmissiveLinearDropOff(albedo, 1.0f));
      }
      if (ImGui::Button("Emissive (Quadratic Drop Off)")) {
        const vec3 albedo(1.0f, 1.0f, 1.0f);
        material_list.emplace_back(new EmissiveQuadraticDropOff(albedo, 1.0f));
      }
      if (ImGui::Button("Lambert (Shadow Ray)")) {
        const vec3 albedo(1.0f, 1.0f, 1.0f);
        material_list.emplace_back(new Lambert(albedo));
      }
      if (ImGui::Button("Metal")) {
        const vec3 albedo(1.0f, 1.0f, 1.0f);
        material_list.emplace_back(new Metal(albedo));
      }
    }
    if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& geometry_list = dynamic_cast<ObjectList&>(scene.get_world()).list;
      uint32_t i = 0;
      std::vector<std::vector<std::unique_ptr<Object>>::iterator> remove_index;
      for (auto itr = geometry_list.begin(); itr < geometry_list.end(); ++itr) {
        auto& light = *itr;
        if (light) {
          ImGui::PushID(i);
          if (auto point = dynamic_cast<Point*>(light.get())) {
            ImGui::Text("%u. Point", i + 1);
            ImGui::InputFloat3("position",
                               reinterpret_cast<float*>(&point->position));
            ImGui::InputScalar("mat_id", ImGuiDataType_U16, &point->mat_id);
          } else if (auto line_segment =
                       dynamic_cast<LineSegment*>(light.get())) {
            ImGui::Text("%u. Line", i + 1);
            ImGui::InputFloat3(
              "start", reinterpret_cast<float*>(&line_segment->position[0]));
            ImGui::InputFloat3(
              "end", reinterpret_cast<float*>(&line_segment->position[1]));
            ImGui::InputScalar(
              "mat_id", ImGuiDataType_U16, &line_segment->mat_id);
          } else if (auto sphere = dynamic_cast<Sphere*>(light.get())) {
            ImGui::Text("%u. Sphere", i + 1);
            ImGui::InputFloat3("center",
                               reinterpret_cast<float*>(&sphere->center));
            ImGui::InputFloat("radius",
                              reinterpret_cast<float*>(&sphere->radius));
            ImGui::InputScalar("mat_id", ImGuiDataType_U16, &sphere->mat_id);
          } else {
            ImGui::Text("%u. %s", i + 1, typeid(*light).name());
          }
          ++i;
          if (ImGui::Button("Remove##geometry")) {
            remove_index.push_back(itr);
          }
          ImGui::PopID();
        }
      }
      for (auto itr : remove_index) {
        geometry_list.erase(itr);
      }

      if (ImGui::Button("New Line")) {
        const vec3 position[2] = {
          vec3{ -1, 0, 0 },
          vec3{ 1, 0, 0 },
        };
        geometry_list.emplace_back(new LineSegment(position, 0));
      }
      if (ImGui::Button("New Point")) {
        const vec3 position = { 0, 0, 0 };
        geometry_list.emplace_back(new Point(position, 0));
      }
      if (ImGui::Button("New Sphere")) {
        const vec3 position = { 0, 0, 0 };
        geometry_list.emplace_back(new Sphere(position, 1, 0));
      }
    }
    if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto& light_list = scene.get_light_indices();
      auto& geometry_list = dynamic_cast<ObjectList&>(scene.get_world()).list;
      uint32_t i = 0;
      std::vector<std::vector<std::unique_ptr<Object>>::iterator> remove_index;
      for (auto index : light_list) {
        auto& light = geometry_list[i];
        if (light) {
          ImGui::PushID(i);
          if (auto point = dynamic_cast<Point*>(light.get())) {
            ImGui::Text("%u. Point Light", i + 1);
            ImGui::InputFloat3("position##light",
                               reinterpret_cast<float*>(&point->position));
            ImGui::InputScalar(
              "mat_id##light", ImGuiDataType_U16, &point->mat_id);
          } else if (auto line_segment =
                       dynamic_cast<LineSegment*>(light.get())) {
            ImGui::Text("%u. Line Light", i + 1);
            ImGui::InputFloat3(
              "start##light",
              reinterpret_cast<float*>(&line_segment->position[0]));
            ImGui::InputFloat3(
              "end##light",
              reinterpret_cast<float*>(&line_segment->position[1]));
            ImGui::InputScalar(
              "mat_id##light", ImGuiDataType_U16, &line_segment->mat_id);
          } else if (auto sphere = dynamic_cast<Sphere*>(light.get())) {
            ImGui::Text("%u. Sphere Light", i + 1);
            ImGui::InputFloat3("center##light",
                               reinterpret_cast<float*>(&sphere->center));
            ImGui::InputFloat("radius##light",
                              reinterpret_cast<float*>(&sphere->radius));
            ImGui::InputScalar(
              "mat_id##light", ImGuiDataType_U16, &sphere->mat_id);
          } else {
            ImGui::Text("%u. Light(%s)", i + 1, typeid(*light).name());
          }
          ++i;
          if (ImGui::Button("Remove##light")) {
            remove_index.push_back(geometry_list.begin() + i);
          }
          ImGui::PopID();
        }
      }
      for (auto itr : remove_index) {
        geometry_list.erase(itr);
      }

      if (ImGui::Button("New Line##light")) {
        const vec3 position[2] = {
          vec3{ -1, 0, 0 },
          vec3{ 1, 0, 0 },
        };
        light_list.emplace_back(geometry_list.size());
        geometry_list.emplace_back(new LineSegment(position, 0));
      }
      if (ImGui::Button("New Point##light")) {
        const vec3 position = { 0, 0, 0 };
        light_list.emplace_back(geometry_list.size());
        geometry_list.emplace_back(new Point(position, 0));
      }
      if (ImGui::Button("New Sphere##light")) {
        const vec3 position = { 0, 0, 0 };
        light_list.emplace_back(geometry_list.size());
        geometry_list.emplace_back(new Sphere(position, 1, 0));
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
