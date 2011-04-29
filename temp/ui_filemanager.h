/********************************************************************************
** Form generated from reading UI file 'filemanager.ui'
**
** Created: Tue Apr 5 13:38:44 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEMANAGER_H
#define UI_FILEMANAGER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QTreeView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionBeenden;
    QAction *action_dev_ttyUSB0;
    QAction *action_dev_rfcomm;
    QWidget *centralWidget;
    QTreeView *remote;
    QPushButton *file_up;
    QPushButton *file_down;
    QPushButton *connect_btn;
    QLabel *Status;
    QLabel *Input;
    QTreeView *local;
    QTextEdit *textEdit;
    QMenuBar *menuBar;
    QMenu *menuDatei;
    QMenu *menuPort;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(708, 467);
        actionBeenden = new QAction(MainWindow);
        actionBeenden->setObjectName(QString::fromUtf8("actionBeenden"));
        action_dev_ttyUSB0 = new QAction(MainWindow);
        action_dev_ttyUSB0->setObjectName(QString::fromUtf8("action_dev_ttyUSB0"));
        action_dev_rfcomm = new QAction(MainWindow);
        action_dev_rfcomm->setObjectName(QString::fromUtf8("action_dev_rfcomm"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        remote = new QTreeView(centralWidget);
        remote->setObjectName(QString::fromUtf8("remote"));
        remote->setGeometry(QRect(360, 110, 341, 291));
        file_up = new QPushButton(centralWidget);
        file_up->setObjectName(QString::fromUtf8("file_up"));
        file_up->setGeometry(QRect(320, 80, 31, 28));
        file_down = new QPushButton(centralWidget);
        file_down->setObjectName(QString::fromUtf8("file_down"));
        file_down->setGeometry(QRect(360, 80, 31, 28));
        connect_btn = new QPushButton(centralWidget);
        connect_btn->setObjectName(QString::fromUtf8("connect_btn"));
        connect_btn->setGeometry(QRect(0, 80, 103, 28));
        Status = new QLabel(centralWidget);
        Status->setObjectName(QString::fromUtf8("Status"));
        Status->setGeometry(QRect(120, 87, 160, 18));
        Input = new QLabel(centralWidget);
        Input->setObjectName(QString::fromUtf8("Input"));
        Input->setGeometry(QRect(620, 80, 70, 18));
        local = new QTreeView(centralWidget);
        local->setObjectName(QString::fromUtf8("local"));
        local->setGeometry(QRect(0, 110, 351, 291));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(10, 0, 691, 71));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 708, 26));
        menuDatei = new QMenu(menuBar);
        menuDatei->setObjectName(QString::fromUtf8("menuDatei"));
        menuPort = new QMenu(menuBar);
        menuPort->setObjectName(QString::fromUtf8("menuPort"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuDatei->menuAction());
        menuBar->addAction(menuPort->menuAction());
        menuDatei->addAction(actionBeenden);
        menuPort->addAction(action_dev_ttyUSB0);
        menuPort->addAction(action_dev_rfcomm);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionBeenden->setText(QApplication::translate("MainWindow", "Beenden", 0, QApplication::UnicodeUTF8));
        action_dev_ttyUSB0->setText(QApplication::translate("MainWindow", "/dev/ttyUSB0", 0, QApplication::UnicodeUTF8));
        action_dev_rfcomm->setText(QApplication::translate("MainWindow", "/dev/rfcomm", 0, QApplication::UnicodeUTF8));
        file_up->setText(QApplication::translate("MainWindow", "->", 0, QApplication::UnicodeUTF8));
        file_down->setText(QApplication::translate("MainWindow", "<-", 0, QApplication::UnicodeUTF8));
        connect_btn->setText(QApplication::translate("MainWindow", "Connect", 0, QApplication::UnicodeUTF8));
        Status->setText(QApplication::translate("MainWindow", "TextLabel", 0, QApplication::UnicodeUTF8));
        Input->setText(QApplication::translate("MainWindow", "TextLabel", 0, QApplication::UnicodeUTF8));
        menuDatei->setTitle(QApplication::translate("MainWindow", "Datei", 0, QApplication::UnicodeUTF8));
        menuPort->setTitle(QApplication::translate("MainWindow", "Port", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEMANAGER_H
