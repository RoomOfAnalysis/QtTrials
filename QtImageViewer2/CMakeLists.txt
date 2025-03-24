cmake_minimum_required(VERSION 3.16)

project(QtImageViewer2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# exiv2
find_package(expat CONFIG REQUIRED)  # xmp parser for exiv2
find_package(exiv2 CONFIG REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE PROJECT_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/MetaData.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/MetaData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/Image.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/Image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/ImageOps.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../QtImageViewer/ImageOps.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Exiv2::exiv2lib
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
