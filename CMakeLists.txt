cmake_minimum_required(VERSION 3.5)
project(soundstone LANGUAGES CXX VERSION 0.1)

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

enable_testing()
find_package(GTest REQUIRED)
find_package(cubeb CONFIG REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
# find_library(CUBEB_LIBRARY NAMES cubeb)

# Source files
file(GLOB_RECURSE SOUNDSTONE_SOURCE_FILES
    "./src/*.cpp" "./src/*.hpp"
    "./include/soundstone/*.hpp" "./include/soundstone_internal/*.hpp"
)
file(GLOB_RECURSE SOUNDSTONE_TEST_SOURCE_FILES "./test/*")

# Targets
add_library(soundstone ${SOUNDSTONE_SOURCE_FILES})
add_library(soundstone_testable ${SOUNDSTONE_SOURCE_FILES})
add_executable(soundstone_test ${SOUNDSTONE_TEST_SOURCE_FILES})

# Pre-define SOUNDSTONE_TESTABLE_EXPORT so testable exports aren't exported for
# main target.
target_compile_definitions(soundstone PRIVATE SOUNDSTONE_TESTABLE_EXPORT=)

set_target_properties(soundstone PROPERTIES CXX_VISIBILITY_PRESET hidden)

generate_export_header(soundstone
    BASE_NAME SOUNDSTONE
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/soundstone/export.h"
)

generate_export_header(soundstone
    BASE_NAME SOUNDSTONE_TESTABLE
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/soundstone/testable_export.h"
)

set_target_properties(soundstone soundstone_testable soundstone_test
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(soundstone
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include>
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include>
    PUBLIC
        $<INSTALL_INTERFACE:include/soundstone>
)
target_include_directories(soundstone_testable
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include>
)
target_include_directories(soundstone_test
    PRIVATE
        # TODO: Get these directly from soundstone target
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include>
)

target_link_libraries(soundstone cubeb::cubeb)
target_link_libraries(soundstone_testable cubeb::cubeb)
target_link_libraries(soundstone_test
    GTest::GTest GTest::Main ${GMOCK_LIBRARY} soundstone_testable
)

# Tests
add_test(SoundstoneTests soundstone_test)

# Install
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(version_config "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Configure '${PROJECT_NAME}ConfigVersion.cmake'
# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file(
    "${version_config}"
    COMPATIBILITY SameMajorVersion
)

# Configure '${PROJECT_NAME}Config.cmake'
# Use variables:
#   * targets_export_name
configure_package_config_file(
    "Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

install(
    TARGETS soundstone
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/soundstone"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soundstone"
)
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/soundstone"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soundstone"
)
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION ${config_install_dir}
)
install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

add_subdirectory(example)

