add_executable(test_ttoy
    ../src/backgroundRenderer.c
    ../src/backgroundToy.c
    ../src/backgroundToyDictionary.c
    ../src/boundingBox.c
    ../src/collisionDetection.c
    ../src/config.c
    ../src/config.c
    ../src/error.c
    ../src/font.c
    ../src/fontRef.c
    ../src/fontRefArray.c
    ../src/fonts.c
    ../src/glyphAtlas.c
    ../src/glyphRenderer.c
    ../src/glyphRendererRef.c
    ../src/logging.c
    ../src/naiveCollisionDetection.c
    ../src/plugin.c
    ../src/pluginDictionary.c
    ../src/profile.c
    ../src/pty.c
    ../src/terminal.c
    ../src/textRenderer.c
    ../src/textToy.c
    ../src/textToyDictionary.c
    ../src/toyFactory.c
    test_ttoy.c
    test_ttoy_BoundingBox.c
    test_ttoy_Config.c
    test_ttoy_Terminal.c
    )
target_link_libraries(test_ttoy
    ${CHECK_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${X11_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${SDL2_LIBRARY}
    common
    dl
    tsm
    )
add_test(NAME test_ttoy_BoundingBox
    COMMAND test_ttoy BoundingBox)
add_test(NAME test_ttoy_Config
    COMMAND test_ttoy Config)
add_test(NAME test_ttoy_Terminal
    COMMAND test_ttoy Terminal)
