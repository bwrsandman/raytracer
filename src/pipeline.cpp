#include "pipeline.h"

#include "private_impl/pipelines/pipeline_raster_opengl.h"

using Raytracer::Graphics::Pipeline;
using Raytracer::Graphics::PipelineCreateInfo;

std::unique_ptr<Pipeline>
Pipeline::create(Type type, const PipelineCreateInfo& info)
{
  switch (type) {
    case Pipeline::Type::RaterOpenGL:
      return PipelineRasterOpenGL::create(info);
  }
  return nullptr;
}
