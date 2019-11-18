#include "pipeline_raster_opengl.h"

#include <glad/glad.h>

PipelineRasterOpenGL::PipelineRasterOpenGL(uint32_t program)
  : program(program)
{}

PipelineRasterOpenGL::~PipelineRasterOpenGL()
{
  glDeleteProgram(program);
}

void
PipelineRasterOpenGL::bind()
{
  glUseProgram(program);
}

std::unique_ptr<Pipeline>
PipelineRasterOpenGL::create(const PipelineCreateInfo& info)
{
  int32_t is_compiled = 0;
  uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderBinary(1,
                 &vertex_shader,
                 GL_SHADER_BINARY_FORMAT_SPIR_V,
                 info.vertex_shader_binary,
                 info.vertex_shader_size);
  glSpecializeShader(
    vertex_shader, info.vertex_shader_entry_point.c_str(), 0, nullptr, nullptr);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
  if (!is_compiled) {
    glDeleteShader(vertex_shader);
    return nullptr;
  }
  uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderBinary(1,
                 &fragment_shader,
                 GL_SHADER_BINARY_FORMAT_SPIR_V,
                 info.fragment_shader_binary,
                 info.fragment_shader_size);
  glSpecializeShader(fragment_shader,
                     info.fragment_shader_entry_point.c_str(),
                     0,
                     nullptr,
                     nullptr);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled == 0) {
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return nullptr;
  }

  uint32_t program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  int32_t is_linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if (is_linked) {
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(program);
    return nullptr;
  }

  return std::make_unique<PipelineRasterOpenGL>(program);
}
