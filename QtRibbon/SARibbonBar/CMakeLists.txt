cmake_minimum_required(VERSION 3.16)
project(SARibbonBar LANGUAGES CXX)
set(SARIBBON_LIB_NAME SARibbonBar)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB _qrc "*.qrc")
file(GLOB _header "*.h")
file(GLOB _source "*.cpp")

add_library(${SARIBBON_LIB_NAME}
	${_qrc}
	${_header}
	${_source}
)	

target_link_libraries(${SARIBBON_LIB_NAME} PUBLIC
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(${SARIBBON_LIB_NAME} PROPERTIES
	AUTOMOC ON
	AUTOUIC ON
	AUTORCC ON
	CXX_EXTENSIONS OFF
)

target_compile_definitions(${SARIBBON_LIB_NAME} PUBLIC "SA_RIBBON_BAR_MAKE_LIB")

if(MSVC)
    # To unbreak std::min/max
    target_compile_definitions(${SARIBBON_LIB_NAME} PRIVATE NOMINMAX)
endif()