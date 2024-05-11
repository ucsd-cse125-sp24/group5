# CMakeList.txt : CMake project for client, include source and define
# project specific logic here.

# Add source to this project's executable.
file(GLOB SOURCES "*.cpp" "./sge/*.cpp" "./sound/*.cpp" "../include/**.h")
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
# Link assimp library for model/animation loading
target_link_libraries(client assimp)
# these added all files in commons
file(GLOB COMMON_SOURCES "../../common/*")
target_sources(client PRIVATE ${COMMON_SOURCES})

# Include the header files
# Add the include directory to the include path
include_directories(../include)
target_include_directories(client PUBLIC ../include)

#
#
#
# for SFML
if(WIN32)
    add_custom_command(
        TARGET client
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:client>
        VERBATIM)
endif()

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

target_link_libraries(client sfml-audio)
install(TARGETS client)


#
#
#
# for json
target_link_libraries(client nlohmann_json::nlohmann_json)