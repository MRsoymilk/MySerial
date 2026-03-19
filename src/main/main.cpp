#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QSharedMemory>

#include "funcdef.h"
#include "mainwindow.h"
#include "version.h"

int main(int argc, char *argv[]) {
    LOG_INFO("version: {}", APP_VERSION);

    QApplication a(argc, argv);
    QSharedMemory sharedMemory;
    sharedMemory.setKey("MySerial_soymilk");

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Application is already running!"));
        LOG_WARN("Application is already running!");
        return 0;
    }

    a.setWindowIcon(QIcon(":/res/icons/icon.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("MySerial");
    parser.addHelpOption();

    QCommandLineOption option_cli("cli", "Run with CLI arguments");
    parser.addOption(option_cli);
    QCommandLineOption option_mode("mode", "Run mode: easy | produce | expert", "mode");
    parser.addOption(option_mode);
    QCommandLineOption option_algorithm("algorithm", "Run algorithm: F15_Single | F15_Curves | F30_Single | F30_Curves", "algorithm");
    parser.addOption(option_algorithm);

    parser.process(a);

    MainWindow w;
    bool is_cli_start = parser.isSet(option_cli);
    if(is_cli_start) {
        LOG_INFO("run with cli");
        QString mode = parser.value(option_mode).toLower();
        if (mode.isEmpty()) {
            mode = "expert";
            LOG_INFO("use default mode: {}", mode);
        }
        else {
            LOG_INFO("use mode: {}", mode);
        }
        if (mode != "easy" && mode != "produce" && mode != "expert") {
            QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Invalid mode! Use easy/produce/expert"));
            LOG_WARN("Invalid mode: {}", mode);
            return 0;
        }

        QString algorithm = parser.value(option_algorithm);
        if(algorithm.isEmpty()) {
            algorithm = "F30_Curves";
            LOG_INFO("use default algorithm: {}", algorithm);
        }
        else {
            LOG_INFO("use algorithm: {}", algorithm);
        }
        if(!(COMPARE_CaseInsensitive(algorithm, CFG_ALGORITHM_F15_SINGLE)
              || COMPARE_CaseInsensitive(algorithm, CFG_ALGORITHM_F15_CURVES)
              || COMPARE_CaseInsensitive(algorithm, CFG_ALGORITHM_F30_SINGLE)
              || COMPARE_CaseInsensitive(algorithm, CFG_ALGORITHM_F30_CURVES))){
            QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Invalid algorithm! Use F15_single/F15_curves/F30_single/F30_curves"));
            LOG_WARN("Invalid algorithm: {}", algorithm);
            return 0;
        }
        w.setCli();
        w.setMode(mode);
        w.setAlgorithm(algorithm);
    } else {
        LOG_INFO("What an ordinary day.");
    }
    w.show();
    return a.exec();
}
