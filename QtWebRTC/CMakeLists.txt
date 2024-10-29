cmake_minimum_required(VERSION 3.6)

set(CMAKE_BUILD_TYPE Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

project(QtWebRTC VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(WEBRTCROOT "${CMAKE_CURRENT_SOURCE_DIR}/webrtc" CACHE STRING "WEBRTC root directory")

FILE(GLOB SOURCE src/*.cpp src/*.ui include/*.h)
add_executable (${PROJECT_NAME} ${SOURCE} ${WEBRTCEXTRAOBJS})
target_include_directories(${PROJECT_NAME} PRIVATE include)

find_package(jsoncpp CONFIG REQUIRED)

set(WEBRTCINCLUDE ${WEBRTCROOT}/include ${WEBRTCROOT}/include/third_party/abseil-cpp ${WEBRTCROOT}/include/third_party/libyuv/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${WEBRTCINCLUDE})
FILE(GLOB_RECURSE LIBWEBRTC ${WEBRTCROOT}/${CMAKE_BUILD_TYPE}/${CMAKE_STATIC_LIBRARY_PREFIX}*${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(${PROJECT_NAME} ${LIBWEBRTC} JsonCpp::JsonCpp)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

if (WIN32)
    # overide compilation flags
    set(CompilerFlags CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_RELEASE)
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

    # Qt's `sigslots.h` conflict with webrtc
    add_definitions(-DQT_DEPRECATED_WARNINGS -DQT_NO_KEYWORDS)

    # webrtc
    add_definitions(-DWEBRTC_WIN -EHsc -DNOMINMAX -D__PRETTY_FUNCTION__=__FUNCTION__ -D_HAS_ITERATOR_DEBUGGING=0)
    add_definitions(-DUSE_X11 -DHAVE_SOUND -D_WINSOCKAPI_)
    target_link_libraries (${PROJECT_NAME} secur32 dmoguids wmcodecdspuuid strmiids msdmo winmm dxgi d3d11 iphlpapi dwmapi shcore ws2_32)
endif()