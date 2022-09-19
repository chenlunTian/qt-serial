#include "serial.h"
#include <QDebug>
#include <QThread>
#include "CodeType.h"

Serial::Serial(QObject *parent)
    : QObject(parent)
    ,timer(new QTimer(this)) //创建timer对象,动态分配空间，指定父对象
    ,buffer(0)  //初始化buffer
{
    //动态分配空间，指定父对象
    MySerial = new QSerialPort(this);


    QObject::connect(MySerial,&QSerialPort::readyRead,this,[=]{
        timer->start(100);//设置100毫秒的延时
        buffer.append(MySerial->readAll());//将读到的数据放入缓冲区
    });
    connect(timer,&QTimer::timeout,this,&Serial::RecvData);//timeout执行真正的读取操作

}

Serial::~Serial()
{
    delete info;
}

void Serial::SerialOpen()
{
    qDebug() << "SerialOpen" << QThread::currentThread();
    this->SerialClose();
    MySerial->setPortName(QString(info->comName));
    if(!MySerial->open(QIODevice::ReadWrite))//用ReadWrite 的模式尝试打开串口
    {
        emit isnoSerialOpen();  //发送打开失败的标志
        qDebug()<<QString(info->comName)<<"打开失败!";
        return;
    }
    InfoSet.append(tr("串口:"));
    InfoSet.append(QString(info->comName));
    qDebug()<<InfoSet<<"sussced!";
    bool Bflag = MySerial->setBaudRate(qint32(info->baudRate));
    if(Bflag){
        InfoSet.append(tr(" 波特率:"));
        //第一个参数为int变量，第二个参数10表示转换为10进制数
        QString baudRateinfo = QString::number(int(info->baudRate),10);
        InfoSet.append(baudRateinfo);
    }
    else{
        InfoSet.append(tr("波特率:Unknown"));
    };
    //设置数据位
    switch (info->dataBits) {
    case 0:
        MySerial->setDataBits(QSerialPort::Data5);
        //                qDebug() <<"dataBitsSet:Data5";
        InfoSet.append(tr(" 数据位:5"));
        break;
    case 1:
        MySerial->setDataBits(QSerialPort::Data6);
        //                qDebug() <<"dataBitsSet:Data6";
        InfoSet.append(tr(" 数据位:6"));
        break;
    case 2:
        MySerial->setDataBits(QSerialPort::Data7);
        //                qDebug() <<"dataBitsSet:Data7";
        InfoSet.append(tr(" 数据位:7"));
        break;
    case 3:
        MySerial->setDataBits(QSerialPort::Data8);
        //                qDebug() <<"dataBitsSet:Data8";
        InfoSet.append(tr(" 数据位:8"));
        break;
    default:
        MySerial->setDataBits(QSerialPort::UnknownDataBits);
        //                qDebug() <<"dataBitsSet:UnknownDataBits";
        InfoSet.append(tr(" 数据位:Unknown"));
        break;
    }
    //设置校验位
    switch (info->parity) {
    case 0:
        MySerial->setParity(QSerialPort::EvenParity);
        //                qDebug() <<"paritySet:EvenParity";
        InfoSet.append(tr(" 校验位:Even"));
        break;
    case 1:
        MySerial->setParity(QSerialPort::MarkParity);
        //                qDebug() <<"paritySet:MarkParity";
        InfoSet.append(tr(" 校验位:Mark"));
        break;
    case 2:
        MySerial->setParity(QSerialPort::NoParity);
        //                qDebug() <<"paritySet:NoParity";
        InfoSet.append(tr(" 校验位:None"));
        break;
    case 3:
        MySerial->setParity(QSerialPort::OddParity);
        //                qDebug() <<"paritySet:OddParity";
        InfoSet.append(tr(" 校验位:Odd"));
        break;
    case 4:
        MySerial->setParity(QSerialPort::SpaceParity);
        //                qDebug() <<"paritySet:SpaceParity";
        InfoSet.append(tr(" 校验位:Space"));
        break;
    default:
        MySerial->setParity(QSerialPort::UnknownParity);
        //                qDebug() <<"paritySet:UnknownParity";
        InfoSet.append(tr(" 校验位:Unknown"));
        break;
    }
    //设置停止位
    switch (info->stopBits) {
    case 0:
        MySerial->setStopBits(QSerialPort::OneStop);
        //                qDebug() <<"stopBitsSet:1";
        InfoSet.append(tr(" 停止位:1"));
        break;
    case 1:
        MySerial->setStopBits(QSerialPort::OneAndHalfStop);
        //                qDebug() <<"stopBitsSet:1.5";
        InfoSet.append(tr(" 停止位:1.5"));
        break;
    case 2:
        MySerial->setStopBits(QSerialPort::TwoStop);
        //                qDebug() <<"stopBitsSet:2";
        InfoSet.append(tr(" 停止位:2"));
        break;
    default:
        MySerial->setStopBits(QSerialPort::UnknownStopBits);
        //                qDebug() <<"stopBitsSet:UnknownParity";
        InfoSet.append(tr(" 停止位:Unknown"));
        break;
    }
    //设置流控位
    switch (info->flowControl) {
    case 0:
        MySerial->setFlowControl(QSerialPort::NoFlowControl);
        InfoSet.append(tr(" 流控位:None"));
        break;
    case 1:
        MySerial->setFlowControl(QSerialPort::HardwareControl);
        InfoSet.append(tr(" 流控位:Hardware"));
        break;
    case 2:
        MySerial->setFlowControl(QSerialPort::SoftwareControl);
        InfoSet.append(tr(" 流控位:Software"));
        break;
    default:
        MySerial->setFlowControl(QSerialPort::UnknownFlowControl);
        //                qDebug() <<"stopBitsSet:UnknownParity";
        InfoSet.append(tr(" 流控位:Unknown"));
        break;
    }
    emit SetInfo(InfoSet);
    qDebug() <<"InfoSet"<<InfoSet;
    //        QObject::connect(MySerial,&QSerialPort::readyRead,this,&Serial::RecvData);
}

void Serial::SerialClose()
{
    qDebug() << "SerialClose" << QThread::currentThread();
    if(MySerial->isOpen())//如果串口已经打开了 先给他关闭了
    {
        MySerial->clear();
        MySerial->close();
    }
}

void Serial::SendData(QByteArray data, bool flag)
{
    if(data.isEmpty())
    {
        return;
    }
    if(flag==true)
    {   //hex模式直接发送
        MySerial->write(data);
    }
    else{ //判断编码格式在发送
        data=SetCodeType(data,info->Encode); //先根据编码转换数据编码格式
        MySerial->write(data);
    }
}


void Serial::RecvData()
{
    timer->stop();//关闭定时器
    QByteArray info = buffer;//读取缓冲区数
    buffer.clear();//清除缓冲区
    if(info.isEmpty())
    {//没有读取到串口数据就退出循环
        return ;
    }
    emit isRecvData(info);
    qDebug() <<"info"<<info;
}

void Serial::RecvSerialConfig(Sinfo *data)
{
    if(info!=nullptr)   //删除原先内存空间
    {
        delete info;
    }
    this->info = new Sinfo;//防止内存泄漏,关闭窗口时 delete info;
    //接收参数设置
    this->info->Encode=data->Encode;
    this->info->baudRate=data->baudRate;
    this->info->comName=data->comName;
    this->info->dataBits=data->dataBits;
    this->info->flowControl=data->flowControl;
    this->info->parity=data->parity;
    this->info->stopBits=data->stopBits;
}


