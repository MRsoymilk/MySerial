cmake_minimum_required(VERSION 3.16)

project(MySerial VERSION 1.0.5 LANGUAGES CXX)
message(STATUS "Project version: ${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts SerialPort)

set(PROJECT_SOURCES
        main/main.cpp
        main/mainwindow.cpp
        main/mainwindow.h
        main/mainwindow.ui
        form/serial/formserial.h
        form/serial/formserial.cpp
        form/serial/formserial.ui
        form/plot/formplot.h
        form/plot/formplot.cpp
        form/plot/formplot.ui
        form/data/formdata.h
        form/data/formdata.cpp
        form/data/formdata.ui
        form/log/formlog.h
        form/log/formlog.cpp
        form/log/formlog.ui
        common/mysetting.h
        common/mysetting.cpp
        common/mylog.h
        common/mylog.cpp
        common/UILogSink.h
        include/keydef.h
        include/datadef.h
        include/funcdef.h
        include/global.h
        res.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MySerial
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        form/plot/plotworker.h form/plot/plotworker.cpp
        form/plot/formplotdata.h form/plot/formplotdata.cpp form/plot/formplotdata.ui
    )
target_include_directories(MySerial PUBLIC "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/common")
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MySerial APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MySerial SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MySerial
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MySerial PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::SerialPort)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MySerial)
endif()
set_target_properties(MySerial PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MySerial
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MySerial)
endif()

add_subdirectory(vendor/spdlog-1.15.0/)
target_link_libraries(MySerial PRIVATE spdlog::spdlog)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
