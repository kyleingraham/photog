include(GNUInstallDirs) # Make CMAKE_INSTALL_<dir> variables available

add_subdirectory(generator)

add_subdirectory(utils)

add_subdirectory(color)

# Project Install
## Target destinations for project
install(TARGETS
        color
        photog_color
        generator
        utils
        ${COLOR_GENERATORS}
        ${COLOR_SHARED_RUNTIME}
        EXPORT photog_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/photog
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog)

## Export details for project
install(EXPORT photog_targets
        FILE ${CMAKE_PROJECT_NAME}Targets.cmake
        NAMESPACE photog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog/cmake/photog) # https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure