add_library(glsltoy SHARED
    glsltoy.c
    toy.c
    )
target_link_libraries(glsltoy
    common
    )
