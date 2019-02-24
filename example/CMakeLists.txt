cmake_minimum_required(VERSION 3.5)
project(soundstone_example LANGUAGES CXX VERSION 0.1)

file(GLOB_RECURSE source_files "./src/*.cpp" "./src/*.hpp")
add_executable(soundstone_example ${source_files})
target_link_libraries(soundstone_example soundstone)
