cmake_minimum_required(VERSION 3.0.2)

project(console)

include(${CMAKE_SOURCE_DIR}/3rdparty/unioncode-qtermwidget-0.14.1.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR true)

set(CXX_CPP
    consolewidget.cpp
    consolemanager.cpp
    generateinput.cpp
    console.cpp
    console.json
    )

set(CXX_H
    consolewidget.h
    consolemanager.h
    generateinput.h
    console.h
    )

add_library(${PROJECT_NAME}
    SHARED
    ${CXX_CPP}
    ${CXX_H}
    )

target_link_libraries(${PROJECT_NAME}
    duc-framework
    duc-base
    duc-services
    ${QtUseModules}
    ${PkgUserModules}
    )

install_logo(${PROJECT_NAME}.svg)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

