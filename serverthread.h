#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(QByteArray data);
    void readClient();
    void timeOut();
    void acceptConnection();
};


class ServerThread : public QThread
{
    Q_OBJECT
public:
    explicit ServerThread(QObject *parent = 0);
    void doSetCoilText(QString data);
    void setWorker(Worker *pWorker);
    void startTimer(int msc);
    void stopTimer();

    QByteArray m_data;
    QTimer *pTime;
    QTcpServer *m_pServer;

private:
    Worker *m_pWorker;



protected:
    virtual void run();

signals:
    void setCoilText(QString data);
    void startMyTimer(int msc);
    void stopMyTimer();


public slots:


};

#endif // SERVERTHREAD_H
