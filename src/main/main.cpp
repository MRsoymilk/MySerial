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

    QCommandLineParser parser;
    parser.setApplicationDescription("MySerial");
    parser.addHelpOption();

    QCommandLineOption option_mode("mode", "Run mode: easy | produce | expert", "mode");

    parser.addOption(option_mode);
    parser.process(a);

    QString mode = parser.value(option_mode).toLower();

    if (mode.isEmpty()) {
        mode = "expert";
    }

    if (mode != "easy" && mode != "produce" && mode != "expert") {
        QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Invalid mode! Use easy/produce/expert"));
        LOG_WARN("Invalid mode: {}", mode);
        return 0;
    }

    QSharedMemory sharedMemory;
    sharedMemory.setKey("MySerial_soymilk_" + mode);

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Application is already running!"));
        LOG_WARN("Application is already running!");
        return 0;
    }

    a.setWindowIcon(QIcon(":/res/icons/icon.png"));

    MainWindow w;
    w.setMode(mode);
    w.show();

    return a.exec();
}
