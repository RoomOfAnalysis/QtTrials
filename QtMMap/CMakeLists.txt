cmake_minimum_required(VERSION 3.16)

project(QtMMap VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Network)

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

add_executable(${PROJECT_NAME}
    Capturer.cpp Capturer.h
    FrameWriter.cpp FrameWriter.h
    FrameReader.cpp FrameReader.h
    FrameWriterMmap.cpp FrameWriterMmap.h
    FrameReaderMmap.cpp FrameReaderMmap.h
    FrameWriterSocket.cpp FrameWriterSocket.h
    FrameReaderSocket.cpp FrameReaderSocket.h
    Viewer.cpp Viewer.h Viewer.ui
    main.cpp
)

add_compile_definitions(
    #USE_MMAP
    USE_SOCKET
    DISPLAY_FPS
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia
    PRIVATE Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Network
    PRIVATE PkgConfig::LIBAV
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
