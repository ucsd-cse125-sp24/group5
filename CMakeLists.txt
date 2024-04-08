# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
project(group5)

set(CMAKE_CXX_STANDARD 14)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# will search for header
include_directories(common)

# GLFW/OpenGL
add_subdirectory(lib/glfw)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/glew/build/cmake" "${CMAKE_SOURCE_DIR/lib/glew/}")
add_subdirectory(lib/glm)
find_package(OpenGL REQUIRED)

# Assimp
set(BUILD_STATIC_LIB ON)
set(BUILD_ASSIMP_TOOLS  ON)
set(ASSIMP_BUILD_STATIC_LIB ON)
add_subdirectory(lib/assimp)

include_directories("lib/glfw/include" ${OPENGL_INCLUDE_DIRS} "lib/glew/include" "lib/glm" "lib/assimp/include")


# Our client/server
add_subdirectory(server)
add_subdirectory(client)
