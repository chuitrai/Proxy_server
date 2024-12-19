#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_StartProxy_clicked();  // Slot to start the process
    void on_CloseProxy_clicked();  // Slot to stop the process

    void on_Add_clicked();
    void on_Remove_clicked();

    void on_Reset_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *proxyProcess;  // QProcess object for managing the process
};

#endif // MAINWINDOW_H
