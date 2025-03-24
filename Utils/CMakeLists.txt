cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE INTERNAL "" FORCE)
endif()

project(Utils VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VTK 9.2 REQUIRED)
if(VTK_FOUND)
    message(VTK_DIR "${VTK_DIR}")
    if(NOT(TARGET VTK::GUISupportQt))
        message(FATAL_ERROR "VTK not built with Qt support.")
    endif()
    if(NOT DEFINED VTK_QT_VERSION)
        set(VTK_QT_VERSION 6)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${PROJECT_NAME}
    ImageFormatConvert.cpp ImageFormatConvert.h
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
    PUBLIC ${VTK_LIBRARIES}
)

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES ${VTK_LIBRARIES}
)
