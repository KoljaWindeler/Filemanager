#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QtGui/QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTextEdit>
#include <QFileSystemModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void ls_dir(QString dir,QStandardItemModel *model, QStandardItem *folder,int level);
private slots:
    void upload_helper();
    void download_helper();
    void delete_helper();
    void connection_fs();
    void ls();
    void mkdir(QString dirname);
    void set_local_path(const QModelIndex &index);
    void set_remote_path();
    void close_newdir_window();
    void create_newdir_window();
    void show_newdir_window();
    void set_device_USB0();
    void set_device_USB1();
    void set_device_rfcomm0();
    void clickkml();
    void remote_click2kml();
    void local_click2kml();
    void conv_imag();
    void send_pilot();
    void show_animation_box();
    void start_animation();
private:
    Ui::MainWindow *ui;
    QFileSystemModel *local_model;//= new QFileSystemModel();
    QString local_path;
    QString remote_path;
    QString dev_path;
    int m_serial_handle;

    void setText(QTextEdit *textfeld,int line_nr,QString inhalt,bool append);
    void kml2navi(QString kml);
    bool rec2kml(QString rec, QString local_kml_file);
    int openSerialDevice(const char* devTTY);
    void connect_fs();
    void disconnect_fs();
    bool delete_file(QString filename);
    bool upload(QStringList local_filenames,QStringList remote_filenames);
    bool download(QStringList local_filename,QStringList remote_filename,QStringList filesize);
    bool cd(QString dir);
};



#endif // FILEMANAGER_H
