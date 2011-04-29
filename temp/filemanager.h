#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QtGui/QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void upload();
    void connect_fs();
    void disconnect_fs();
private:
    Ui::MainWindow *ui;
    int m_serial_handle;
};


#endif // FILEMANAGER_H
