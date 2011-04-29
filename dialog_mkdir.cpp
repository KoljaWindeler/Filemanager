#include <QtGui>
#include "dialog_mkdir.h"

// if we include <QtGui> there is no need to include every class used: <QString>, <QFileDialog>,...
myQtApp::myQtApp(QWidget *parent):
{
    setupUi(this); // this sets up GUI
    // signals/slots mechanism in action

}

QString myQtApp::okay(){
    return ;
}

