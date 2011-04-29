#include <QFileSystemModel>
#include <QTreeView>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDirModel>
#include <QDebug>
#include <QVBoxLayout>

#include "filemanager.h"
#include "ui_filemanager.h"
#include "stdio.h"

#define BAUDRATE B57600 /* baudrate settings are defined in <asm/termbits.h>, which is included by <termios.h> */
#define MODEMDEVICE "/dev/ttyUSB0" /*"/dev/ttyUSB0" change this definition for the correct port */
#define _POSIX_SOURCE 1 /* POSIX compliant source */
using namespace std;
#define BUF 255

//////SERIAL
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int openSerialDevice(const char* devTTY, struct termios oldTio) {
    int fileDescriptor;
    fileDescriptor = open(devTTY, O_RDWR | O_NOCTTY);
    if (fileDescriptor == -1) {
        perror("Fehler beim Herstellen der Verbindung.\nFehlermeldung: ");
        exit(1);
        return -99;
    }
    // set new parameters to the serial device
    struct termios newtio;
    fcntl(fileDescriptor, F_SETFL, 0);
    // set everything to 0
    bzero(&newtio, sizeof(newtio));
    // again set everything to 0
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= BAUDRATE; // Set Baudrate first time
    newtio.c_cflag |= CLOCAL; // Local line - do not change "owner" of port
    newtio.c_cflag |= CREAD; // Enable receiver
    newtio.c_cflag &= ~ECHO; // Disable echoing of input characters
    newtio.c_cflag &= ~ECHOE;
    // set to 8N1
    newtio.c_cflag &= ~PARENB; // no parentybyte
    newtio.c_cflag &= ~CSTOPB; // 1 stop bit
    newtio.c_cflag &= ~CSIZE; // Mask the character size bits
    newtio.c_cflag |= CS8; // 8 data bits
    // output mode to
    newtio.c_oflag = 0;
    //newtio.c_oflag |= OPOST;
    // Set teh baudrate for sure
    cfsetispeed(&newtio, BAUDRATE);
    cfsetospeed(&newtio, BAUDRATE);
    newtio.c_cc[VTIME] = 10; /* inter-character timer  */
    newtio.c_cc[VMIN] = 0; /* blocking read until  */
    tcflush(fileDescriptor, TCIFLUSH); // flush pending data
    // set the new defined settings
    if (tcsetattr(fileDescriptor, TCSANOW, &newtio)) {
        perror("could not set the serial settings!");
        return -99;
    }
    return fileDescriptor;
}

int strpos(const string &haystack, const string &needle)
{
    int sleng = haystack.length();
    int nleng = needle.length();

    if (sleng==0 || nleng==0)
        return -1;

    for(int i=0, j=0; i<sleng; j=0, i++ )
    {
        while (i+j<sleng && j<nleng && haystack[i+j]==needle[j])
            j++;
        if (j==nleng)
            return i;
    }
    return -1;
}
////////SERIAL


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFileSystemModel *model = new QFileSystemModel();
    const QModelIndex rootIdx = model->setRootPath(QLatin1String("/home/kolja/Desktop"));
    ui->local->setModel(model);
    ui->local->header()->setClickable(true);
    ui->local->header()->setStretchLastSection(true);
    ui->local->header()->setSortIndicator(0,Qt::AscendingOrder);
    ui->local->header()->setSortIndicatorShown(true);
    ui->local->setRootIndex( rootIdx );

    ui->local->setModel(model);
    ui->Input->setText("treffer");


    connect(ui->file_up,SIGNAL(clicked()),this,SLOT(upload()));
    connect(ui->connect_btn,SIGNAL(clicked()),this,SLOT(connect_fs()));
    //connect(ui->actionBeenden)
}

MainWindow::~MainWindow()
{
    delete ui;
} 

void MainWindow::upload(){
    ui->Input->setText(ui->local->currentIndex().sibling(ui->local->currentIndex().row(),0).data().toString());


};

void MainWindow::connect_fs(){
    //ui->
    struct termios oldtio;
    this->m_serial_handle=openSerialDevice(MODEMDEVICE,oldtio);
    if(this->m_serial_handle!=-1){
        ui->Status->setText("connected");
        ui->connect_btn->setText("disconnect");
        connect(ui->connect_btn,SIGNAL(clicked()),this,SLOT(disconnect_fs()));
    };

};

void MainWindow::disconnect_fs(){
    struct termios oldtio;
    write(this->m_serial_handle,"*",1);
    tcsetattr(this->m_serial_handle,TCSANOW,&oldtio);
    ui->Status->setText("disconnected");
    ui->connect_btn->setText("connect");
    connect(ui->connect_btn,SIGNAL(clicked()),this,SLOT(connect_fs()));
}
