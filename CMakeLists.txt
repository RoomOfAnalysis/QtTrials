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
add_subdirectory(QtDebunce)
add_subdirectory(QtConfig)
add_subdirectory(QtLogger)
add_subdirectory(QtDrawOnVideo2)
add_subdirectory(QtDrawOnVideo3)
add_subdirectory(QtAnimation)
add_subdirectory(QtAspect)
add_subdirectory(QtImageViewer)
add_subdirectory(QtOverlay2)
add_subdirectory(QtPlot)
add_subdirectory(QtImageViewer2)
#add_subdirectory(Utils)