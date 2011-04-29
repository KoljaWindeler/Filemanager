warum geht das denn nicht, he?
#define dbug false
#define BAUDRATE B57600 /* baudrate settings are defined in <asm/termbits.h>, which is included by <termios.h> */
#define CONNECT_TIME 0000000
using namespace std;

#include <QFileSystemModel>
#include <QTreeView>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDirModel>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>

#include "filemanager.h"
#include "ui_filemanager.h"
#include "stdio.h"

#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <fstream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    // starte bluetooth
    system("sudo /usr/bin/rfcomm bind 0  2> /dev/null");

    // erstelle gui und verstecke bereiche
    ui->setupUi(this);
    ui->createdir_area->hide();
    ui->Animation_menu->hide();
    ui->remote->setStyleSheet("QTreeView {show-decoration-selected: 1;   background-color: #DDDDDD;}");
    ui->local_kml->setEnabled(false);
    ui->img_conv->setEnabled(false);
    ui->file_up->setEnabled(false);
    ui->rec2kml->setEnabled(false);
    ui->file_down->setEnabled(false);
    ui->delete_btn->setEnabled(false);
    ui->newdir_btn->setEnabled(false);
    //ui->Animation_button->setEnabled(false);
    ui->pilotbutton->setEnabled(false);
    ui->refresh_btn->setEnabled(false);
    ui->uploadgooglekml->setEnabled(false);

    m_serial_handle=-1;
    dev_path="/dev/ttyUSB0";
    local_model= new QFileSystemModel();
    remote_path="/";
    local_path="/home/kolja/";

    // erstelle modelle
    const QModelIndex rootIdx = local_model->setRootPath(local_path);
    ui->local->setModel(local_model);
    ui->local->header()->setClickable(true);
    ui->local->header()->setStretchLastSection(true);
    ui->local->header()->setSortIndicator(0,Qt::AscendingOrder);
    ui->local->header()->setSortIndicatorShown(true);
    ui->local->setRootIndex( rootIdx );
    ui->local->header()->resizeSection(0,190);
    ui->local->header()->resizeSection(1,79);
    ui->local->header()->resizeSection(2,60);
    ui->local->header()->resizeSection(3,120);
    ui->local->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //ui->local->header()->sortIndicatorOrder();
    ui->local->sortByColumn(0, Qt::AscendingOrder);

    // verbinde knöppe
    connect(ui->file_up,SIGNAL(clicked()),this,SLOT(upload_helper()));
    connect(ui->file_down,SIGNAL(clicked()),this,SLOT(download_helper()));
    connect(ui->connect_btn,SIGNAL(clicked()),this,SLOT(connection_fs()));
    connect(ui->refresh_btn,SIGNAL(clicked()),this,SLOT(ls()));
    connect(ui->delete_btn,SIGNAL(clicked()),this,SLOT(delete_helper()));
    connect(ui->newdir_btn,SIGNAL(clicked()),this,SLOT(show_newdir_window()));
    connect(ui->img_conv,SIGNAL(clicked()),this,SLOT(conv_imag()));
    connect(ui->local, SIGNAL(clicked(const QModelIndex&)) ,this, SLOT( set_local_path(const QModelIndex&) ) );
    connect(ui->remote, SIGNAL(clicked(const QModelIndex&)) ,this, SLOT( set_remote_path() ) );
    connect(ui->remote, SIGNAL(entered(const QModelIndex&)) ,this, SLOT( set_remote_path() ) );

    connect(ui->newdir_cancle, SIGNAL(clicked()),this,SLOT(close_newdir_window()));
    connect(ui->newdir_create, SIGNAL(clicked()),this,SLOT(create_newdir_window()));
    connect(ui->action_dev_ttyUSB0, SIGNAL(triggered()),this,SLOT(set_device_USB0()));
    connect(ui->action_dev_ttyUSB1, SIGNAL(triggered()),this,SLOT(set_device_USB1()));
    connect(ui->action_dev_rfcomm, SIGNAL(triggered()),this,SLOT(set_device_rfcomm0()));
    connect(ui->uploadgooglekml,SIGNAL(clicked()),this,SLOT(clickkml()));
    connect(ui->rec2kml,SIGNAL(clicked()),this,SLOT(remote_click2kml()));
    connect(ui->local_kml,SIGNAL(clicked()),this,SLOT(local_click2kml()));
    connect(ui->pilotbutton,SIGNAL(clicked()),this,SLOT(send_pilot()));
    connect(ui->ani_show,SIGNAL(clicked()),this,SLOT(start_animation()));
    connect(ui->Animation_button,SIGNAL(clicked()),this,SLOT(show_animation_box()));

}

MainWindow::~MainWindow(){
    delete ui;
}

int MainWindow::openSerialDevice(const char* devTTY) {
    int fileDescriptor;
    fileDescriptor = open(devTTY, O_RDWR | O_NOCTTY);
    if (fileDescriptor == -1) {
        return -1;
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
    usleep(CONNECT_TIME); // Done -> geht wunderbar mit 0
    tcflush(fileDescriptor, TCIFLUSH); // flush pending data
    // set the new defined settings
    if (tcsetattr(fileDescriptor, TCSANOW, &newtio)) {
        perror("could not set the serial settings!");
        return -1;
    }
    setText(ui->status,-1," | Serial port configured",true);
    this->update();
    this->repaint();
    return fileDescriptor;
}

void MainWindow::set_local_path(const QModelIndex &index){
    QString filename = local_model->filePath(index);
    QString extension = local_model->fileName(index);
    extension=extension.split(".").at(extension.split(".").count()-1);

    // all knöppe deaktivieren
    ui->img_conv->setEnabled(false);
    ui->uploadgooglekml->setEnabled(false);
    ui->local_kml->setEnabled(false);

    // passenden aktivieren
    if(extension.toLower()=="bmp" || extension.toLower()=="jpg"){
        ui->img_conv->setEnabled(true);
    } else if(extension.toLower()=="kml"){
        ui->uploadgooglekml->setEnabled(true);
    } else if(extension.toLower()=="log" || extension.toLower()=="gps"|| extension.toLower()=="txt"){
        ui->local_kml->setEnabled(true);
    };

    // rausfinden obs ein verzeichniss ist, wenn ja nur ein / ran, sonst hinten den dateinamen wegschneiden
    QString parent_dir;
    if(local_model->fileInfo(index).isDir()){
        parent_dir=filename;
        parent_dir.append("/");
    } else {
        QStringList dir_list=filename.split("/");
        for(int a=0;a<dir_list.count()-1;a++){
            parent_dir.append(dir_list.at(a));
            parent_dir.append("/");
        }
    }
    if(dbug) cout << parent_dir.toStdString() << endl;
    local_path=parent_dir;
}

void MainWindow::set_remote_path()
{
    QString path="/";
    QString subdirs[99]; // array of string
    int subdir_pointer=0;
    QModelIndex where=ui->remote->currentIndex();
    while(where.parent().data().toString()!=""){ // solange es parents gibt
        QModelIndex parent_where=where.parent();
        where=parent_where; // pointer weiterreichen
        subdirs[subdir_pointer]=parent_where.data().toString();
        subdir_pointer++;
    }
    // hänge die subdirs rückwärts an, damit der pfad daraus wird
    for(int a=subdir_pointer-1;a>=0;a--){
        path.append(subdirs[a]);
        path.append("/");
    }
    // das angeklickte ding ist ein selbst ein verzeichniss also gehen wir auch noch da rein
    if(ui->remote->currentIndex().child(0,0).data().toString().toStdString()!=""){
        path.append(ui->remote->currentIndex().sibling(ui->remote->currentIndex().row(),0).data().toString());
        path.append("/");
    }
    if(dbug)    cout << path.toStdString() << endl;
    remote_path=path;

    // all knöppe deaktivieren
    ui->rec2kml->setEnabled(false);

    QString extension = ui->remote->currentIndex().sibling(ui->remote->currentIndex().row(),0).data().toString();
    extension=extension.split(".").at(extension.split(".").count()-1);

    // passenden aktivieren
    if(extension.toLower()=="log" || extension.toLower()=="gps"){
        ui->rec2kml->setEnabled(true);
    };
};

void MainWindow::upload_helper(){
    // drei leere variablen aufsetzen
    QStringList locals; locals.empty();
    QStringList remotes; remotes.empty();

    // generate valid filename - convert to 8.3
    QModelIndexList indexes = ui->local->selectionModel()->selectedIndexes();
    bool upload_ok=true;
    for(int a=0; a<indexes.count()/4; a++){
        QModelIndex index=indexes.at(a);
        QString filename = ui->local->currentIndex().sibling(index.row(),0).data().toString(); // dateiname nicht pfad

        filename.replace(QString(" "),QString(""));
        QStringList tmp=filename.split(".");
        filename.clear();
        if(tmp.count()==2){ // bla.xyz
            QString tmp2 = tmp.at(0);
            QString tmp3 = tmp.at(1);
            if(tmp2.length()>8 or tmp3.length()>3)
                ui->status->append("Warning! Filename cutted");
            filename.append(tmp2.left(8));
            filename.append(".");
            filename.append(tmp3.left(3));
        } else if(tmp.count()==1){ // blabla
            QString tmp2 = tmp.at(0);
            if(tmp2.length()>8)
                ui->status->append("Warning! Filename cutted");
            filename.append(tmp2.left(8));
        } else { // bla1.bla2.bla3.vwxyz => bla1.vwx
            QString tmp2 = tmp.at(0);
            if(tmp2.length()>8)
                ui->status->append("Warning! Filename cutted");
            filename.append(tmp2.left(8));
            tmp2=tmp.at(tmp.count()-1);
            filename.append(".");
            filename.append(tmp2.left(3));
        };

        // create local file
        QString local_filename=local_path;
        QString r_filename = ui->local->currentIndex().sibling(index.row(),0).data().toString();
        local_filename.append(r_filename);

        // create remote filename
        QString remote_filename=remote_path;
        remote_filename.append(filename);

        // add to list
        locals.append(local_filename);
        remotes.append(remote_filename);
        //cout << "will aufrufen: upload("<< local_filename.toStdString() << ")" << endl;
    };

    // wenn mindestens ein eintrag in derListe ist
    if(locals.count()>0){
        if(!upload(locals,remotes)){
            upload_ok=false;
        }
    }

    if(upload_ok && indexes.count()>0){
        // mach noch mal listen nach den upload
        ls();
    }
}

bool MainWindow::upload(QStringList local_filenames,QStringList remote_filenames){
    for(int i=0;i<local_filenames.count();i++){
        int j=0;
        if(i>0) j=i+1;
        setText(ui->debug,j,local_filenames.at(i)+" <- "+remote_filenames.at(i),false);
    }
    // variablen die über alle schleifeninterationen gültig sind
    bool status_good=true;

    // für jede Datei den download
    for(int i=0;i<local_filenames.count();i++){
        QString local_filename=local_filenames.at(i);
        QString remote_filename=remote_filenames.at(i);

        if(dbug) cout << "uploading " << local_filename.toStdString() << " to " << remote_filename.toStdString() << endl;
        if(m_serial_handle!=-1){
            // wechsel remote in passenden subfolder
            QStringList subdirs=remote_filename.split("/");
            if(dbug) cout << "ich gehe jetzt " << subdirs.count() << " stufen runter" << endl;
            for(int a=1;a<subdirs.count()-1;a++){
                if(dbug) cout << " dann geh ich mal runter " << subdirs[a].toStdString() << endl;
                cd(subdirs[a]);
            }
            // wechsel remote in passenden subfolder

            // some output
            QString statusreport="Uploading file \""+local_filename+"\" ... ";
            ui->status->append(statusreport);
            // some output

            // open remote file
            QString commando="w";
            commando.append(subdirs.at(subdirs.count()-1));
            commando.append('\n');
            if(dbug) cout << "Sende:" << commando.toAscii().data() << endl;
            write(m_serial_handle,commando.toAscii().data(),commando.length()); //senden zum deaktiviere des eventuell noch offenen f_m
            // open remote file


            // open local file
            ifstream file(local_filename.toAscii().data(),ios::out | ios::app | ios::binary);
            file.seekg (0, ios::end);
            ifstream::pos_type file_size_off = file.tellg();
            int file_size=file_size_off;
            char * memblock;
            memblock = new char[file_size+3];
            file.seekg (0, ios::beg);
            file.read (memblock, file_size);
            memblock[file_size]='*';
            memblock[file_size+1]='*';
            memblock[file_size+2]='*';
            file.close();

            tcflush(m_serial_handle, TCIFLUSH); // flush pending data
            // empfange das ack
            char buffer[5];
            time_t time_start=time(0);
            int rate=0; // download rate
            int a=0; // a upload counter
            while(a<(file_size+3)){
                write(m_serial_handle,&memblock[a],sizeof(char));
                if(dbug) cout << "sende " << memblock[a] << endl;
                a++;
                // alle 100 zeichen eine ack abwarten
                if((a)%100==0){
                    int recv_chars=0;
                    if(dbug) cout << "warte auf ACK" << endl;
                    while(recv_chars<3){
                        usleep(10000); // wait 0,1 sec
                        char temp[2];
                        if(read(m_serial_handle, temp, 1)>0){
                            buffer[recv_chars]=temp[0];
                            recv_chars++;
                        }
                    }
                    if(dbug) cout << "return \"" << buffer << "\"" << endl;
                    // ja, ack empfangen
                    if(strncmp(buffer,"ACK", 3)==0){
                        int bisher_zeit=time(0)-time_start;
                        if(bisher_zeit==0) bisher_zeit=1;
                        rate=a/bisher_zeit;

                        int min=((file_size-a)/rate/60)%60;
                        int sec=((file_size-a)/rate)%60;
                        char zeit_remaining[10];
                        sprintf(zeit_remaining,"%02i:%02i min",min,sec);

                        min=bisher_zeit/60;
                        sec=bisher_zeit%60;
                        char zeit_passed[10];
                        sprintf(zeit_passed,"%02i:%02i min",min,sec);

                        QString download_status="    "+QString::QString(zeit_passed)+" passed    |    "+QString::QString(zeit_remaining);
                        download_status.append(" remaining    |    "+QString::number(100*a/(file_size+3),'g',2.2)+"%    |    ");
                        download_status.append(QString::number(a)+" / "+QString::number(file_size));
                        download_status.append(" Bytes ("+QString::number(rate)+" B/s)");
                        this->setText(ui->debug,1,download_status,false);

                        this->update();
                        this->repaint();
                    } else { // haben was anderes als ACK empfangen
                        // popup box
                        QMessageBox msgBox;
                        msgBox.setText("Error, wrong return value");
                        QString text="Expected: \"ACK\", got:\"";
                        text.append(buffer);
                        text.append("\"\n Try pilot?");
                        msgBox.setInformativeText(text);
                        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        msgBox.setDefaultButton(QMessageBox::Yes);
                        msgBox.setEscapeButton(QMessageBox::Cancel);
                        switch (msgBox.exec()) {
                        case QMessageBox::Yes:
                            send_pilot();
                            break;
                        default:
                            break;
                        };
                        // popup box
                        if(dbug) cout << "bin im else: " << buffer << endl;
                        a=file_size; // out
                        status_good=false;
                    }
                }
            }

            memset(buffer, 0, 5);
            int recv_chars=0;
            if(dbug) cout << "warte auf done" << endl;
            // warte auf vier char
            while(recv_chars<4){
                usleep(10000); // wait 0,1 sec
                char temp[2];
                if(read(m_serial_handle, temp, 1)>0){
                    buffer[recv_chars]=temp[0];
                    if(dbug) cout << "speichere für nr["<<recv_chars<<"]="<<temp[0]<<endl;
                    recv_chars++;
                }
            }
            tcflush(m_serial_handle, TCIFLUSH); // flush pending data
            if(dbug) cout << "return \"" << buffer << "\"" << endl;
            // eigentlich kann hier nur done stehen oder ?
            if(strncmp(buffer,"Done", 4)==0 || strncmp(buffer,"ACKD", 4)==0){ // hier kann auch "ACKD"one stehen
                setText(ui->status,-1,"successful",true);
                status_good=true;
            } else {
                // popup box
                QMessageBox msgBox;
                msgBox.setText("Error, wrong return value");
                QString text="Expected: \"Done\", got:\"";
                text.append(buffer);
                text.append("\"\n Try pilot?");
                msgBox.setInformativeText(text);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                msgBox.setEscapeButton(QMessageBox::Cancel);
                switch (msgBox.exec()) {
                case QMessageBox::Yes:
                    send_pilot();
                    break;
                default:
                    break;
                };
                // popup box
                if(dbug) cout << "bin im else" << buffer << endl;
                if(dbug) cout << "marker4" << endl;
                ui->status->append("strcmp war nicht 0");
                status_good=false;
            }
            // wechsel wieder hoch
            for(int a=1;a<subdirs.count()-1;a++){
                cd("..");
            }

            // erstelle neue Liste unten
            QString ganzer_text = ui->debug->toPlainText();
            QStringList zeilen=ganzer_text.split("\n");
            // wenn mehr als 2 Zeilen da sind, schiebe das um
            if(zeilen.count()>2){
                for(int z=0;z<zeilen.count();z++){
                    cout << "Zeile "<<z<<":" << zeilen.at(z).toStdString() << "." << endl;
                }
                setText(ui->debug,0,"",false); // zeile 0 löschen!
                setText(ui->debug,0,zeilen.at(2),false);// zeile 1 auf zeile 0 schieben
                setText(ui->debug,1," ",false);// zeile 1 auf zeile 0 schieben
            } else { // sonst leere die "liste" die ohnehin nur noch aus dem einen element besteht
                ui->debug->clear();
            }
            // ende des erfolgreichen uploades
        } else {
            if(dbug) ui->status->append("Uploading file failed! \nNo Connection to Speedoino.");
            // popup box
            QMessageBox msgBox;
            msgBox.setText("No Connection to Speedoino");
            msgBox.setInformativeText("Do you want to try a pilot?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setEscapeButton(QMessageBox::Cancel);
            switch (msgBox.exec()) {
            case QMessageBox::Yes:
                send_pilot();
                break;
            default:
                break;
            };
            // popup box
            status_good=false;
        }
    }
    // return status
    if(status_good) return true;
    else return false;
};

void MainWindow::setText(QTextEdit *textfeld,int line_nr,QString inhalt,bool append){
    QString ganz_text=textfeld->toPlainText();
    QStringList text_liste=ganz_text.split("\n");
    // line_nr umwandeln
    if(line_nr<0) // bei bla\nbla\n => 3 -> 3-1-1=> 1
        line_nr=text_liste.count()+line_nr;

    // falls line nr größer als anzahl der lines
    while(text_liste.count()<=line_nr){ // wir haben 3 felder, 0,1,2 wollen aber in 3 schreiben
        text_liste.append("");
    };

    ganz_text.clear();
    for(int a=0;a<text_liste.count();a++){
        if(a==line_nr){ // genau die zeile wollte ich ändern
            // falls inhalt "" die zeile löschen sonst
            if(inhalt!=""){
                // anhängen
                if(append){
                    inhalt=text_liste.at(a)+inhalt;
                }
                // wenn wir nicht am zeilenende sind ein \n
                if(a<text_liste.count()-1){
                    ganz_text.append(inhalt+"\n");
                } else {
                    ganz_text.append(inhalt);
                };
            };
        } else { // zeile kopieren
            // wenn wir nicht am ende sind ein \n rein
            if(a<text_liste.count()-1)
                ganz_text.append(text_liste.at(a)+"\n");
            else
                ganz_text.append(text_liste.at(a));
        }

    }
    textfeld->setText(ganz_text);
    textfeld->moveCursor(QTextCursor::End);
    textfeld->ensureCursorVisible();
    textfeld->repaint();
    textfeld->update();
}

void MainWindow::download_helper(){
    // generate filename & filesize with umrechnung to KB
    QModelIndexList indexes = ui->remote->selectionModel()->selectedIndexes();
    // drei leere variablen aufsetzen
    QStringList locals; locals.empty();
    QStringList remotes; remotes.empty();
    QStringList sizes; sizes.empty();

    for(int a=0; a<indexes.count()/4; a++){
        QModelIndex index=indexes.at(a);
        QString filename = ui->remote->currentIndex().sibling(index.row(),0).data().toString(); // dateiname nicht pfad
        QString filesize = ui->remote->currentIndex().sibling(index.row(),1).data().toString();
        int filesize_int=filesize.left(filesize.indexOf(" ")).toInt();
        if(filesize.split("KB").count()==2){
            filesize_int=filesize_int*1024;
        }
        // nur hochladen wenns nicht leer ist
        if(filename.toStdString()!="(empty)" && ui->remote->currentIndex().child(0,0).data().toString().toStdString()==""){ // eine datei wurde ausgewählt
            // local filename
            QString local_filename=local_path;
            local_filename.append(filename);

            // remote filename
            QString remote_filename=remote_path;
            remote_filename.append(filename);
            // add to list
            locals.append(local_filename);
            remotes.append(remote_filename);
            sizes.append(QString::number(filesize_int));
            // that it, comitting later
        } else if(filename.toStdString()=="(empty)"){
            ui->status->append("You've tried to download a empty dummy file. Thats not supported!");
        }else {
            ui->status->append("You've tried to download a folder. Thats not supported!");
        }
    };
    if(locals.count()>0){
        download(locals,remotes,sizes);
    }

}

bool MainWindow::download(QStringList local_filenames,QStringList remote_filenames,QStringList filesizes){
    for(int i=0;i<local_filenames.count();i++){
        int j=0;
        if(i>0) j=i+1;
        setText(ui->debug,j,local_filenames.at(i)+" <- "+remote_filenames.at(i),false);
    }
    // variablen die über alle schleifeninterationen gültig sind
    bool status_good=true;

    // für jede Datei den download
    for(int i=0;i<local_filenames.count();i++){
        QString local_filename=local_filenames.at(i);
        QString remote_filename=remote_filenames.at(i);
        int filesize=filesizes.at(i).toInt();

        QStringList subdirs=remote_filename.split("/");
        // wechsel in die subfolder
        for(int a=1;a<subdirs.count()-1;a++){
            cout << " dann geh ich mal runter " << subdirs[a].toStdString() << endl;
            cd(subdirs[a]);
        }
        // bin im richtigen folder

        // some output
        ui->status->append("Downloading file \""+remote_filename+"\"... ");


        // send read commando
        QString commando="r";
        commando.append(subdirs.at(subdirs.count()-1));
        if(dbug) cout << "Sende:" << commando.toAscii().data() << endl;
        tcflush(m_serial_handle, TCIFLUSH); // flush pending data
        write(m_serial_handle,commando.toAscii().data(),commando.length()); //senden zum deaktiviere des eventuell noch offenen f_m
        if(dbug) cout << "command gesendet" << endl;
        usleep(500000);

        // lokale ziel datei öffnen
        ofstream file (local_filename.toAscii().data());

        // lokale schleifen variablen
        QString file_copy=""; // die ganze bisherige datei
        QString line_copy=""; // die aktuelle zeile
        bool breaker=false;   // breaker ist solange false bis wir "***" finden
        char buffer[1280];    // empfangspuffer
        time_t time_entry=time(0); // Zeit die jedesmal resetet wird wenn zeichen empfangen werden
        time_t time_start=time(0); // Zeit vom Start der Übertragung als ganzes
        int file_download_counter=0; // wieviele zeichen haben wir empfangen bisher
        int dialog_show=0; // counter der hochgezählt wird und immer bei 100 den status updated
        int rate=1; // Downloadrate

        // breaker ist solange false bis wir "***" finden
        while(!breaker){
            // 3sek nichts gehört
            if(time(0)-time_entry>4) {
                breaker=true;
                if(dbug) cout << "timeout "<< endl;
                ui->status->append("Timeout while downloading.");
                status_good=false;
            };

            int empfangen=read(m_serial_handle, buffer, sizeof(buffer));
            if(empfangen>0){
                line_copy=buffer; // in den qstring kopieren
                time_entry=time(0); // timer zurücksetzen
                if(dbug) cout << line_copy.left(empfangen).toStdString();

                // ab hier empfangenen char bearbeiten
                file_copy.append(line_copy.left(empfangen));
                if(file_copy.split("***").count()!=2){ // wenn im empfangenen NICHT *** ist
                    file << line_copy.left(empfangen).toAscii().data();
                    // zählen
                    file_download_counter=file_download_counter+empfangen;
                    dialog_show=dialog_show+empfangen;
                    // anzeigen
                    if(dialog_show>100){// immer 20 Byte blöcke
                        dialog_show=0;
                        int bisher_zeit=time(0)-time_start;
                        if(bisher_zeit==0) bisher_zeit=1;
                        rate=file_download_counter/bisher_zeit;

                        int min=((filesize-file_download_counter)/rate/60)%60;
                        int sec=((filesize-file_download_counter)/rate)%60;
                        char zeit_remaining[10];
                        sprintf(zeit_remaining,"%02i:%02i min",min,sec);

                        min=bisher_zeit/60;
                        sec=bisher_zeit%60;
                        char zeit_passed[10];
                        sprintf(zeit_passed,"%02i:%02i min",min,sec);

                        QString download_status="    "+QString::QString(zeit_passed)+" passed    |    "+QString::QString(zeit_remaining);
                        download_status.append(" remaining    |    "+QString::number(100*file_download_counter/(filesize),'g',2.2)+"%    |    ");
                        download_status.append(QString::number(file_download_counter)+" / "+QString::number(filesize));
                        download_status.append(" Bytes ("+QString::number(rate)+" B/s)");
                        this->setText(ui->debug,1,download_status,false);
                        if(dbug) cout << endl;
                    }
                } else {
                    if(dbug) cout << "*** empfangen" << endl;
                    if(dbug) cout << "downloadcounter " << file_download_counter << "/" << file_copy.indexOf("***") << endl;
                    for(int a=file_download_counter; a<file_copy.indexOf("***"); a++){
                        file<<buffer[a-file_download_counter];
                    };
                    file.close();
                    breaker=true;
                };
            }
        }
        if(dbug) cout<<"while durch"<<endl;

        char done_buffer[5];
        int recv_chars=0;
        // wenn jetzt schon in unserem puffer das "***\nDone" drin steht
        if(dbug) cout << "ich habe *** an stelle " << file_copy.indexOf("***") << " gefunden, lese also ab dort+5, bis "<<file_copy.length() << endl;
        for(int a=file_copy.indexOf("***")+3+2; a<file_copy.length() && recv_chars<4; a++){ /// *** steht bei 100,101,102 empfangen wurden 105 ...
            if(dbug) cout << "kopiere " << file_copy.at(a).toAscii() << " nach done_buffer["<<a<<"]" << endl;
            done_buffer[recv_chars]=file_copy.at(a).toAscii();
            recv_chars++;
        }
        if(dbug) cout << "bisher steht in done_buffer " << done_buffer << endl;
        // eventuell noch ein paar dran basteln
        while(recv_chars<4){
            if(dbug) cout << " warte auf done krams "<< endl;
            usleep(10000); // wait 0,1 sec
            if(time(0)-time_entry>2) { // 3sek nichts gehört
                recv_chars=4;
            };
            char temp[2];
            if(read(m_serial_handle, temp, 1)>0){
                time_entry=time(0);
                done_buffer[recv_chars]=temp[0];
                recv_chars++;
                if(dbug) cout << "hab ein " << temp[0] << "empfangen" << endl;
            }
        }
        if(dbug) cout << "return \"" << done_buffer << "\"" << endl;
        // done empfangen
        if(strncmp(done_buffer,"Done", 4)==0){
            setText(ui->status,-1," completed",true);
        } else {
            // popup box
            QMessageBox msgBox;
            msgBox.setText("Error, wrong return value");
            QString text="Expected: \"Done\", got:\"";
            text.append(done_buffer);
            text.append("\"\n Try pilot?");
            msgBox.setInformativeText(text);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setEscapeButton(QMessageBox::Cancel);
            switch (msgBox.exec()) {
            case QMessageBox::Yes:
                send_pilot();
                break;
            default:
                break;
            };
            // popup box
            ui->status->append("strcmp war nicht 0,marker 5");
            status_good=false;
        };

        // wechsel wieder hoch
        for(int a=1;a<subdirs.count()-1;a++){
            cd("..");
        }

        // erstelle neue Liste unten
        QString ganzer_text = ui->debug->toPlainText();
        QStringList zeilen=ganzer_text.split("\n");
        // wenn mehr als 2 Zeilen da sind, schiebe das um
        if(zeilen.count()>2){
            setText(ui->debug,0,"",false); // zeile 0 löschen!
            setText(ui->debug,0,zeilen.at(2),false);// zeile 1 auf zeile 0 schieben
            setText(ui->debug,1," ",false);// zeile 1 auf zeile 0 schieben
        } else { // sonst leere die "liste" die ohnehin nur noch aus dem einen element besteht
            ui->debug->clear();
        }
    }
    //return
    return status_good;
}

void MainWindow::connect_fs(){
    QFile file;
    int stop_me=0;
    // durch iterieren von ttyUSB0 bis ttyUSB9
    while(stop_me<10){
        QString port="/dev/ttyUSB"+QString::number(stop_me);
        if(file.exists(port)){
            dev_path=port;
            ui->status->append("Choose /dev/ttyUSB"+QString::number(stop_me)+" as port");
            this->update();
            this->repaint();
            stop_me=99;
        }
        stop_me++;
    }

    // wenn alle ttyUSB falsch sind => rfcomm testen
    if(file.exists("/dev/rfcomm0") && stop_me!=100){
        dev_path="/dev/rfcomm0";
        ui->status->append("Choose /dev/rfcomm0 as port");
        stop_me=100;
    }

    // alles scheisse
    if(stop_me!=100){
        ui->status->append("No Device found");
    }

    //struct termios oldtio;
    this->m_serial_handle=openSerialDevice(dev_path.toAscii());
    if(this->m_serial_handle!=-1){
        // vars, buffer, ack def
        char buffer[1];
        char start[]="file_manager activated";
        unsigned int empfangen=0;
        bool breaker=false;
        unsigned int right=0;
        setText(ui->status,-1," | Sending login ... ",true);
        this->update();
        this->repaint();
        write(this->m_serial_handle,"aa",2); //deaktiviert eventuell noch offenen filemaneger
        usleep(100000);
        write(this->m_serial_handle,"*811*",5);
        usleep(100000);
        tcflush(m_serial_handle, TCIFLUSH); // flush pending data
        write(m_serial_handle,"d",1); //senden zum aktiviere des f_m
        usleep(250000);

        time_t time_entry=time(0); // aktuelle Zeit speichern
        // Zeichen empfangen
        while(empfangen<strlen(start) && !breaker){ // maximal so lange suchen wie der teststring lang ist
            if(time(0)-time_entry>2) { // 3sek nichts gehört
                empfangen=999;
            };
            if(read(m_serial_handle, buffer, 1)>0){
                if(buffer[0]==start[right]){
                    right++;
                    if(right==strlen(start)){
                        breaker=true;
                        setText(ui->status,-1," | Login passed ",true);
                        this->update();
                        this->repaint();
                    };
                } else {
                    right=0;
                }
                // debug
                if(dbug) cout << buffer[0];
                // debug
                empfangen++;
                time_entry=time(0); // reset waiting intervall
            }
        }
        if(dbug) cout << endl;
        // anzahl richtiger zeichen testen
        if(breaker){
            setText(ui->status,-1," | Connection established",true);
            ui->connect_btn->setText("disconnect");

            ui->remote->setStyleSheet(" QTreeView {show-decoration-selected: 1;   background-color: #ffffff;  }");
            ui->remote->setStyleSheet(ui->local->styleSheet());
        } else if(empfangen==999){
            setText(ui->status,-1," | timeout",true);
            ui->connect_btn->setText("connect");
            this->m_serial_handle=-1;
        } else if(empfangen!=0){ // wie auch immer das passieren soll
            setText(ui->status,-1," | Wrong data received",true);
            ui->connect_btn->setText("connect");
            this->m_serial_handle=-1;
        } else { // höh?
            setText(ui->status,-1," | Serial Port ok, but Connection failed.",true);
            ui->connect_btn->setText("connect");
            this->m_serial_handle=-1;
        }
        this->update();
        this->repaint();
    }else{
        ui->status->append("Connection failed.");
    }
};

void MainWindow::disconnect_fs(){
    struct termios oldtio;
    write(this->m_serial_handle,"*11*",1);
    usleep(100000);
//    write(this->m_serial_handle,"1",1);
//    usleep(100000);
//    write(this->m_serial_handle,"1",1);
//    usleep(100000);
//    write(this->m_serial_handle,"*",1);
//    usleep(100000);
    tcsetattr(this->m_serial_handle,TCSANOW,&oldtio);
    ui->status->append("disconnected");
    ui->connect_btn->setText("connect");
    this->m_serial_handle=-1;

    QStandardItemModel *model_r = new QStandardItemModel; // das hier ist wohl das problem weil das den konstruktor nicht überlebt
    ui->remote->setModel( model_r );

    ui->remote->setStyleSheet(" QTreeView {show-decoration-selected: 1;   background-color: #DDDDDD;  }");
}

void MainWindow::connection_fs(){
    int tries=3;
    if(this->m_serial_handle!=-1){
        disconnect_fs();
        ui->file_down->setEnabled(false);
        ui->file_up->setEnabled(false);
        ui->rec2kml->setEnabled(false);
        ui->refresh_btn->setEnabled(false);
        ui->refresh_btn->setEnabled(false);
        ui->delete_btn->setEnabled(false);
        ui->newdir_btn->setEnabled(false);
        ui->pilotbutton->setEnabled(false);
        tries=0;
    };

    for(int a=0;a<tries;a++){
        connect_fs();
        this->repaint();
        if(this->m_serial_handle!=-1){
            ui->file_down->setEnabled(true);
            ui->file_up->setEnabled(true);
            ui->refresh_btn->setEnabled(true);
            ui->refresh_btn->setEnabled(true);
            ui->delete_btn->setEnabled(true);
            ui->newdir_btn->setEnabled(true);
            ui->Animation_button->setEnabled(true);
            ui->pilotbutton->setEnabled(true);
            ls();
            tries=-1;
        } else {
            if(a<tries-1){
                QString status="connecting "+QString::number(a+2)+"/"+QString::number(tries);
                ui->status->append(status);
            };
            this->repaint();
        }
    }
}

void MainWindow::delete_helper(){
    QStringList dateien;
    QStringList type;
    QModelIndexList indexes = ui->remote->selectionModel()->selectedIndexes();
    for(int a=0; a<indexes.count(); a++){
        QModelIndex index=indexes.at(a);
        QString filename = ui->remote->currentIndex().sibling(index.row(),0).data().toString(); // dateiname nicht pfad
        QString this_type= ui->remote->currentIndex().sibling(index.row(),2).data().toString(); // dateiname nicht pfad
        bool add=true; // default: füge datei hinzu
        for(int b=0;b<dateien.count();b++){
            if(dateien.at(b)==filename) add=false; // haben wir schon
        };
        if(add){
            cout << "will " << filename.toStdString() << " vom type " << this_type.toStdString() << " löschen"<< endl;
            dateien.append(filename);
            type.append(this_type);
        };
    };
    // "globale" var
    bool all_ok=true;

    for(int a=0;a<dateien.count();a++){
        QString filename;
        bool file_mode=true;
        if(type.at(a)=="File"){
            filename=remote_path+dateien.at(a);
        } else {
            file_mode=false;
            filename=remote_path;
        };
        // an dieser stelle wäre gut rauszufinden ob das verzeichniss nicht zufällig voll ist
        // dann müsste man hier deutlich tiefsinniger löschen
        // das ist definitiv ein TODO
        if(!delete_file(filename)){
            all_ok=false;
        };
    };

    // wenn alle ok sind listing
    if(all_ok){
        this->ls();
    }
}

bool MainWindow::delete_file(QString filename){
    bool file_mode=true;
    if(filename.lastIndexOf("/")==filename.length()-1){ // der letzte "/" steht am ende -> es ist ein dir
        file_mode=false;
    };

    // wechsel remote in passenden subfolder
    QStringList subdirs=filename.split("/"); // ordner haben /asd/asd2/ = 4
    if(dbug) cout << "der remote path ist: " << remote_path.toStdString() << endl;
    int weniger=1; // also: generell müssen wir in den 1. unterordner, den 2., ... den n-1 weil n der dateiname ist
    if(!file_mode) weniger++; // wenn wir aber ein verzeichniss löschen müssen wir eine ebene höher stoppen
    for(int a=1;a<subdirs.count()-weniger;a++){
        if(dbug) cout << " dann geh ich mal runter " << subdirs[a].toStdString() << endl;
        cd(subdirs[a]);
    }
    // wechsel remote in passenden subfolder

    ui->status->append("Deleting \""+filename+"\" ... ");

    QString commando="";
    if(file_mode){
        commando.append("e");
    } else {
        commando.append("d");
    }
    // in subdirs.at(subdirs.count()-weniger) steht was wir löschen wollen
    commando.append(subdirs.at(subdirs.count()-weniger));
    if(dbug) cout << "Sende:" << commando.toAscii().data() << endl;

    write(m_serial_handle,commando.toAscii().data(),commando.length()); //senden zum deaktiviere des eventuell noch offenen f_m
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    if(dbug) cout << "command gesendet" << endl;
    char buffer[5];
    memset(buffer, 0, 5);
    usleep(500000);
    read(m_serial_handle, buffer, 4);
    if(dbug) cout << "return \"" << buffer << "\"" << endl;
    bool delete_ok;
    if(strncmp(buffer,"Done", 4)==0){
        setText(ui->status,-1," successful ",true);
        delete_ok=true;
    } else {
        // popup box
        QMessageBox msgBox;
        msgBox.setText("Error, wrong return value");
        QString text="Expected: \"Done\", got:\"";
        text.append(buffer);
        text.append("\"\n Try pilot?");
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setEscapeButton(QMessageBox::Cancel);
        switch (msgBox.exec()) {
        case QMessageBox::Yes:
            send_pilot();
            break;
        default:
            break;
        };
        // popup box
        if(dbug) cout << "bin im else" << buffer[0] << endl;
        ui->status->append("Delete failed");
        if(dbug) cout << "marker1" << endl;
        delete_ok=false;
    }

    // wechsel wieder hoch
    for(int a=1;a<subdirs.count()-weniger;a++){
        cd("..");
    }
    // return was immer das ergeben hat
    return delete_ok;
}

void MainWindow::mkdir(QString dirname){
    if(m_serial_handle!=-1){
        // wechsel remote in passenden subfolder
        QStringList subdirs=remote_path.split("/");
        for(int a=1;a<subdirs.count()-1;a++){
            cout << " dann geh ich mal runter " << subdirs[a].toStdString() << endl;
            cd(subdirs[a]);
        }
        // wechsel remote in passenden subfolder
        // generate valid filename
        QString commando;
        commando="m";
        commando.append(dirname);
        commando.append('\n');
        cout << "Sende:" << commando.toAscii().data() << endl;
        write(m_serial_handle,commando.toAscii().data(),commando.length()); //senden zum deaktiviere des eventuell noch offenen f_m
        tcflush(m_serial_handle, TCIFLUSH); // flush pending data
        cout << "command gesendet" << endl;
        usleep(1000000);
        char buffer[5];
        memset(buffer, 0, 5);
        read(m_serial_handle, buffer, 4);
        cout << "return \"" << buffer << "\"" << endl;
        if(strncmp(buffer,"Done", 4)==0){
            ui->status->append("Make directory successful");
            this->ls();
        } else {
            cout << "bin im else" << buffer << endl;
            ui->status->append("strcmp war nicht 0");
            cout << "marker2" << endl;
        }
    } else {
            ui->status->append("Not connected to Speedoino");
    }
}

void MainWindow::ls_dir(QString dir,QStandardItemModel *model, QStandardItem *folder,int level){
    //cout << "betrete subdir " << dir.toStdString() << endl;

    if(dir.toStdString()!="..") cd(dir.toAscii().data());
    setText(ui->status,-1," | ls("+ dir +")...",true);
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    QString ls_result="";
    char buffer[2];
    char done[]="Done";
    int pos=0;

    int empfangen=0;
    bool breaker=false;
    unsigned int right=0;

    //senden zum listen
    if(dbug) cout << "sende list befehl " << endl;
    write(m_serial_handle,"l",1);
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    time_t time_entry=time(0); // aktuelle Zeit speichern

    // empfangen
    while(empfangen<99999 && !breaker){ // maximal so lange suchen wie der teststring lang ist
        if(time(0)-time_entry>4) { // 3sek nichts gehört
            empfangen=99999;
            if(dbug) cout << "timeout beim empfangen" << endl;
        };
        if(read(m_serial_handle, buffer, 1)>0){
            time_entry=time(0);
            ls_result.append(buffer[0]);
            if(dbug) cout << buffer[0] << endl;
            if(buffer[0]==done[right]){
                if(dbug) cout << " richtig! das ist nr " << right << endl;
                right++;
                if(right==strlen(done)){
                    if(dbug) cout << "aus!" << endl;
                    breaker=true;
                };
            } else {
                right=0;
            }
        };
    };
    if(dbug) cout << "Daten vollständig empfangen" << endl;
    if(dbug) cout << ls_result.toStdString() << endl << "===========" << endl;
    // interpretieren
    if(breaker){
        //////////////
        QString temp;
        int ls_zeiger=0;
        //cout << "empf " << ls_result.length() << endl;
        if(ls_result.length()==4){
            QStandardItem *file = new QStandardItem("(empty)");
            folder->appendRow(file);
        }
        bool skip_files=true;

        while(ls_zeiger<ls_result.length()){
            // zweimal durchgehen
            if(ls_zeiger+1==ls_result.length() && skip_files){
                ls_zeiger=0;
                skip_files=false;
                temp.clear();
            };
            // ein zeichen einlesen
            if(ls_result[ls_zeiger]!='\n'){
                temp.append(ls_result[ls_zeiger]);
            } else if(ls_result[ls_zeiger]=='\n') { // zeilen umbruch -> wir fangen von vorn aus
                QStringList zeilensegmente=temp.split(" ");
                temp.clear();
                if(zeilensegmente.count()>1){ // gut
                    QStringList folder_test=zeilensegmente.at(0).split("/");

                    if(folder_test.count()>1 && skip_files && folder_test.at(0)!="FOUND.000" && folder_test.at(0)!="TRASH-~1"){ // wir haben einen ordner und wollen den auch noch
                        QStandardItem *subfolder = new QStandardItem(QString("%0").arg(folder_test.at(0)));
                        subfolder->setIcon(QIcon(QPixmap(":/trolltech/styles/commonstyle/images/parentdir-128.png")));
                        QStandardItem *file_size = new QStandardItem(QString("%0").arg(""));
                        QStandardItem *file_type = new QStandardItem(QString("%0").arg("Folder"));
                        // get last mod
                        QString file_modif_string = zeilensegmente.at(zeilensegmente.count()-2);
                        file_modif_string=file_modif_string.right(file_modif_string.length()-2);
                        QStringList datum=file_modif_string.split("-");
                        file_modif_string=datum.at(2)+"."+datum.at(1)+"."+datum.at(0);
                        file_modif_string.append(" "+zeilensegmente.at(zeilensegmente.count()-1));
                        file_modif_string=file_modif_string.left(file_modif_string.length()-1 );
                        QStandardItem *file_modif = new QStandardItem(QString("%0").arg(file_modif_string));
                        if(level==0){
                            model->setItem(pos,0,subfolder);
                            model->setItem(pos,1,file_size);
                            model->setItem(pos,2,file_type);
                            model->setItem(pos,3,file_modif);
                            // DEBUG ///
                            QString tmp;
                            tmp="Haenge den Ordner \"";
                            tmp.append(subfolder->text().toAscii());
                            tmp.append("\" an / an ");
                            //ui->debug->append(tmp);
                            //cout << tmp.toStdString();
                            // DEBUG ///
                            pos++;
                        } else {
                            QList<QStandardItem*> * row = new QList<QStandardItem*>;
                            *row << subfolder << file_size << file_type << file_modif;
                            folder->appendRow( *row );
                            // DEBUG ///
                            QString tmp;
                            tmp="Haenge den Ordner \"";
                            tmp.append(subfolder->text().toAscii());
                            tmp.append("\" an den Unterordner ");
                            tmp.append(folder->text().toAscii());
                            tmp.append(" an");
                            //ui->debug->append(tmp);
                            //cout << tmp.toStdString() << endl;
                            // DEBUG ///
                        }
                        this->update();
                        this->repaint();

                        ls_dir(subfolder->text().toAscii(),model,subfolder,level+1); // rekursion
                    } else if(folder_test.count()==1 && !skip_files){ // wir haben eine datei und wollen das auch
                        QStandardItem *file = new QStandardItem(QString("%0").arg(folder_test.at(0)));
                        file->setIcon(QIcon(QPixmap(":/trolltech/styles/commonstyle/images/file-128.png")));
                        // get filesize
                        QString file_size_string = zeilensegmente.at(zeilensegmente.count()-1);
                        file_size_string=file_size_string.left(file_size_string.length()-1);
                        // convert to kb
                        if(file_size_string.toInt()>=1000){
                            file_size_string=QString::number(file_size_string.toInt()/1024)+" KB";
                        } else {
                            file_size_string.append(" bytes");
                        }
                        // get last mod
                        QString file_modif_string = zeilensegmente.at(zeilensegmente.count()-3);
                        file_modif_string=file_modif_string.right(file_modif_string.length()-2);
                        QStringList datum=file_modif_string.split("-");
                        file_modif_string=datum.at(2)+"."+datum.at(1)+"."+datum.at(0);
                        file_modif_string.append(" "+zeilensegmente.at(zeilensegmente.count()-2));

                        QStandardItem *file_size = new QStandardItem(QString("%0").arg(file_size_string));
                        QStandardItem *file_type = new QStandardItem(QString("%0").arg("File"));
                        QStandardItem *file_modif = new QStandardItem(QString("%0").arg(file_modif_string));

                        if(level==0){
                            model->setItem(pos,0,file);
                            model->setItem(pos,1,file_size);
                            model->setItem(pos,2,file_type);
                            model->setItem(pos,3,file_modif);

                            // DEBUG ///
                            QString tmp;
                            tmp="Heange die Datei \"";
                            tmp.append(file->text().toAscii());
                            tmp.append("\" an / ");
                            //ui->debug->append(tmp);
                            //cout << tmp.toStdString() << endl;
                            // DEBUG ///
                            pos++;
                        } else {
                            QList<QStandardItem*> * row = new QList<QStandardItem*>;
                            *row << file << file_size << file_type << file_modif;
                            folder->appendRow( *row );
                            // DEBUG ///
                            QString tmp;
                            tmp="Haenge die Datei \"";
                            tmp.append(file->text().toAscii());
                            tmp.append("\" an den folder ");
                            tmp.append(folder->text().toAscii());
                            tmp.append(" an");
                            //ui->debug->append(tmp);
                            //cout << tmp.toStdString() << endl;
                            // DEBUG ///
                        };
                    };
                };
            };
            ls_zeiger++;
        };
        setText(ui->status,-1," ok",true);
    }

    //cout << "Und wieder eine level hoeher " << endl;
    cd("..");
};

void MainWindow::ls(){
    if(dbug) cout << "ls gestartet ";
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    ui->status->append("listing dirs...");

    QStandardItemModel *model_r = new QStandardItemModel; // das hier ist wohl das problem weil das den konstruktor nicht überlebt
    QStandardItem *folder = new QStandardItem(); // leeres item, damit der ls aufruf stimmt
    QString dir="..";
    ls_dir(dir,model_r,folder,0);
    model_r->setHorizontalHeaderItem( 0, new QStandardItem( "Name" ) );
    model_r->setHorizontalHeaderItem( 1, new QStandardItem( "Size" ) );
    model_r->setHorizontalHeaderItem( 2, new QStandardItem( "Type" ) );
    model_r->setHorizontalHeaderItem( 3, new QStandardItem( "Date Modified" ) );
    ui->remote->setModel( model_r );
    ui->remote->header()->resizeSection(0,190);
    ui->remote->header()->resizeSection(1,79);
    ui->remote->header()->resizeSection(2,60);
    ui->remote->header()->resizeSection(3,120);
    ui->remote->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //ui->remote->sortByColumn(0, Qt::AscendingOrder);

    setText(ui->status,-1," | listing finished ",true);
}

bool MainWindow::cd(QString dir){
    QString commando;
    commando="c";
    commando.append(dir);
    if(dbug) cout << "Sende:" << commando.toAscii().data() << endl;
    //usleep(500000);
    write(m_serial_handle,commando.toAscii().data(),commando.length()); //senden zum deaktiviere des eventuell noch offenen f_m
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    if(dbug) cout << "command gesendet" << endl;
    char buffer[5];
    memset(buffer, 0, 5);
    // warte auf 4 byte
    int recv_chars=0;
    time_t time_entry=time(0); // aktuelle Zeit speichern
    while(recv_chars<4){
        if(time(0)-time_entry>5){ recv_chars=4; };
        usleep(10000); // wait 0,1 sec
        char temp[2];
        if(read(m_serial_handle, temp, 1)>0){
            buffer[recv_chars]=temp[0];
            recv_chars++;
            time_entry=time(0); // aktuelle Zeit speichern
        }
    }
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    // warte auf 4 byte
    if(dbug) cout << "return \"" << buffer << "\"" << endl;
    if(strncmp(buffer,"Done", 4)==0){
        QString helper="cd to directory \"";
        helper.append(dir);
        helper.append("\" successful");
        //ui->status->append(helper);
        return true;
    } else {
        cout << "bin im else" << buffer << endl;
        ui->status->append("strcmp war nicht 0");
        cout << "marker3" << endl;
        return false;
    }
}

void MainWindow::close_newdir_window(){
    ui->createdir_area->hide();
};

void MainWindow::create_newdir_window(){
    ui->createdir_area->hide();
    mkdir(ui->newdir_text->toPlainText());
}

void MainWindow::show_newdir_window(){

/*
    QMessageBox msgBox;
    msgBox.setText("The document has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
*/

    ui->newdir_text->setText("enter name");
    ui->newdir_text->selectAll();
    ui->createdir_area->show();
    ui->newdir_text->setFocus();

}

void MainWindow::set_device_USB0(){ dev_path="/dev/ttyUSB0"; }

void MainWindow::set_device_USB1(){ dev_path="/dev/ttyUSB1"; }

void MainWindow::set_device_rfcomm0(){ dev_path="/dev/rfcomm0"; }

void MainWindow::clickkml(){
    // open local file
    QString local_filename=local_path;
    QString r_filename = ui->local->currentIndex().sibling(ui->local->currentIndex().row(),0).data().toString();
    local_filename.append(r_filename);

    //kml2navi("/home/kolja/.googleearth/myplaces.kml");
    kml2navi(local_filename);
}

void MainWindow::kml2navi(QString kml){
    QFile kml_datei(kml);
    if(kml_datei.exists(kml)){
        if(kml_datei.open(QIODevice::ReadOnly | QIODevice::Text)){

            QString file_content=kml_datei.readAll();
            QStringList zeilen=file_content.split("\n");
            file_content.clear();
            for(int a=0;a<zeilen.count();a++){
                bool found=true;
                QStringList felder=zeilen.at(a).split("<name>l,");
                // suchen
                if(felder.count()!=2){
                    felder=zeilen.at(a).split("<name>r,");
                    if(felder.count()!=2){
                        felder=zeilen.at(a).split("<name>g,");
                        if(felder.count()!=2){
                            found=false;
                        }
                    }
                }
                // so, genug gesucht
                if(found){
                    QString zeile=zeilen.at(a);
                    zeile=zeile.right(zeile.length()-zeile.indexOf("<name>")-6);
                    zeile=zeile.left(zeile.indexOf("</name>"));
                    // richtung umbiegen
                    QString richtung=zeile.left(1);
                    richtung.replace("l","1");
                    richtung.replace("g","0");
                    richtung.replace("r","2");
                    zeile=richtung+zeile.right(zeile.length()-1);
                    // sonderzeichen raus
                    zeile.replace("ü","ue");
                    zeile.replace("ö","oe");
                    zeile.replace("ä","ae");
                    zeile.replace("ß","ss");
                    // kommata im text loeschen
                    zeile=zeile.left(2)+zeile.right(zeile.length()-2).replace(",","");
                    // lnge krzen
                    if(zeile.length()>12) zeile=zeile.left(11)+".";
                    while(zeile.length()<15) zeile.append(" "); // auch wenn nur die ersten 12 Zeichen angezeigt werden werden 15 erzeugt
                    QString anweisung=zeile;

                    a++;
                    while(zeilen.at(a).split("<coordinates>").count()!=2){
                        a++;
                    };
                    zeile=zeilen.at(a);
                    zeile=zeile.right(zeile.length()-zeile.indexOf("<coordinates>")-13);
                    zeile=zeile.left(zeile.indexOf("</coordinates>")-2);

                    QString ganz_zahl=zeile.left(zeile.indexOf(","));
                    QString vor_komma=ganz_zahl.left(ganz_zahl.indexOf("."));
                    QString nach_komma=ganz_zahl.right(ganz_zahl.length()-ganz_zahl.indexOf(".")-1 );
                    nach_komma=nach_komma.left(6);
                    if(vor_komma.toInt()<10) vor_komma="00"+vor_komma;
                    else if(vor_komma.toInt()<100) vor_komma="0"+vor_komma;
                    QString longitude=vor_komma+nach_komma;

                    ganz_zahl=zeile.right(zeile.length()-zeile.indexOf(",")-1);
                    vor_komma=ganz_zahl.left(ganz_zahl.indexOf("."));
                    nach_komma=ganz_zahl.right(ganz_zahl.length()-ganz_zahl.indexOf(".")-1 );
                    nach_komma=nach_komma.left(6);
                    if(vor_komma.toInt()<10) vor_komma="00"+vor_komma;
                    else if(vor_komma.toInt()<100) vor_komma="0"+vor_komma;
                    QString latitude=vor_komma+nach_komma;

                    file_content.append(latitude+","+longitude+","+anweisung+"\n");
                }
            }
            QMessageBox msgBox;
            msgBox.setText(file_content);
            msgBox.setInformativeText("File correct?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setEscapeButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            bool upload=false;
            switch (ret) {
              case QMessageBox::Yes:
                  upload=true;
                  break;
              default:
                  break;
              };
            if(upload){
                QString filename="/tmp/navi.txt";
                ofstream file (filename.toAscii());
                file << file_content.toStdString();
                file.close();
                QString temp="/NAVI.TXT";
                this->upload(filename.split("***///***"),temp.split("***///***"));
            }
        }
    }
}

void MainWindow::conv_imag(){
    QString local_filename=local_path;
    QString r_filename = ui->local->currentIndex().sibling(ui->local->currentIndex().row(),0).data().toString();
    local_filename.append(r_filename);
    QString command="../image_converter-build-desktop/image_converter "+local_filename;
    system(command.toAscii().data());

    QFile file;
    QString filename_new=local_filename.split("/").at(local_filename.split("/").count()-1);
    filename_new=filename_new.split(".").at(0);

    QString old_remote_path=remote_path; // to save
    remote_path="/gfx/"; // hart gesetzt
    if(file.exists("/tmp/"+filename_new+".TXT")){
        if(upload(QString::QString("/tmp/"+filename_new+".TXT").split("***///***"),QString::QString(remote_path+filename_new+".TXT").split("***///***"))){
            this->ls();
            tcflush(m_serial_handle, TCIFLUSH); // flush pending data
            QString commando="s"+filename_new+".TXT";
            write(m_serial_handle,commando.toAscii().data(),commando.length());
            tcflush(m_serial_handle, TCIFLUSH); // flush pending data
        }
    };
    remote_path=old_remote_path; //old
}

bool MainWindow::rec2kml(QString rec, QString local_kml_file){
    QFile file(rec);
    cout << "rec2kml startet" << endl;
    if(file.exists(rec)){
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){ return false; };
        cout << "file found" << endl;
        QString kml_file="<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://earth.google.com/kml/2.1\"><Document><name>"+local_kml_file.split("/").at(local_kml_file.split("/").count()-1)+"</name>\n\n";
        QString file_content=file.readAll();
        QStringList zeilen=file_content.split("\n");
        cout << "zeilen:" << zeilen.count() << endl;
        for(int a=0; a<zeilen.count();a++){
            QStringList felder=zeilen.at(a).split(",");
            if(felder.count()==10){
                QString temp;

                temp=QString::number(felder.at(3).right(6).toFloat()/10000/60);
                temp=felder.at(3).left(3)+"."+temp.right(temp.length()-2);
                QString latitude=temp;
                temp=QString::number(felder.at(2).right(6).toFloat()/10000/60);
                temp=felder.at(2).left(3)+"."+temp.right(temp.length()-2);
                QString longitude=temp;

                kml_file.append("\n\n<Placemark id=\"point\">");
                kml_file.append("<name></name>");
                kml_file.append("<visibility>1</visibility>");
                kml_file.append("<description><![CDATA[<TABLE border=\"0\">");
                kml_file.append("<TR><TD colspan=\"2\" align=\"center\"><b><u>Speedoino</u></b></TD></TR>");
                kml_file.append("<TR><TD><b>Zeit</b></TD><TD ALIGN=\"right\">"+felder.at(0).left(2)+":"+felder.at(0).left(5).right(2)+":"+felder.at(0).left(8).right(2)+"</TD></TR>");
                kml_file.append("<TR><TD colspan=\"2\" align=\"center\">"+felder.at(4)+" Km/h</TD></TR>");
                kml_file.append("<TR><TD colspan=\"2\" align=\"center\">"+felder.at(7)+" Sateliten</TD></TR>");
                if(felder.at(8).toInt()==1) { temp="fixiert"; } else { temp="kein Fix"; };
                kml_file.append("<TR><TD colspan=\"2\" align=\"center\">"+temp+"</TD></TR>");
                kml_file.append("<TR><TD>Höhe</TD><TD ALIGN=\"right\">"+QString::number(felder.at(6).toFloat()/10)+" Meter</TD></TR>");
                kml_file.append("<TR><TD>Kurs</TD><TD ALIGN=\"right\">"+QString::number(felder.at(5).toInt())+" Grad</TD></TR>");
                kml_file.append("<TR><TD>Longitude</TD><TD ALIGN=\"right\">"+longitude+"</TD></TR>");
                kml_file.append("<TR><TD>Latitude</TD><TD ALIGN=\"right\">"+latitude+"</TD></TR></TABLE>]]>");
                int color=255;
                if(felder.at(4).toInt()<150) color=felder.at(4).toInt()*255/150;
                temp.sprintf("FF00%02x%02x",color,255); // AA BB GG RR
                kml_file.append("</description><Style><IconStyle><color>"+temp+"</color><scale>0.25</scale>");
                kml_file.append("<Icon><href>http://www.tischlerei-windeler.de/punkt.png</href></Icon></IconStyle></Style>"); // http://192.168.5.196/gps_krams/punkt.png
                kml_file.append("<Point id=\"point_point\"><altitudeMode>relativeToGround</altitudeMode><tessellate>1</tessellate>");
                kml_file.append("<extrude>0</extrude><coordinates>"+latitude+","+longitude+",0</coordinates></Point></Placemark>");
            };
        }
        kml_file.append("</Document></kml>");

        cout << "kml landet in " << local_kml_file.toAscii().data() << endl;
        ofstream out(local_kml_file.toAscii());
        out << kml_file.toStdString();

        out.close();
        cout << "everything good" << endl;
        return true;
    }
    cout << "inputfile no existing" << endl;
    return false;
};

void MainWindow::remote_click2kml(){
    QModelIndexList indexes = ui->remote->selectionModel()->selectedIndexes();
    for(int a=0; a<indexes.count()/4; a++){
        QModelIndex index=indexes.at(a);
        QString filename = ui->remote->currentIndex().sibling(index.row(),0).data().toString(); // dateiname nicht pfad
        QString filesize = ui->remote->currentIndex().sibling(index.row(),1).data().toString();

        int filesize_int=filesize.left(filesize.indexOf(" ")).toInt();
        if(filesize.split("KB").count()==2){
            filesize_int=filesize_int*1024;
        }

        if(filename.toStdString()!="(empty)" && ui->remote->currentIndex().child(0,0).data().toString().toStdString()==""){ // eine datei wurde ausgewählt
            // local
            QString local_filename=local_path;
            local_filename.append(filename.split(".").at(0)+".kml");

            // remote
            QString remote_filename=remote_path;
            remote_filename.append(filename);

            // convert to List
            QStringList temp_r; temp_r.clear(); temp_r.append(remote_filename);
            QStringList temp_l; temp_l.clear(); temp_l.append("/tmp/to_convert.txt");
            QStringList temp_s; temp_s.clear(); temp_s.append(QString::number(filesize_int));

            download(temp_l,temp_r,temp_s);
            ui->status->append("Converting to kml file ...");
            this->update();
            this->repaint();

            if(rec2kml("/tmp/to_convert.txt",local_filename)){
                setText(ui->status,-1," done",true);
                QMessageBox msgBox;
                msgBox.setText("Do you want to run GoogleEarth now?");
                msgBox.setInformativeText("Run GoogleEarth?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                msgBox.setEscapeButton(QMessageBox::Cancel);
                int ret = msgBox.exec();
                switch (ret) {
                case QMessageBox::Yes: {
                        QString commando="googleearth "+local_filename+" &";
                        system(commando.toAscii());
                        break;
                    }
                default:
                    break;
                };
            } else {
                ui->status->append("Converting FAILED");
            }
            this->update();
            this->repaint();

        } else if(filename.toStdString()=="(empty)"){
            ui->status->append("You've tried to download a empty dummy file. Thats not supported!");
        }else {
            ui->status->append("You've tried to download a folder. Thats not supported!");
        }
    };
}

void MainWindow::local_click2kml(){
    QString filename = ui->local->currentIndex().sibling(ui->local->currentIndex().row(),0).data().toString();
    QString local_dest_filename=local_path;
    QString local_src_filename=local_path;
    local_src_filename.append(filename);
    local_dest_filename.append(filename.split(".").at(0)+".kml");

    cout << "calling rec2kml(" << local_src_filename.toStdString() << "," << local_dest_filename.toStdString() << ")" << endl;
    if(rec2kml(local_src_filename,local_dest_filename)){
        ui->status->append("Converting done");
        QMessageBox msgBox;
        msgBox.setText("Do you want to run GoogleEarth now?");
        msgBox.setInformativeText("Run GoogleEarth?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setEscapeButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Yes: {
                QString commando="googleearth "+local_dest_filename+" &";
                system(commando.toAscii());
                break;
            }
        default:
            break;
        };
    } else {
        ui->status->append("Converting FAILED");
    }
    this->update();
    this->repaint();

}

void MainWindow::send_pilot(){
    QString commando="p";
    tcflush(m_serial_handle, TCIFLUSH); // flush pending data
    write(m_serial_handle,commando.toAscii().data(),commando.length());
    time_t start_time=time(0);
    bool  search = true;
    char temp[2];
    char pilot[6]="pilot";
    int right=0;
    while(search){
        if(time(0)-start_time>4) search=false;
        usleep(10000); // wait 0,1 sec
        if(read(m_serial_handle, temp, 1)>0){
            cout << "hab empfangen "<< temp[0] << endl;
            if(temp[0]==pilot[right]){
                right++;
            } else {
                right=0;
            }
            if(right==5) search=false;
        }
    }
    QString text;
    if(right==5){
        text="Status has been recovered";
    } else {
        text="Status has NOT been recovered";
    }
    // popup box
    QMessageBox msgBox;
    msgBox.setText("Recovering result");
    msgBox.setInformativeText(text);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setEscapeButton(QMessageBox::Ok);
    msgBox.exec();
    // popup box
}

void MainWindow::show_animation_box(){
    if(ui->Animation_menu->isHidden()){
        ui->Animation_menu->show();
    } else {
        ui->Animation_menu->hide();
    }
}

void MainWindow::start_animation(){
    if(m_serial_handle!=-1){
        QString commando="s";
        commando.append(ui->ani_filename->text()+","+ui->ani_start->text()+","+ui->ani_ende->text()+","+ui->ani_delay->text()+"!");
        tcflush(m_serial_handle, TCIFLUSH); // flush pending data
        write(m_serial_handle,commando.toAscii().data(),commando.length());
        ui->status->append("Animation command \""+commando+"\" send");
        ui->Animation_menu->hide();
    } else {
        ui->status->append("No connection to Speedoino!");
    };
}
