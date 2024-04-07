# CMakeList.txt : CMake project for client, include source and define
# project specific logic here.

# Add source to this project's executable.
file(GLOB SOURCES "*.cpp" "../include/*.h")
add_executable(client ${SOURCES})
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET client PROPERTY CXX_STANDARD 20)
endif()

# Link with OpenGL
target_link_libraries(client OpenGL::GL)

# Link with GLFW
target_link_libraries(client glfw ${GLFW_LIBRARIES})
# Link GLEW library to the client executable
target_link_libraries(client glew_s)
# these added all files in commons
file(GLOB COMMON_SOURCES "../../common/*")
target_sources(client PRIVATE ${COMMON_SOURCES})

# Include the header files
# Add the include directory to the include path
include_directories(../include)
target_include_directories(client PUBLIC ../include)