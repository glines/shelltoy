add_executable(check_boundingBox
    check_boundingBox.c
    ../src/boundingBox.c
    )
target_link_libraries(check_boundingBox
    ${CHECK_LIBRARIES}
    )
add_test(check_boundingBox_test check_boundingBox)

add_executable(check_config
    ../src/config.c
    ../src/error.c
    ../src/fonts.c
    ../src/logging.c
    ../src/profile.c
    check_config.c
    )
target_link_libraries(check_config
    ${CHECK_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${JANSSON_LIBRARIES}
    )
add_test(check_config_test check_config)
