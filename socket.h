#ifndef SOCKET_H
#define SOCKET_H
#include<QtNetwork>
#include<QTcpSocket>

class Socket:public QObject
{
    Q_OBJECT
public:
    Socket();
    virtual void setSocketFlag(const uint);
    virtual void setSocket(QTcpSocket*);
    virtual const uint getSocketFlag();
    virtual bool readMsg(QString& msg);
    virtual bool sendMsg(const QString msg);
private:
    uint socketDescriptor;
    QTcpSocket* socket;
signals:
    void disconnectServer(uint);
    void readyToRead(uint);
public slots:
    virtual void receiveMsg();
    virtual void disconnectFromServer();
};

#endif // SOCKET_H
