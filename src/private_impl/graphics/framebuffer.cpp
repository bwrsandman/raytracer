#include "framebuffer.h"

#include <glad/glad.h>

#include "texture.h"
#include "math/vec4.h"

using namespace Raytracer::Graphics;

std::unique_ptr<Framebuffer>
Raytracer::Graphics::Framebuffer::create(
    const std::unique_ptr<Texture>* textures, uint8_t size)
{
  uint32_t frameBuffer = 0;
  glGenFramebuffers(1, &frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

  std::vector<uint32_t> bufs;
  bufs.resize(size);

  for (uint8_t i = 0; i < size; ++i) {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D,
                           textures[i]->native_texture,
                           0);
    bufs[i] = GL_COLOR_ATTACHMENT0 + i;
  }
  glDrawBuffers(static_cast<GLsizei>(bufs.size()), bufs.data());

  return std::unique_ptr<Framebuffer>(new Framebuffer(frameBuffer, size));
}

std::unique_ptr<Framebuffer>
Framebuffer::default_framebuffer()
{
  return std::unique_ptr<Framebuffer>(new Framebuffer(0, 0));
}

Framebuffer::Framebuffer(uint32_t native_handle,
                         uint8_t size)
  : native_handle(native_handle)
  , size(size)
{}

Framebuffer::~Framebuffer()
{
  if (native_handle > 0) {
    glDeleteFramebuffers(1, &native_handle);
  }
}

void
Framebuffer::clear(const std::vector<vec4>& color) const
{
  bind();
  for (uint8_t i = 0; i < size; ++i) {
    glClearBufferfv(GL_COLOR, i, color[i].e);
  }
}

void
Framebuffer::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, native_handle);
}
