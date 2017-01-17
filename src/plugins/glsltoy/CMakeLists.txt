add_library(glsltoy SHARED
    plugin.c
    toy.c
    )
target_link_libraries(glsltoy
    common
    )
