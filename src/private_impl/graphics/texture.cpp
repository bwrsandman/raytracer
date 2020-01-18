#include "texture.h"

#include <cassert>

#include <array>

#include <glad/glad.h>

struct TextureFormatLookUp
{
  const uint32_t format;
  const uint32_t internal_format;
  const uint32_t type;
};

std::array<TextureFormatLookUp, 4> TextureFormatLookUpTable = {
  /* rgb32f  */ TextureFormatLookUp{ GL_RGB, GL_RGB32F, GL_FLOAT },
  /* rgba32f */ TextureFormatLookUp{ GL_RGBA, GL_RGBA32F, GL_FLOAT },
  /* rgba32u */
  TextureFormatLookUp{ GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT },
  /* rgba32i */
  TextureFormatLookUp{ GL_RGBA_INTEGER, GL_RGBA32I, GL_INT },
};

std::array<uint32_t, 2> TextureFilterLookUpTable = {
  /* linear  */ GL_LINEAR,
  /* nearest */ GL_NEAREST,
};

using namespace Raytracer::Graphics;

std::unique_ptr<Texture>
Raytracer::Graphics::Texture::create(uint32_t width,
                                     uint32_t height,
                                     MipMapFilter filter,
                                     Format format)
{
  uint32_t texture = 0;
  uint32_t sampler = 0;
  glGenTextures(1, &texture);
  glGenSamplers(1, &sampler);

  int32_t gl_filter = TextureFilterLookUpTable[static_cast<uint32_t>(filter)];
  int32_t clamp_to_edge = GL_CLAMP_TO_EDGE;
  glSamplerParameteriv(sampler, GL_TEXTURE_WRAP_S, &clamp_to_edge);
  glSamplerParameteriv(sampler, GL_TEXTURE_WRAP_T, &clamp_to_edge);
  glSamplerParameteriv(sampler, GL_TEXTURE_MIN_FILTER, &gl_filter);
  glSamplerParameteriv(sampler, GL_TEXTURE_MAG_FILTER, &gl_filter);

  auto gl_format = TextureFormatLookUpTable[static_cast<uint32_t>(format)];

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               gl_format.internal_format,
               width,
               height,
               0,
               gl_format.format,
               gl_format.type,
               nullptr);

  return std::unique_ptr<Texture>(
    new Texture(texture, sampler, width, height, format));
}

Texture::Texture(uint32_t native_texture,
                 uint32_t native_sampler,
                 uint32_t width,
                 uint32_t height,
                 Format format)
  : native_texture(native_texture)
  , native_sampler(native_sampler)
  , width(width)
  , height(height)
  , format(format)
{}

Texture::~Texture()
{
  glDeleteTextures(1, &native_texture);
  glDeleteSamplers(1, &native_sampler);
}

void
Texture::set_debug_name([[maybe_unused]] const std::string& name) const
{
#if !__EMSCRIPTEN__
  glObjectLabel(GL_TEXTURE, native_texture, -1, (name + " texture").c_str());
  glObjectLabel(GL_SAMPLER, native_sampler, -1, (name + " sampler").c_str());
#endif
}

void
Texture::bind(uint32_t slot) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, native_texture);
  glBindSampler(slot, native_sampler);
}

void
Texture::upload(const void* data, [[maybe_unused]] uint32_t size) const
{
  assert(size == width * height * sizeof(float) * 3);
  glBindTexture(GL_TEXTURE_2D, native_texture);
  auto gl_format = TextureFormatLookUpTable[static_cast<uint32_t>(format)];
  glTexSubImage2D(GL_TEXTURE_2D,
                  0,
                  0,
                  0,
                  width,
                  height,
                  gl_format.format,
                  gl_format.type,
                  data);
}
