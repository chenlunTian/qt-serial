#include "CodeType.h"
#include <qdebug.h>
//编码
QByteArray SetCodeType(const QByteArray &data, qint32 control)
{
    QByteArray tmpData;
    switch (control) {
        case ASCII: tmpData=QTextCodec::codecForName("latin1")->fromUnicode(data);break;
        case Utf8: tmpData= QTextCodec::codecForName("UTF-8")->fromUnicode(data);break;
        case Utf16: tmpData= QTextCodec::codecForName("UTF-16")->fromUnicode(data);break;
        case GBK: tmpData= QTextCodec::codecForName("GBK")->fromUnicode(data);break;
        case Big5: tmpData= QTextCodec::codecForName("Big5")->fromUnicode(data);break;
        case ShiftJIS: tmpData= QTextCodec::codecForName("Shift-JIS")->fromUnicode(data);break;
        default:;break;
    }
    return tmpData;
}

//解码
QByteArray GetCodeType(const QByteArray &data, qint32 control)
{
    QString tmpData;
    switch (control) {
        case ASCII: tmpData= QTextCodec::codecForName("latin1")->toUnicode(data);break;
        case Utf8: tmpData= QTextCodec::codecForName("UTF-8")->toUnicode(data);break;
        case Utf16: tmpData= QTextCodec::codecForName("UTF-16")->toUnicode(data);break;
        case GBK: tmpData= QTextCodec::codecForName("GBK")->toUnicode(data);break;
        case Big5: tmpData= QTextCodec::codecForName("Big5")->toUnicode(data);break;
        case ShiftJIS: tmpData= QTextCodec::codecForName("Shift-JIS")->toUnicode(data);break;
        default:;break;
    }
    return tmpData.toUtf8();
}
