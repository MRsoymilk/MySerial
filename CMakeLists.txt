cmake_minimum_required(VERSION 3.16)

project(MySerial VERSION 1.1.24 LANGUAGES CXX)
message(STATUS "Project version: ${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts SerialPort Network LinguistTools OpenGLWidgets OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts SerialPort Network LinguistTools OpenGLWidgets OpenGL)

set(PROJECT_SOURCES
        src/main/main.cpp
        src/main/mainwindow.cpp
        src/main/mainwindow.h
        src/main/mainwindow.ui
        src/module/server/server.h
        src/module/server/server.cpp
        src/form/serial/formserial.h
        src/form/serial/formserial.cpp
        src/form/serial/formserial.ui
        src/form/plot/formplot.h
        src/form/plot/formplot.cpp
        src/form/plot/formplot.ui
        src/form/data/formdata.h
        src/form/data/formdata.cpp
        src/form/data/formdata.ui
        src/form/log/formlog.h
        src/form/log/formlog.cpp
        src/form/log/formlog.ui
        src/form/plot/plot_algorithm.h
        src/form/plot/3d/myglcurvewidget.h
        src/form/plot/3d/myglcurvewidget.cpp
        src/form/plot/enhance/mychartview.h
        src/form/plot/enhance/mychartview.cpp
        src/form/plot/plotworker.h
        src/form/plot/plotworker.cpp
        src/form/plot/formplotdata.h
        src/form/plot/formplotdata.cpp
        src/form/plot/formplotdata.ui
        src/form/plot/formplothistory.h
        src/form/plot/formplothistory.cpp
        src/form/plot/formplothistory.ui
        src/form/plot/formplotsimulate.h
        src/form/plot/formplotsimulate.cpp
        src/form/plot/formplotsimulate.ui
        src/form/plot/formplotcorrection.h
        src/form/plot/formplotcorrection.cpp
        src/form/plot/formplotcorrection.ui
        src/module/httpclient/httpclient.h
        src/module/httpclient/httpclient.cpp
        src/form/plot/fitting/ImageViewer/imageviewer.h
        src/form/plot/fitting/ImageViewer/imageviewer.cpp
        src/form/plot/fitting/formfittingkb.h
        src/form/plot/fitting/formfittingkb.cpp
        src/form/plot/fitting/formfittingkb.ui
        src/form/plot/fitting/formfittingsin.h
        src/form/plot/fitting/formfittingsin.cpp
        src/form/plot/fitting/formfittingsin.ui
        src/form/setting/formsetting.h
        src/form/setting/formsetting.cpp
        src/form/setting/formsetting.ui
        src/form/setting/formtip.h
        src/form/setting/formtip.cpp
        src/form/setting/formtip.ui
        src/common/mysetting.h
        src/common/mysetting.cpp
        src/common/mylog.h
        src/common/mylog.cpp
        src/common/UILogSink.h
        src/include/keydef.h
        src/include/datadef.h
        src/include/funcdef.h
        src/include/global.h
        res.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(MySerial
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
  )

target_include_directories(MySerial PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/module/server"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/module/httpclient"
)

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

target_link_libraries(MySerial PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Charts
  Qt${QT_VERSION_MAJOR}::SerialPort
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::OpenGLWidgets
  Qt${QT_VERSION_MAJOR}::OpenGL
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.soymilk.MySerial)
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

set(TRANSLATIONS
 res/i18n/en.ts
 res/i18n/zh_simple.ts
 res/i18n/zh_traditional.ts
)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES)
qt_add_translations(MySerial TS_FILES ${TRANSLATIONS})

add_subdirectory(vendor/spdlog-1.15.0/)
target_link_libraries(MySerial PRIVATE spdlog::spdlog)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
