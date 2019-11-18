#pragma once

#include "pipeline.h"

class PipelineRasterOpenGL : public Pipeline
{
public:
  explicit PipelineRasterOpenGL(uint32_t program);
  ~PipelineRasterOpenGL() override;

  void bind() override;

  /// A factory function in the impl class allows for an error to return null
  static std::unique_ptr<Pipeline> create(const PipelineCreateInfo& info);

private:
  const uint32_t program;
};
