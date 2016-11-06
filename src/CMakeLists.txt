add_executable(shelltoy
    fonts.c
    main.c
    pty.c
    render.c
    terminal.c
    )
target_link_libraries(shelltoy
    common
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${LIBTSM_LIBRARY}
    ${SDL2_LIBRARY}
    )

add_subdirectory("./common")
add_subdirectory("./toys")
