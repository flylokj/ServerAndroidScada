#include "serverthread.h"
#include <Protocol.h>
#include "mainwindow.h"



QTcpSocket *clientConnection;
extern MainWindow *gMainWindow;
ServerThread *gServerThread;
static int cnt;
int id;


uint16 writeFunc( quint8* data, uint16 sz )
{
    QByteArray sendData((const char *)data, sz);
    gMainWindow->sendData(sendData);
}




int API_WriteData(mbyte coil, uint16 addr, uint16 dataGet)
{

    QString data("D ");
    data=data+QString::number(addr)+" = "+QString::number(dataGet);
    qDebug()<<data;
    gServerThread->doSetCoilText(data);
    if(coil == dcoilD && addr>=MONITOR_ADDR)
    {
        if(addr==MONITOR_ADDR)
        {
            cnt = 0;
        }
        cnt++;
    }
    else if(coil == dcoilD && addr==MONITORID_ADDR)//监控id;
    {
        id = dataGet;
        qDebug()<<"----------------------------get id "<<id;
    }

    return 1;
}



int API_WriteBit(mbyte coil, uint16 addr, uint16 dataGet)
{
    static int num = 0;
    if(coil == dcoilM)
    {
        qDebug()<<coil<<" "<<addr<<" "<<dataGet;
        if(addr == MONITOR_ADDR && dataGet == 1)//M8040=1,返回监控信息;
        {
            QByteArray data("");
            data.append(CMD_WRITE);
            data.append(dcoilD);
            data.append((MONITOR_ADDR>>8)&0xff);
            data.append(MONITOR_ADDR&0xff);
            data.append((char)(cnt/2));
            for(int i = 0; i < cnt/2; i++)
            {
                num++;
                data.append((num>>8)&0xff);
                data.append(num&0xff);
            }
            for(int i = 0; i < data.size(); i++)
            {
                quint8 temp = data.at(i);
                qDebug()<<"monitor: "+QString::number(temp);
            }

            gServerThread->m_data = data;
            gServerThread->startTimer(10);
        }
    }
    return 1;
}








ServerThread::ServerThread(QObject *parent) :
    QThread(parent),
    m_pServer(NULL),
    m_pWorker(NULL)
{
    gServerThread = this;
}

void ServerThread::doSetCoilText(QString data)
{
    emit(setCoilText(data));
}

void ServerThread::setWorker(Worker *pWorker)
{
    pWorker->moveToThread(this);
    m_pWorker = pWorker;
}

void ServerThread::startTimer(int msc)
{
    emit(startMyTimer(msc));
}

void ServerThread::stopTimer()
{
    emit(stopMyTimer());
}

void ServerThread::run()
{
    m_pServer = new QTcpServer();
    m_pServer->listen(QHostAddress::Any, 6665);
    connect(m_pServer, SIGNAL(newConnection()), m_pWorker, SLOT(acceptConnection()) );
    //注册通讯函数;
    ProtocolConf proConf;
    proConf.write = writeFunc;
    SetProtocolConf( &proConf, PROTOCOL_CONF_WRITEFUN );
    //定时;
    pTime = new QTimer();
    connect(pTime, SIGNAL(timeout()), m_pWorker, SLOT(timeOut()) );
    connect(this, SIGNAL(startMyTimer(int)), pTime, SLOT(start(int)) );
    connect(this, SIGNAL(stopMyTimer()), pTime, SLOT(stop()) );
    qDebug()<<"worker thread "<<QThread::currentThreadId();
    exec();

}
int diaoni = 0;
int diaoni2 = 0;
int diaoni3 = 0;
int diaoni4 = 0;
void Worker::readClient()
{
    qDebug()<<"readclient id "<<QThread::currentThreadId();
    QByteArray data  = clientConnection->readAll();
    //解析协议;
    API_Protocol((mbyte *)data.data(), data.size());//解析协议;

//    qDebug()<<"recv:";
//    mbyte *d = (mbyte *)data.data();
//    for(int i = 0; i < data.size(); i++)
//        qDebug()<<d[i];
//    qDebug()<<"diaoni="<<diaoni<<"  "<<"diaoni2="<<diaoni2<<"  diaoni3="<<diaoni3<<"  diaoni4="<<diaoni4;
//    diaoni = 0;
//    diaoni2 = -1;
//    diaoni3 = -1;
//    diaoni4 = -1;
    //显示数据;
    //ui->recvLabel->setText(str);
}

void Worker::acceptConnection()
{
    //ui->recvLabel->setText("have connected");

    gServerThread->doSetCoilText("have connected");
    clientConnection = gServerThread->m_pServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readClient()) );

}

void Worker::timeOut()
{
    qDebug()<<"time out thread "<<QThread::currentThreadId();
    QByteArray tid;
    tid.append((id>>8)&0xff);
    tid.append(id&0xff);
    gServerThread->m_data.replace(5, 2, tid);
    API_ProtocolSend((mbyte *)gServerThread->m_data.data(), gServerThread->m_data.size());
    qDebug()<<"send id:"<<id;
    gServerThread->stopTimer();
}



void Worker::doWork(QByteArray data){
//    qDebug()<<"===========================worker write==============";
//    qDebug()<<"send:";
//    for(int i = 0; i < data.size(); i++)
//        qDebug()<<(quint8)data[i];
    clientConnection->write((const char*)data.data(), data.size());
//    qDebug()<<"===========================worker write done!!!==============";
}
