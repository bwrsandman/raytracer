#include "ui.h"

#include <imgui.h>

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_sdl.h>

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
Ui::run() const
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  // Demo window, replace this with actual rendering
  ImGui::ShowDemoWindow();

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
