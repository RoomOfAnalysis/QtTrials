cmake_minimum_required(VERSION 3.16)

project(QtTilesViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui WebEngineWidgets WebChannel)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_library(openslide
    NAMES openslide
    PATHS ENV OPENSLIDE_LIB_PATH
    NO_CACHE
    REQUIRED
)
if (${openslide} STREQUAL "openslide-NOTFOUND")
    message(FATAL_ERROR "required openslide library but not found!")
else()
    message(STATUS "openslide library found in ${openslide}")
    get_filename_component(openslide_lib_dir ${openslide} PATH)
    get_filename_component(openslide_dir ${openslide_lib_dir} PATH)
    set(openslide_INCLUDE_DIRS ${openslide_dir}/include/openslide)
    message(STATUS "openslide include dirs: ${openslide_INCLUDE_DIRS}")
endif()

find_package(pugixml CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main2.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${openslide_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    PRIVATE Qt${QT_VERSION_MAJOR}::WebChannel
    PRIVATE ${openslide}
    PRIVATE pugixml::shared pugixml::pugixml
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${openslide_dir}/bin/libopenslide-1.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

find_library(QCefView
    NAMES QCefView
    PATHS ENV QCefView_LIB_PATH
    NO_CACHE
    REQUIRED
)
if (${openslide} STREQUAL "QCefView-NOTFOUND")
    message(FATAL_ERROR "required QCefView library but not found!")
else()
    message(STATUS "QCefView library found in ${QCefView}")
    get_filename_component(QCefView_lib_dir ${QCefView} PATH)
    get_filename_component(QCefView_dir ${QCefView_lib_dir} PATH)
    set(QCefView_INCLUDE_DIRS ${QCefView_dir}/include/)
    message(STATUS "QCefView include dirs: ${QCefView_INCLUDE_DIRS}")
endif()

add_executable(${PROJECT_NAME}_cef
    ${CMAKE_CURRENT_SOURCE_DIR}/main3.cpp
)

target_include_directories(${PROJECT_NAME}_cef
    PRIVATE ${openslide_INCLUDE_DIRS}
    PRIVATE ${QCefView_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}_cef
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE ${openslide}
    PRIVATE ${QCefView}
)

add_custom_command(TARGET ${PROJECT_NAME}_cef POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${openslide_dir}/bin/libopenslide-1.dll"
        "${QCefView_dir}/bin/QCefView.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
add_custom_command(TARGET ${PROJECT_NAME}_cef POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${QCefView_dir}/bin/CefView/"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/CefView
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
