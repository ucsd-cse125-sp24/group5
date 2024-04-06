# CMakeList.txt : CMake project for server, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (server "server.cpp" "server.h" "ServerGame.cpp" "ServerGame.h" "ServerNetwork.cpp" "ServerNetwork.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
file(GLOB COMMON_SOURCES "../../common/*")
target_sources(server PRIVATE ${COMMON_SOURCES})