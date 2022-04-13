cmake_minimum_required(VERSION 3.14)

project(qtdesignviewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Core
    Widgets
    Quick
    Gui
    Qml
    REQUIRED)


qt_add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
    importdummy_wasm.qml
)

# This does not work on Qt 6.3 beta 1, should be fixed in beta 3
# You need to modify Qt6Targets.cmake to change the memory parameter
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY QT_WASM_INITIAL_MEMORY "50MB")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    )

qt6_import_qml_plugins(${CMAKE_PROJECT_NAME})

add_subdirectory(www)

