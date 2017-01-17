add_library(glsltoy SHARED
    plugin.c
    backgroundToy.c
    )
target_link_libraries(glsltoy
    common
    )
