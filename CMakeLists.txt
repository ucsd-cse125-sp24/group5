cmake_minimum_required(VERSION 3.27)
project(group5)

set(CMAKE_CXX_STANDARD 14)

include_directories(Game/src)

add_executable(group5
        Game/src/ClientGame.cpp
        Game/src/ClientGame.h
        Game/src/ClientNetwork.cpp
        Game/src/ClientNetwork.h
        Game/src/main.cpp
        Game/src/NetworkData.h
        Game/src/NetworkServices.cpp
        Game/src/NetworkServices.h
        Game/src/ServerGame.cpp
        Game/src/ServerGame.h
        Game/src/ServerNetwork.cpp
        Game/src/ServerNetwork.h
        Game/src/stdafx.cpp
        Game/src/stdafx.h
        Game/src/targetver.h)
