#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <Protocol.h>

MainWindow *gMainWindow;
extern int bAutoAck;
extern int bAutoRead;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug()<<"main:"<<QThread::currentThreadId();
    pSeverThread = new ServerThread();
    pWorker = new Worker;
    pSeverThread->setWorker(pWorker);
    connect(pSeverThread, SIGNAL(setCoilText(QString)), this, SLOT(onSetCoilText(QString)) );
    connect(this, SIGNAL(writeData(QByteArray)), pWorker, SLOT(doWork(QByteArray)) );
    pSeverThread->start();

    gMainWindow = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::sendData(QByteArray data)
{
    emit(writeData(data));
}

void MainWindow::onSetCoilText(QString data)
{
    ui->recvLabel->setText(data);
}


void MainWindow::on_checkBox_clicked()
{
    bAutoAck = ui->checkBox->isChecked();
}

void MainWindow::on_checkBoxRead_clicked()
{
    bAutoRead = ui->checkBoxRead->isChecked();
}

//·¢ËÍÊý¾Ý;
void MainWindow::on_pushButtonSend_clicked()
{
//    if(ui->lineEditSend->text().isEmpty())
//    {
//        QMessageBox::warning(this, "warning", "conten empty");
//        return;
//    }

//    if(!clientConnection->isValid())
//    {
//        QMessageBox::warning(this, "error", "network not work");
//        return;
//    }


//    QByteArray data("");
//    data.append(0x90);//X
//    data.append(1);

//    protocolSend(data, formatFunc);
//    QMessageBox::information(0, "info", "success");
//    //clientConnection->write();
}


