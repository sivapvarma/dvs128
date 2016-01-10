cmake_minimum_required(VERSION 3.4)
PROJECT(ShowEvents)

find_package(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(
    ${edvstools_SOURCE_DIR}
)

SET(QT_USE_QTOPENGL TRUE)

SET(ShowRetina_SOURCES
    main.cpp
    WdgtEdvsVisual.cpp
    EventStream.cpp
    EventIO.cpp
)
SET(ShowRetina_HEADERS
    WdgtEdvsVisual.h
    EventStream.hpp
    EventIO.hpp
    Event.hpp
)
SET(ShowRetina_FORMS
    WdgtEdvsVisual.ui
)

QT4_WRAP_CPP(ShowRetina_HEADERS_MOC ${ShowRetina_HEADERS})
QT4_WRAP_UI(ShowRetina_FORMS_HEADERS ${ShowRetina_FORMS})

INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)

ADD_EXECUTABLE(${PROJECT_NAME}
    ${ShowRetina_SOURCES}
    ${ShowRetina_HEADERS_MOC}
    ${ShowRetina_FORMS_HEADERS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -pthread -lcaer")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    ${QT_LIBRARIES}
)

