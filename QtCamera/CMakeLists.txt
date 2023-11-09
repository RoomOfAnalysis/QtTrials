cmake_minimum_required(VERSION 3.6)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE INTERNAL "" FORCE)
endif()

project(QtCamera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# for QtMultimedia ffmpeg backend
# still need manually copy those dlls into target bin folder...
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavdevice
    libavfilter
    libavformat
    libswresample
    libswscale
    libavutil
)

# OpenCV
find_package(OpenCV CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    # main.cpp
    # CameraSource.cpp CameraSource.h
    mainCV.cpp
)

target_link_libraries(${PROJECT_NAME}
    # PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    # PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia
    # PRIVATE Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    # PRIVATE PkgConfig::LIBAV
    PRIVATE ${OpenCV_LIBS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
