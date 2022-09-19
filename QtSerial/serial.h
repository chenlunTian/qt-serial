#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "SerialInfo.h"
#include <QTimer>

class Serial : public QObject
{
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial(void);
    void SerialOpen();
    void SerialClose();
    void SendData(QByteArray data,bool flag);
    void RecvData();
    void RecvSerialConfig(Sinfo *data);
    QString InfoSet;

    bool isflag;
private:
    QSerialPort* MySerial;
    Sinfo *info=nullptr;    //串口配置
    QTimer *timer;
    QByteArray buffer;

signals:
    void SetInfo(QString info);
    void isSerialrxData(QByteArray data);
    void isnoSerialOpen();
    void isRecvData(QByteArray);
};

#endif // SERIAL_H
