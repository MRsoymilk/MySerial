cmake_minimum_required(VERSION 3.16)

project(MySerial VERSION 1.5.14 LANGUAGES CXX)
message(STATUS "Project version: ${PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Widgets
    Charts
    SerialPort
    Network
    LinguistTools
)

set(FORM_SERIAL
    src/form/serial/formserial.h
    src/form/serial/formserial.cpp
    src/form/serial/formserial.ui
    src/form/serial/LineSend/linesend.h
    src/form/serial/LineSend/linesend.cpp
    src/form/serial/LineSend/linesend.ui
    src/form/serial/ThreadParser/threadparser.h
    src/form/serial/ThreadParser/threadparser.cpp
    src/form/serial/HandleModeProduce/handlemodeproduce.h
    src/form/serial/HandleModeProduce/handlemodeproduce.cpp
    src/form/serial/HandleModeExpert/handlemodeexpert.h
    src/form/serial/HandleModeExpert/handlemodeexpert.cpp
    src/form/serial/HandleModeEasy/handlemodeeasy.h
    src/form/serial/HandleModeEasy/handlemodeeasy.cpp
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
    src/form/plot/DarkSpectrum/darkspectrum.h
    src/form/plot/DarkSpectrum/darkspectrum.cpp
    src/form/plot/DarkSpectrum/darkspectrum.ui
    src/form/plot/PointsTracker/pointstracker.h
    src/form/plot/PointsTracker/pointstracker.cpp
    src/form/plot/PointsTracker/pointstracker.ui
)

set(FORM_LOG
    src/form/log/formlog.h
    src/form/log/formlog.cpp
    src/form/log/formlog.ui
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
    src/enhance/HoverSliderButton/hoversliderbutton.h
    src/enhance/HoverSliderButton/hoversliderbutton.cpp
    src/enhance/HoverTbtnButton/hovertbtnbutton.h
    src/enhance/HoverTbtnButton/hovertbtnbutton.cpp
    src/enhance/peakcfg.h
    src/enhance/peakcfg.cpp
    src/enhance/peakcfg.ui
)

set(SRC_MODULE
    src/module/httpclient/httpclient.h
    src/module/httpclient/httpclient.cpp
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
    src/form/FormPlotCorrection/fitting/formfittingpoints.h
    src/form/FormPlotCorrection/fitting/formfittingpoints.cpp
    src/form/FormPlotCorrection/fitting/formfittingpoints.ui
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
    src/form/FormPlotHistory/ShowData/showdata.h
    src/form/FormPlotHistory/ShowData/showdata.cpp
    src/form/FormPlotHistory/ShowData/showdata.ui
)

set(PLOT_SIMULATE
    src/form/FormPlotSimulate/formplotsimulate.h
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
    src/common/findpeak.h
    src/common/findpeak.cpp
    src/common/findfwhm.h
    src/common/findfwhm.cpp
    src/common/myprocess.h
    src/common/myprocess.cpp
    src/common/processthread.h
    src/common/processthread.cpp
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
    src/mode/LoadingOverLay/loadingoverlay.h
    src/mode/LoadingOverLay/loadingoverlay.cpp
    src/mode/LoadingOverLay/loadingoverlay.ui
    src/mode/FormExpert/formexpert.h
    src/mode/FormExpert/formexpert.cpp
    src/mode/FormExpert/formexpert.ui
    src/mode/FormProduce/formproduce.h
    src/mode/FormProduce/formproduce.cpp
    src/mode/FormProduce/formproduce.ui
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
        ${FORM_LOG}
        ${FORM_SETTING}
        ${FORM_AUTOUPDATE}
        ${FORM_PLAY}
        ${PLOT_CORRECTION}
        ${PLOT_HISTORY}
        ${PLOT_SIMULATE}
        ${FORM_EXTERNAL}
        ${SERIAL_MODE}
        res.qrc
)

qt_add_executable(${PROJECT_NAME}
  MANUAL_FINALIZATION
  ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/enhance"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadWorker"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/module/httpclient"
)

if(MSVC)
    add_compile_options(/Zm1000)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt::Widgets
  Qt::Charts
  Qt::SerialPort
  Qt::Network
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

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
