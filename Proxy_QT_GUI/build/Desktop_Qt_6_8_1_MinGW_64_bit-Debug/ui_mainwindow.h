/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *StartProxy;
    QPushButton *CloseProxy;
    QPushButton *Add;
    QPushButton *Remove;
    QLineEdit *InputName;
    QListWidget *Ban_list;
    QListWidget *listWidget;
    QListWidget *listWidget_2;
    QFrame *line;
    QFrame *line_2;
    QFrame *line_3;
    QFrame *line_4;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(922, 521);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        StartProxy = new QPushButton(centralwidget);
        StartProxy->setObjectName("StartProxy");
        StartProxy->setGeometry(QRect(200, 460, 111, 41));
        CloseProxy = new QPushButton(centralwidget);
        CloseProxy->setObjectName("CloseProxy");
        CloseProxy->setGeometry(QRect(340, 460, 111, 41));
        Add = new QPushButton(centralwidget);
        Add->setObjectName("Add");
        Add->setGeometry(QRect(550, 460, 131, 41));
        Remove = new QPushButton(centralwidget);
        Remove->setObjectName("Remove");
        Remove->setGeometry(QRect(770, 460, 121, 41));
        InputName = new QLineEdit(centralwidget);
        InputName->setObjectName("InputName");
        InputName->setGeometry(QRect(550, 420, 341, 31));
        Ban_list = new QListWidget(centralwidget);
        Ban_list->setObjectName("Ban_list");
        Ban_list->setGeometry(QRect(550, 260, 341, 151));
        listWidget = new QListWidget(centralwidget);
        listWidget->setObjectName("listWidget");
        listWidget->setGeometry(QRect(30, 40, 421, 411));
        listWidget_2 = new QListWidget(centralwidget);
        listWidget_2->setObjectName("listWidget_2");
        listWidget_2->setGeometry(QRect(550, 41, 341, 181));
        line = new QFrame(centralwidget);
        line->setObjectName("line");
        line->setGeometry(QRect(901, 10, 20, 501));
        line->setFrameShape(QFrame::Shape::VLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);
        line_2 = new QFrame(centralwidget);
        line_2->setObjectName("line_2");
        line_2->setGeometry(QRect(10, 0, 901, 21));
        line_2->setFrameShape(QFrame::Shape::HLine);
        line_2->setFrameShadow(QFrame::Shadow::Sunken);
        line_3 = new QFrame(centralwidget);
        line_3->setObjectName("line_3");
        line_3->setGeometry(QRect(10, 500, 901, 21));
        line_3->setFrameShape(QFrame::Shape::HLine);
        line_3->setFrameShadow(QFrame::Shadow::Sunken);
        line_4 = new QFrame(centralwidget);
        line_4->setObjectName("line_4");
        line_4->setGeometry(QRect(0, 10, 20, 501));
        line_4->setFrameShape(QFrame::Shape::VLine);
        line_4->setFrameShadow(QFrame::Shadow::Sunken);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        StartProxy->setText(QCoreApplication::translate("MainWindow", "StartProxy", nullptr));
        CloseProxy->setText(QCoreApplication::translate("MainWindow", "CloseProxy", nullptr));
        Add->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        Remove->setText(QCoreApplication::translate("MainWindow", "Remove", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
