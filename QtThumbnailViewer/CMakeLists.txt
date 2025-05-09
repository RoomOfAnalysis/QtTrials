cmake_minimum_required(VERSION 3.16)

project(QtThumbnailViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Charts Xml Concurrent)
find_package(qtadvanceddocking-qt6 CONFIG REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE PROJECT_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

set(RES_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/qdarkstyle/dark/darkstyle.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/qdarkstyle/light/lightstyle.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${RES_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE Qt${QT_VERSION_MAJOR}::Charts
    PRIVATE Qt${QT_VERSION_MAJOR}::Xml
    PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent
    PRIVATE ads::qtadvanceddocking-qt6
)
