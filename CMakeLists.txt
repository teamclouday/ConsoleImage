cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ConsoleImage)

add_definitions(-DUNICODE)
add_definitions(-DGLEW_STATIC)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -fpermissive")
endif()

find_package(OpenGL REQUIRED)

option(glew-cmake_BUILD_SHARED "Build the shared glew library" OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glew-cmake)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/argparse)
option(CPPTERMINAL_BUILD_EXAMPLES "Set to ON to build examples" OFF)
option(CPPTERMINAL_ENABLE_INSTALL "Set to ON to enable install" OFF)
option(CPPTERMINAL_ENABLE_TESING "Set to ON to enable testing" OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/cpp-terminal)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.hpp)
add_executable(show ${SRC_FILES})

set_target_properties(show
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
)

target_include_directories(show PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/glew-cmake/include
    ${CMAKE_SOURCE_DIR}/external/stb
)

target_link_libraries(show PRIVATE
    libglew_static
    OpenGL::GL
    argparse
    cpp-terminal
)

if(WIN32)
target_link_libraries(show PRIVATE
    user32
)
endif(WIN32)