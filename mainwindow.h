#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "serverthread.h"







namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void sendData(QByteArray data);
    Ui::MainWindow *ui;


private:
    Worker *pWorker;
    ServerThread *pSeverThread;

signals:
    void writeData(QByteArray data);


public slots:
    void onSetCoilText(QString data);

private slots:
    void on_pushButtonSend_clicked();
    void on_checkBox_clicked();
    void on_checkBoxRead_clicked();

};

#endif // MAINWINDOW_H
