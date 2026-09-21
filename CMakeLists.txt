cmake_minimum_required(VERSION 3.16)
project(ForgeUI LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(forgeui INTERFACE)
target_include_directories(forgeui INTERFACE include)

option(FORGEUI_BUILD_EXAMPLES "Build host examples" ON)
if(FORGEUI_BUILD_EXAMPLES)
    add_executable(forgeui_host_demo examples/host_demo/main.cpp)
    target_link_libraries(forgeui_host_demo PRIVATE forgeui)
endif()

if(BUILD_TESTING)
    add_executable(forgeui_protocol_test tests/protocol_test.cpp)
    target_link_libraries(forgeui_protocol_test PRIVATE forgeui)
    target_include_directories(forgeui_protocol_test PRIVATE .)
    add_test(NAME forgeui_protocol_test COMMAND forgeui_protocol_test)
endif()

option(FORGEUI_BUILD_STUDIO "Build the optional Qt desktop editor" ON)
if(FORGEUI_BUILD_STUDIO)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets SerialPort QUIET)
    if(QT_FOUND)
        find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets SerialPort REQUIRED)
        set(CMAKE_AUTOMOC ON)
        add_executable(forgeui_studio
            studio/main.cpp
            studio/MainWindow.cpp
            studio/MainWindow.h
            studio/PreviewWidget.cpp
            studio/PreviewWidget.h
            studio/Exporter.cpp
            studio/Exporter.h
            studio/SceneModel.cpp
            studio/SceneModel.h
            studio/StudioLink.cpp
            studio/StudioLink.h
        )
        target_link_libraries(forgeui_studio PRIVATE
            forgeui
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::SerialPort
        )
        target_compile_features(forgeui_studio PRIVATE cxx_std_17)
    else()
        message(STATUS "Qt 5/6 not found; skipping optional forgeui_studio")
    endif()
endif()
