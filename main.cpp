#include <QFileSystemModel>
#include <QTreeView>
#include <QStandardItemModel>
#include <QSplitter>
#include <QtGui/QApplication>
#include "filemanager.h"
#include "dialog_mkdir.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

