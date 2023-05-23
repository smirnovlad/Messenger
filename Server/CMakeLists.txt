cmake_minimum_required(VERSION 3.5)

project(Server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(Qt5 COMPONENTS Widgets Core Sql Network REQUIRED)

set(PROJECT_SOURCES
        src/server.cpp
        src/sqlitedb.cpp
        src/serversocket.cpp
)

set(PROJECT_HEADERS
        inc/server.h
        inc/sqlitedb.h
        inc/serversocket.h
        inc/config.h
)

add_executable(Server main.cpp ${PROJECT_SOURCES} ${PROJECT_HEADERS})
conan_target_link_libraries(Server PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)
#target_link_libraries(Server PRIVATE Qt5::Widgets Qt5::Core Qt5::Sql Qt5::Network)