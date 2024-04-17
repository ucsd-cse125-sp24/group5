# CMakeList.txt : CMake project for server, include source and define
# project specific logic here.
#

# Add source to this project's executable.
file(GLOB SOURCES "*.cpp" "./bge/*.cpp" "../include/*.h")
add_executable(server ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
file(GLOB COMMON_SOURCES "../../common/*")
target_sources(server PRIVATE ${COMMON_SOURCES})

# Include the header files
# Add the include directory to the include path
include_directories(../include)
target_include_directories(server PUBLIC ../include)
