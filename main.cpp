#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << QDir::currentPath();
    MainWindow w;
    w.show();

    return a.exec();
}
