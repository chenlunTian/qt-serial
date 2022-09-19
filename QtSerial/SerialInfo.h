#ifndef SERIALINFO_H
#define SERIALINFO_H
#include <QVector>
#include <QMetaType>

//获取串口设备信息列表，以便于识别正确设备
typedef struct SerialComInfos
{                           //描述:        eg:
    QString SerialName;     //名称:        "COMx"
    qint16  ProductCode;    //产品编号:     "29987"
    QString SystemPosition;//系统位置:     "\\\\.\\COM10"
    QString SerialNumStr;   //序列号字符串: ""
    QString DescribeStr;    //描述字符串:   "USB-SERIAL CH340"
    QString Manufacturer;   //制造商:      "wch.cn"
    QString SupplierCode;   //供应商编码:   "6790"
}ScomInfo;

typedef struct SerialInfos         //串口配置信息
{
     QString comName;    //串口名称
     qint32 baudRate;     //波特率
     qint32 dataBits;     //数据位
     qint32 parity;       //校验位
     qint32 stopBits;     //停止位
     qint32 flowControl;  //流控位
     qint32 Encode;       //编码格式

}Sinfo;


//通过Q_DECLARE_METATYPE声明后，就可以让自定义的类型设置到QVariant。
Q_DECLARE_METATYPE(Sinfo);
Q_DECLARE_METATYPE(ScomInfo);

#endif // SERIALINFO_H
