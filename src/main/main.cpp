#include "mainwindow.h"

#include <QApplication>
#include <QSharedMemory>
#include "funcdef.h"
#include "version.h"

int main(int argc, char *argv[])
{
    LOG_INFO("version: {}", APP_VERSION);
    QApplication a(argc, argv);

    QSharedMemory sharedMemory;
    sharedMemory.setKey("MySerial_soymilk");

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr,
                             QObject::tr("Warning"),
                             QObject::tr("Application is already running!"));

        return 0;
    }

    a.setWindowIcon(QIcon(":/res/icons/icon.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
