add_executable(shelltoy
    backgroundRenderer.c
    backgroundToy.c
    backgroundToyDictionary.c
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
    plugin.c
    pluginDictionary.c
    profile.c
    pty.c
    terminal.c
    textRenderer.c
    textToy.c
    toyFactory.c
    )
add_dependencies(shelltoy shelltoy_version)
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
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include")

add_subdirectory("./common")
add_subdirectory("./plugins")
