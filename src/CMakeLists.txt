add_executable(shelltoy
    fonts.c
    main.c
    )
target_link_libraries(shelltoy
    ${FREETYPE_LIBRARIES}
    ${SDL2_LIBRARY}
    )
