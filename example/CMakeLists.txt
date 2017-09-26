cmake_minimum_required(VERSION 3.5)
project(soundstone_example LANGUAGES CXX VERSION 0.1)

file(GLOB_RECURSE SOUNDSTONE_EXAMPLE_SOURCE_FILES "./src/*.cpp" "./src/*.hpp")
add_executable(soundstone_example ${SOUNDSTONE_EXAMPLE_SOURCE_FILES})
target_link_libraries(soundstone_example soundstone
    "-framework AudioUnit" "-framework CoreAudio" "-framework CoreServices"
)
target_include_directories(soundstone_example PRIVATE
    $<TARGET_PROPERTY:soundstone,INTERFACE_INCLUDE_DIRECTORIES>
)
set_target_properties(soundstone_example
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
)
