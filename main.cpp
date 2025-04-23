#include "mainwindow.h"

#include <QApplication>
#include "funcdef.h"
#include "version.h"

int main(int argc, char *argv[])
{
    LOG_INFO("version: {}", APP_VERSION);
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/icons/icon.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
