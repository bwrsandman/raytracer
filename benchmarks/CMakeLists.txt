enable_testing()

file(GLOB BENCHMARK_FILES CONFIGURE_DEPENDS *.cpp)

file(GLOB_RECURSE textures CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/assets/*.jpg
  ${CMAKE_SOURCE_DIR}/assets/*.png
  ${CMAKE_SOURCE_DIR}/assets/*.tga
  ${CMAKE_SOURCE_DIR}/assets/*.gltf
)

foreach(texture ${textures})
  get_filename_component(texture_name ${texture} NAME)
  set(texture_output ${CMAKE_CURRENT_BINARY_DIR}/${texture_name})
  add_custom_command(
    OUTPUT ${texture_output}
    COMMAND ${CMAKE_COMMAND} -E copy ${texture} ${texture_output}
    DEPENDS ${texture}
  )
  list(APPEND BENCHMARK_FILES ${texture_output})
endforeach()

find_package(benchmark REQUIRED benchmark_main)
add_executable(RayBenchmark ${BENCHMARK_FILES})
target_link_libraries(RayBenchmark PRIVATE benchmark::benchmark_main RaytracerLib)
set_property(TARGET RayBenchmark PROPERTY CXX_STANDARD 17)
