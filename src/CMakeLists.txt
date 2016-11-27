add_executable(shelltoy
    boundingBox.c
    collisionDetection.c
    config.c
    fonts.c
    glyphAtlas.c
    glyphRenderer.c
    main.c
    naiveCollisionDetection.c
    pty.c
    render.c
    screenRenderer.c
    terminal.c
    )
target_link_libraries(shelltoy
    common
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${JANSSON_LIBRARIES}
    ${LIBTSM_LIBRARY}
    ${SDL2_LIBRARY}
    )
install(TARGETS shelltoy
    RUNTIME DESTINATION bin)

add_subdirectory("./common")
add_subdirectory("./toys")
