add_library(glsltoy SHARED
    plugin.c
    backgroundToy.c
    )
target_include_directories(glsltoy SYSTEM
    PRIVATE "${CMAKE_SOURCE_DIR}/include" "${CMAKE_BINARY_DIR}/include"
    )
target_link_libraries(glsltoy
    common
    )
