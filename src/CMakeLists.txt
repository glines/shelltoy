add_executable(ttoy
    backgroundRenderer.c
    backgroundToy.c
    backgroundToyDictionary.c
    boundingBox.c
    collisionDetection.c
    config.c
    error.c
    fileWatcher.c
    font.c
    fontRef.c
    fontRefArray.c
    fonts.c
    glyphAtlas.c
    glyphRenderer.c
    glyphRendererRef.c
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
    textToyDictionary.c
    toyFactory.c
    )
add_dependencies(ttoy ttoy_version)
# The --dynamic-list linker flag tells the linker which symbols need to be
# exported for use in plugins loaded with dlopen(3).
set_target_properties(ttoy
    PROPERTIES LINK_FLAGS "-Wl,--dynamic-list=${CMAKE_CURRENT_SOURCE_DIR}/dynamic_list.txt"
    )
target_link_libraries(ttoy
    common
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${JANSSON_LIBRARIES}
    ${LIBTSM_LIBRARY}
    ${SDL2_LIBRARY}
    ${X11_LIBRARIES}
    )
set_property(TARGET ttoy PROPERTY C_STANDARD 11)
install(TARGETS ttoy
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include")

add_subdirectory("./common")
add_subdirectory("./plugins")
