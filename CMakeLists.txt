cmake_minimum_required(VERSION 3.16)

project(MySerial VERSION 1.3.11 LANGUAGES CXX)
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
        src/ThreadWorker/threadworker.h
        src/ThreadWorker/threadworker.cpp
        src/module/server/server.h
        src/module/server/server.cpp
        src/form/serial/formserial.h
        src/form/serial/formserial.cpp
        src/form/serial/formserial.ui
        src/form/serial/LineSend/linesend.h
        src/form/serial/LineSend/linesend.cpp
        src/form/serial/LineSend/linesend.ui
        src/form/plot/formplot.h
        src/form/plot/formplot.cpp
        src/form/plot/formplot.ui
        src/form/data/formdata.h
        src/form/data/formdata.cpp
        src/form/data/formdata.ui
        src/form/log/formlog.h
        src/form/log/formlog.cpp
        src/form/log/formlog.ui
        src/form/plot/3d/myglcurvewidget.h
        src/form/plot/3d/myglcurvewidget.cpp
        src/enhance/mychartview.h
        src/enhance/mychartview.cpp
        src/form/FormPlotData/formplotdata.h
        src/form/FormPlotData/formplotdata.cpp
        src/form/FormPlotData/formplotdata.ui
        src/form/FormPlotHistory/formplothistory.h
        src/form/FormPlotHistory/formplothistory.cpp
        src/form/FormPlotHistory/formplothistory.ui
        src/form/FormPlotSimulate/formplotsimulate.h
        src/form/FormPlotSimulate/formplotsimulate.cpp
        src/form/FormPlotSimulate/formplotsimulate.ui
        src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.h
        src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.cpp
        src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.ui
        src/form/FormPlotCorrection/formplotcorrection.h
        src/form/FormPlotCorrection/formplotcorrection.cpp
        src/form/FormPlotCorrection/formplotcorrection.ui
        src/form/FormPlotCorrection/fitting/formfittingkb.h
        src/form/FormPlotCorrection/fitting/formfittingkb.cpp
        src/form/FormPlotCorrection/fitting/formfittingkb.ui
        src/form/FormPlotCorrection/fitting/formfittingsin.h
        src/form/FormPlotCorrection/fitting/formfittingsin.cpp
        src/form/FormPlotCorrection/fitting/formfittingsin.ui
        src/form/FormPlotCorrection/fitting/ImageViewer/imageviewer.h
        src/form/FormPlotCorrection/fitting/ImageViewer/imageviewer.cpp
        src/form/FormPlotCorrection/fitting/formfittingself.h
        src/form/FormPlotCorrection/fitting/formfittingself.cpp
        src/form/FormPlotCorrection/fitting/formfittingself.ui
        src/form/FormPlotCorrection/fitting/CalculateKB/calculatekb.h
        src/form/FormPlotCorrection/fitting/CalculateKB/calculatekb.cpp
        src/form/FormPlotCorrection/fitting/CalculateKB/calculatekb.ui
        src/module/httpclient/httpclient.h
        src/module/httpclient/httpclient.cpp
        src/form/setting/formsetting.h
        src/form/setting/formsetting.cpp
        src/form/setting/formsetting.ui
        src/form/setting/formtip.h
        src/form/setting/formtip.cpp
        src/form/setting/formtip.ui
        src/form/setting/AutoUpdate/autoupdate.h
        src/form/setting/AutoUpdate/autoupdate.cpp
        src/form/setting/AutoUpdate/autoupdate.ui
        src/form/setting/Calculate/calculate.h
        src/form/setting/Calculate/calculate.cpp
        src/form/setting/Calculate/calculate.ui
        src/form/setting/ExternalControl/externalcontrol.h
        src/form/setting/ExternalControl/externalcontrol.cpp
        src/form/setting/ExternalControl/externalcontrol.ui
        src/form/setting/FrameSetting/framesetting.h
        src/form/setting/FrameSetting/framesetting.cpp
        src/form/setting/FrameSetting/framesetting.ui
        src/form/setting/FrameSetting/LengthCalc/lengthcalc.h
        src/form/setting/FrameSetting/LengthCalc/lengthcalc.cpp
        src/form/setting/FrameSetting/LengthCalc/lengthcalc.ui
        src/form/serial/SerialWorker/serialworker.h
        src/form/serial/SerialWorker/serialworker.cpp
        src/form/play/formplaympu6050.h
        src/form/play/formplaympu6050.cpp
        src/form/play/formplaympu6050.ui
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
  "${CMAKE_CURRENT_SOURCE_DIR}/src/enhance"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadWorker"
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

if (WIN32)
    set(APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/icon.ico")

    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    file(WRITE ${APP_ICON_RC} "IDI_ICON1 ICON \"${APP_ICON_PATH}\"\n")

    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RC})
endif()
