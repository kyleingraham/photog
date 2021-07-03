include(CMakePackageConfigHelpers)
include(GNUInstallDirs) # Make CMAKE_INSTALL_<dir> variables available

add_subdirectory(photog)

# Project Install
## Target destinations for project
## https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets
install(TARGETS
        color
        ${COLOR_HALIDE_LIBRARIES}
        definitions
        ${SHARED_HALIDE_RUNTIME}
        EXPORT photog_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/photog
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog/private
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog/public/photog # Copied to this folder
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog/public) # Included from this folder

## Export details for project
install(EXPORT photog_targets
        FILE ${CMAKE_PROJECT_NAME}Targets.cmake
        NAMESPACE photog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog/cmake/photog) # https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog/cmake/photog)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
        VERSION "${${CMAKE_PROJECT_NAME}_VERSION}"
        COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog/cmake/photog)