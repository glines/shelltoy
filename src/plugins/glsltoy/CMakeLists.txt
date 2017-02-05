add_library(glsltoy SHARED
    backgroundToy.c
    plugin.c
    textToy.c
    )
target_link_libraries(glsltoy
    common
    )
