cmake_minimum_required(VERSION 3.16)

project(MySerial VERSION 1.4.0 LANGUAGES CXX)
message(STATUS "Project version: ${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts SerialPort Network LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts SerialPort Network LinguistTools)

set(FORM_SERIAL
    src/form/serial/formserial.h
    src/form/serial/formserial.cpp
    src/form/serial/formserial.ui
    src/form/serial/LineSend/linesend.h
    src/form/serial/LineSend/linesend.cpp
    src/form/serial/LineSend/linesend.ui
)

set(FORM_PLOT
    src/form/plot/formplot.h
    src/form/plot/formplot.cpp
    src/form/plot/formplot.ui
    src/form/plot/PeakTrajectory/peaktrajectory.h
    src/form/plot/PeakTrajectory/peaktrajectory.cpp
    src/form/plot/PeakTrajectory/peaktrajectory.ui
    src/form/plot/FourierTransform/fouriertransform.h
    src/form/plot/FourierTransform/fouriertransform.cpp
    src/form/plot/FourierTransform/fouriertransform.ui
    src/form/plot/SignalNoiseRatio/signalnoiseratio.h
    src/form/plot/SignalNoiseRatio/signalnoiseratio.cpp
    src/form/plot/SignalNoiseRatio/signalnoiseratio.ui
    src/form/plot/Derivation/derivation.h
    src/form/plot/Derivation/derivation.cpp
    src/form/plot/Derivation/derivation.ui
    src/form/plot/Accumulate/accumulate.h
    src/form/plot/Accumulate/accumulate.cpp
    src/form/plot/Accumulate/accumulate.ui
    src/form/plot/TemperatureView/temperatureview.h
    src/form/plot/TemperatureView/temperatureview.cpp
    src/form/plot/TemperatureView/temperatureview.ui
    src/form/plot/TemperatureConversion/temperatureconversion.h
    src/form/plot/TemperatureConversion/temperatureconversion.cpp
    src/form/plot/TemperatureConversion/temperatureconversion.ui
)

set(FORM_LOG
    src/form/log/formlog.h
    src/form/log/formlog.cpp
    src/form/log/formlog.ui
)

set(FORM_DATA
    src/form/data/formdata.h
    src/form/data/formdata.cpp
    src/form/data/formdata.ui
)

set(FORM_AUTOUPDATE
    src/form/AutoUpdate/autoupdate.h
    src/form/AutoUpdate/autoupdate.cpp
    src/form/AutoUpdate/autoupdate.ui
)

set(FORM_SETTING
    src/form/setting/formsetting.h
    src/form/setting/formsetting.cpp
    src/form/setting/formsetting.ui
    src/form/setting/formtip.h
    src/form/setting/formtip.cpp
    src/form/setting/formtip.ui
)

set(SRC_ENHANCE
    src/enhance/DraggableLine/draggableline.h
    src/enhance/DraggableLine/draggableline.cpp
    src/enhance/MyChartView/mychartview.h
    src/enhance/MyChartView/mychartview.cpp
)

set(SRC_MODULE
    src/module/httpclient/httpclient.h
    src/module/httpclient/httpclient.cpp
)

set(PLOT_DATA
    src/form/FormPlotData/formplotdata.h
    src/form/FormPlotData/formplotdata.cpp
    src/form/FormPlotData/formplotdata.ui
)

set(PLOT_CORRECTION
    src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.h
    src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.cpp
    src/form/FormPlotCorrection/ShowCorrectionCurve/showcorrectioncurve.ui
    src/form/FormPlotCorrection/ShowCorrectionCurve/DataInput/datainput.h
    src/form/FormPlotCorrection/ShowCorrectionCurve/DataInput/datainput.cpp
    src/form/FormPlotCorrection/ShowCorrectionCurve/DataInput/datainput.ui
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
    src/form/FormPlotCorrection/fitting/formfittingarcsin.h
    src/form/FormPlotCorrection/fitting/formfittingarcsin.cpp
    src/form/FormPlotCorrection/fitting/formfittingarcsin.ui
)

set(FORM_PLAY
    src/form/play/formplaympu6050.h
    src/form/play/formplaympu6050.cpp
    src/form/play/formplaympu6050.ui
)

set(FORM_EXTERNAL
    src/form/FormExternal/formexternal.h
    src/form/FormExternal/formexternal.cpp
    src/form/FormExternal/formexternal.ui
)

set(PLOT_HISTORY
    src/form/FormPlotHistory/formplothistory.h
    src/form/FormPlotHistory/formplothistory.cpp
    src/form/FormPlotHistory/formplothistory.ui
    src/form/FormPlotSimulate/formplotsimulate.h
)

set(PLOT_SIMULATE
    src/form/FormPlotSimulate/formplotsimulate.cpp
    src/form/FormPlotSimulate/formplotsimulate.ui
)

set(SRC_MAIN
    src/main/main.cpp
    src/main/mainwindow.cpp
    src/main/mainwindow.h
    src/main/mainwindow.ui
)

set(SRC_THREADWORKER
    src/ThreadWorker/threadworker.h
    src/ThreadWorker/threadworker.cpp
)
set(SRC_COMMON
    src/common/mysetting.h
    src/common/mysetting.cpp
    src/common/mylog.h
    src/common/mylog.cpp
    src/common/UILogSink.h
)

set(SRC_INCLUDE
    src/include/keydef.h
    src/include/datadef.h
    src/include/funcdef.h
    src/include/global.h
)

set(SERIAL_MODE
    src/mode/FormEasy/formeasy.h
    src/mode/FormEasy/formeasy.cpp
    src/mode/FormEasy/formeasy.ui
    src/mode/FormEasy/LoadingOverLay/loadingoverlay.h
    src/mode/FormEasy/LoadingOverLay/loadingoverlay.cpp
    src/mode/FormEasy/LoadingOverLay/loadingoverlay.ui
    src/mode/FormExpert/formexpert.h
    src/mode/FormExpert/formexpert.cpp
    src/mode/FormExpert/formexpert.ui
)

set(PROJECT_SOURCES
        ${SRC_MAIN}
        ${SRC_THREADWORKER}
        ${SRC_MODULE}
        ${SRC_ENHANCE}
        ${SRC_COMMON}
        ${SRC_INCLUDE}
        ${FORM_SERIAL}
        ${FORM_PLOT}
        ${FORM_DATA}
        ${FORM_LOG}
        ${FORM_SETTING}
        ${FORM_AUTOUPDATE}
        ${FORM_PLAY}
        ${PLOT_DATA}
        ${PLOT_CORRECTION}
        ${PLOT_HISTORY}
        ${PLOT_SIMULATE}
        ${FORM_EXTERNAL}
        ${SERIAL_MODE}
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

if(MSVC)
    add_compile_options(/Zm1000)
endif()

target_link_libraries(MySerial PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Charts
  Qt${QT_VERSION_MAJOR}::SerialPort
  Qt${QT_VERSION_MAJOR}::Network
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

include(ExternalProject)

set(FFTW_PREFIX ${CMAKE_BINARY_DIR}/fftw)
set(FFTW_SRC ${CMAKE_SOURCE_DIR}/vendor/fftw-3.3.10)

ExternalProject_Add(fftw_external
    SOURCE_DIR ${FFTW_SRC}
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${FFTW_PREFIX}
        -DBUILD_SHARED_LIBS=ON          # ON 生成动态库, OFF 静态库
        -DENABLE_THREADS=OFF            # 单线程
        -DENABLE_OPENMP=OFF
        -DENABLE_MPI=OFF
        -DENABLE_FLOAT=OFF
        -DENABLE_LONG_DOUBLE=OFF
        -DENABLE_QUAD_PRECISION=OFF
    UPDATE_COMMAND ""                  # 禁止自动下载更新
    PATCH_COMMAND ""                   # 禁止 patch
)

add_dependencies(${PROJECT_NAME} fftw_external)
target_include_directories(${PROJECT_NAME} PRIVATE ${FFTW_PREFIX}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${FFTW_PREFIX}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE fftw3)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/eigen-5.0.0
)

target_link_libraries(${PROJECT_NAME} PRIVATE eigen)

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cpp-httplib-0.28.0
)
