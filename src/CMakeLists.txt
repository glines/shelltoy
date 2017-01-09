add_executable(shelltoy
    boundingBox.c
    collisionDetection.c
    config.c
    error.c
    fonts.c
    glyphAtlas.c
    glyphRenderer.c
    logging.c
    main.c
    naiveCollisionDetection.c
    profile.c
    pty.c
    screenRenderer.c
    terminal.c
    )
target_link_libraries(shelltoy
    common
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${JANSSON_LIBRARIES}
    ${LIBTSM_LIBRARY}
    ${SDL2_LIBRARY}
    )
set_property(TARGET shelltoy PROPERTY C_STANDARD 11)
install(TARGETS shelltoy
    RUNTIME DESTINATION bin)

add_subdirectory("./common")
add_subdirectory("./toys")
