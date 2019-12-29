#pragma once

#include "pipeline.h"

namespace Raytracer::Graphics {
class PipelineRasterOpenGL : public Pipeline
{
public:
  explicit PipelineRasterOpenGL(uint32_t program);
  ~PipelineRasterOpenGL() override;

  void bind() override;
  uint32_t get_native_handle() const override;

  /// A factory function in the impl class allows for an error to return null
  static std::unique_ptr<Pipeline> create(const PipelineCreateInfo& info);

private:
  const uint32_t program;
};
} // namespace Raytracer::Graphics
