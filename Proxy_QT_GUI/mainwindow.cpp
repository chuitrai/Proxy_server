#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QMessageBox>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    proxyProcess(new QProcess(this))  // Initialize QProcess object
{
    ui->setupUi(this);

   // connect(ui->Add, &QPushButton::clicked, this, &MainWindow::on_Add_clicked());

    // Kết nối sự kiện cho nút Xóa
    //connect(ui->Remove, &QPushButton::clicked, this, &MainWindow::on_Remove_clicked());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_StartProxy_clicked()
{
    QString exePath = "C:/Users/ADMIN/OneDrive - VNU-HCMUS/Nam_2/HK1/MMT/Proxy/proxy.exe"; //Path to your exe

    //Read the banned list from Ban_list.txt
    QFile file("C:/Users/ADMIN/OneDrive - VNU-HCMUS/Nam_2/HK1/MMT/Proxy_QT_GUI/Ban_list");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Ban_list.txt has problem!";
        return;
    }
    ui->Ban_list->clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        ui->Ban_list->addItem(line);
    }
    file.close();
    //Read ban list
    QStringList ban_list;

    for (int i = 0; i<ui->Ban_list->count(); i++) {
        ban_list << ui->Ban_list->item(i)->text();
    }

    //QMessageBox::information(this,"None", exePath);
    // Start the process
    proxyProcess->start(exePath, ban_list);

    if (proxyProcess->waitForStarted()) {
        QMessageBox::information(this, "Process Started", "The process has been started successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to start the process.");
    }
}

void MainWindow::on_CloseProxy_clicked()
{
    if (proxyProcess->state() == QProcess::Running) {
        // Try to terminate the process gracefully
        proxyProcess->terminate();

        // Wait for the process to finish or timeout after a short duration (e.g., 2 seconds)
        if (!proxyProcess->waitForFinished(2000)) {
            // If the process doesn't terminate gracefully, forcefully kill it
            proxyProcess->kill();
            QMessageBox::warning(this, "Force Kill", "The process was forcefully terminated.");
        } else {
            QMessageBox::information(this, "Process Stopped", "The process has been stopped.");
        }
    } else {
        QMessageBox::information(this, "No Process", "No running process to stop.");
    }
}

void MainWindow::on_Add_clicked()
{
    QString name = ui->InputName->text();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Empty!");
        return;
    }

    // Ghi vao list
    ui->Ban_list->addItem(name);
    ui->InputName->clear();

    //Ghi vao file Ban_list
    QFile file("C:/Users/ADMIN/OneDrive - VNU-HCMUS/Nam_2/HK1/MMT/Proxy_QT_GUI/Ban_list");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Ban_list.txt has problem!";
        return;
    }
    QTextStream out(&file);
    out << '\n' << name;
    file.close();
}


void MainWindow::on_Remove_clicked()
{
    QListWidgetItem *selectedItem = ui->Ban_list->currentItem();
    // xoa trong ban list
    QFile file("C:/Users/ADMIN/OneDrive - VNU-HCMUS/Nam_2/HK1/MMT/Proxy_QT_GUI/Ban_list");
    QStringList tmp;
    QTextStream out(&file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Ban_list.txt has problem!";
        return;
    }
    for (int i = 0; i<ui->Ban_list->count(); i++) {
        if (ui->Ban_list->item(i)->text() != ui ->Ban_list->currentItem()->text())
            out << ui->Ban_list->item(i)->text();
    }
    file.close();

    // xoa trong list widget
    if (!selectedItem) {
        QMessageBox::warning(this, "Warning", "Nothing to remove!");
        return;
    }
    delete selectedItem;
}


void MainWindow::on_Reset_clicked()
{
    ui->CloseProxy->click();
    ui->StartProxy->click();
}

