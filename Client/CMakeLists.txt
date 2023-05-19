cmake_minimum_required(VERSION 3.5)

project(Client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STAcNDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(Qt5 COMPONENTS Widgets Core Gui Network REQUIRED)

set(PROJECT_SOURCES
        src/registration.cpp
        src/authorization.cpp
        src/user.cpp
        src/userlist.cpp
        src/chat.cpp
        src/message.cpp
        src/clientsocket.cpp
        src/clientui.cpp
        src/client.cpp
        src/main.cpp
)

set(PROJECT_HEADERS
        inc/client.h
        inc/registration.h
        inc/authorization.h
        inc/user.h
        inc/userlist.h
        inc/chat.h
        inc/message.h
        inc/clientsocket.h
        inc/clientui.h
)

set(PROJECT_UI
        src/client.ui
        src/registration.ui
        src/authorization.ui
        src/user.ui
        src/userlist.ui
        src/chat.ui
        src/message.ui
        src/clientui.ui
)

add_executable(Client ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_UI})
conan_target_link_libraries(Client PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)
#target_link_libraries(Client PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)