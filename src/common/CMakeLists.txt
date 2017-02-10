add_library(common
    array.c
    dictionary.c
    glError.c
    mkdir.c
    shader.c
    shaders.c
    )
target_include_directories(common
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
    )

# Convert all of our shaders to C header files
file(GLOB_RECURSE shaders
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.frag"
    )
foreach(shader ${shaders})
  file(RELATIVE_PATH shader_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${shader}")
  string(REPLACE "/" "_" shader_target "${shader_rel}.c")
  get_filename_component(shader_dir "${shader_rel}" DIRECTORY)
  add_custom_target("${shader_target}"
      COMMAND xxd -include ${shader_rel} > "${CMAKE_CURRENT_BINARY_DIR}/${shader_target}"
      DEPENDS "${shader}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      )
  add_dependencies(common
      "${shader_target}"
      )
endforeach()
