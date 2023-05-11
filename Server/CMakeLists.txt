cmake_minimum_required(VERSION 3.5)

project(Server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(QT NAMES Qt5 COMPONENTS Widgets Core Gui Sql Network REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui Sql Network REQUIRED)


set(PROJECT_SOURCES
        src/main.cpp
        src/server.cpp
        src/sqlitedb.cpp
)

set(PROJECT_HEADERS
        inc/server.h
        inc/sqlitedb.h
)

add_executable(Server ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_link_libraries(Server PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Sql Qt5::Network)