# CMakeList.txt : CMake project for client, include source and define
# project specific logic here.

# Add source to this project's executable.
add_executable (client "client.cpp" "client.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET client PROPERTY CXX_STANDARD 20)
endif()

# Link with OpenGL
target_link_libraries(client OpenGL::GL)

# Link with GLFW
target_link_libraries(client glfw ${GLFW_LIBRARIES})
# Link GLEW library to the client executable
target_link_libraries(client glew_s)
# TODO: Add tests and install targets if needed.
