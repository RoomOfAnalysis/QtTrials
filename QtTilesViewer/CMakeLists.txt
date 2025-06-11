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

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
