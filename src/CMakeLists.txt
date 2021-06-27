include(GNUInstallDirs) # Make CMAKE_INSTALL_<dir> variables available

add_subdirectory(photog)

# Project Install
## Target destinations for project
install(TARGETS
        color
        ${COLOR_HALIDE_LIBRARIES}
        ${SHARED_HALIDE_RUNTIME}
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