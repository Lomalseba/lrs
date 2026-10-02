cmake_minimum_required(VERSION 3.16)

project(LR_1_1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

if(NOT DEFINED OPENSSL_ROOT_DIR)
    set(OPENSSL_ROOT_DIR "D:/programs/OpenSSL-Win64")
endif()

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    pindialog.cpp
    pindialog.h
    pindialog.ui

    creditem.cpp
    creditem.h

    creditemwidget.cpp
    creditemwidget.h
    CredItem.ui
)

qt_add_executable(LR_1_1
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/creds.enc
              ${CMAKE_CURRENT_BINARY_DIR}/creds.enc
              COPYONLY)

target_link_libraries(LR_1_1
    PRIVATE
    Qt6::Widgets
    OpenSSL::SSL
    OpenSSL::Crypto

)


qt_finalize_executable(LR_1_1)
