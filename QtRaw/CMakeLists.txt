cmake_minimum_required(VERSION 3.16)

project(QtRaw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

# tiff
find_package(TIFF REQUIRED)
find_package(LibLZMA REQUIRED)

# raw
find_package(libraw CONFIG REQUIRED)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# TODO: how to make plugin with CMake?
# https://doc.qt.io/qt-6/qt-add-plugin.html

add_library(${PROJECT_NAME}
    RawIOHandler.cpp RawIOHandler.hpp
    #RawPlugin.cpp raw.json
)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DNOMINMAX)

add_executable(${PROJECT_NAME}_test main.cpp RawReader.cpp RawReader.hpp)

target_link_libraries(${PROJECT_NAME}
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
    PUBLIC Qt${QT_VERSION_MAJOR}::Widgets
    PUBLIC Qt${QT_VERSION_MAJOR}::Gui
    PUBLIC TIFF::TIFF
    PUBLIC LibLZMA::LibLZMA
    PUBLIC libraw::raw_r # libraw::raw_r is thread safe version compared with libraw::raw
    PUBLIC ${PNG_LIBRARIES}
    PUBLIC ${JPEG_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
