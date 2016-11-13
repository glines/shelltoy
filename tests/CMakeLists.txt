add_executable(check_boundingBox
    check_boundingBox.c
    ../src/boundingBox.c
    )
target_link_libraries(check_boundingBox
    ${CHECK_LIBRARIES}
    )
