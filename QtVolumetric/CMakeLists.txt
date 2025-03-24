cmake_minimum_required(VERSION 3.16)

project(QtVolumetric VERSION 0.1 LANGUAGES CXX)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)  # -Wno-dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Graphs GraphsWidgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# file(GLOB_RECURSE PROJECT_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
#     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/*.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
# )

add_executable(${PROJECT_NAME}
    main.cpp
    vol.cpp vol.h
    mainwindow.cpp mainwindow.h mainwindow.ui
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE Qt${QT_VERSION_MAJOR}::Graphs
    PRIVATE Qt${QT_VERSION_MAJOR}::GraphsWidgets
)

add_executable(${PROJECT_NAME}_example
    main_example.cpp
    volumetric.cpp volumetric.h
)

target_link_libraries(${PROJECT_NAME}_example
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE Qt${QT_VERSION_MAJOR}::Graphs
    PRIVATE Qt${QT_VERSION_MAJOR}::GraphsWidgets
)

set(${PROJECT_NAME}_example_resource_files
    "layer_ground.png"
    "layer_magma.png"
    "layer_water.png"
)

qt6_add_resources(${PROJECT_NAME}_example ${${PROJECT_NAME}_example}
    PREFIX
        "/heightmaps"
    FILES
        ${${PROJECT_NAME}_example_resource_files}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
