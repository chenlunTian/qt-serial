#ifndef CODETYPE_H
#define CODETYPE_H

#include <QString>
#include <QTextCodec>

//编码格式列表
typedef enum
{
    ASCII = 0,
    Utf8,     //Utf8编码格式
    Utf16,    //Utf16编码格式
    GBK,  //GBK编码格式、兼容GBK18030、GB2312
    Big5,     //Big5
    ShiftJIS
}CodeType;
//设置编码格式
QByteArray SetCodeType(QByteArray const &data,qint32 control);
//解析编码格式
QByteArray GetCodeType(QByteArray const &data, qint32 control);
#endif // CODETYPE_H
