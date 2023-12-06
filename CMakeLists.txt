cmake_minimum_required(VERSION 3.5)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE INTERNAL "" FORCE)
endif()

project(QtTrials VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED)

add_subdirectory(QtSqliteTest)
add_subdirectory(QtWidgetRecorder)
add_subdirectory(QtScreenCapturer)
add_subdirectory(QtDrawTool)
add_subdirectory(QtMagnificationGlass)
add_subdirectory(QtSingleAppGuard)
add_subdirectory(QtPdfGenerator)
add_subdirectory(QtZMQ)
add_subdirectory(QtCamera)
add_subdirectory(QtWebRTC)
add_subdirectory(QtMMap)
add_subdirectory(QtZip)
add_subdirectory(QtFrameHandler)
add_subdirectory(QtOverlay)
add_subdirectory(QtDrawOnVideo)