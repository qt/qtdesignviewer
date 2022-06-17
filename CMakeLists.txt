cmake_minimum_required(VERSION 3.14)

project(qtdesignviewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS
    Core
    Widgets
    Quick
    Gui
    Qml
    REQUIRED)


set(QT_MINIMUM_VERSION 6.3.0)
if(QT_VERSION VERSION_LESS QT_MINIMUM_VERSION)
    MESSAGE(FATAL_ERROR "Minimum supported Qt version: ${QT_MINIMUM_VERSION}")
endif()

qt_add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
    importdummy_wasm.qml
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY QT_WASM_INITIAL_MEMORY "50MB")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::Gui
    Qt6::Qml
    Qt6::GuiPrivate
    )

qt6_import_qml_plugins(${CMAKE_PROJECT_NAME})

file(COPY ${CMAKE_SOURCE_DIR}/www/ DESTINATION ${CMAKE_BINARY_DIR}/)


