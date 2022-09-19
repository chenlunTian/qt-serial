#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "SerialInfo.h"
#include "serial.h"
#include <QThread>
#include <QByteArray>
#include <QStringList>
#include <QIntValidator>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE


class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    Sinfo *info=nullptr;
    ScomInfo *comInfo=nullptr;
    QStringList portStringList; //保存当前可用的串口号
    QIntValidator* aIntValidator; //限定波特率下拉框只能输入数字，最高为1M
    QByteArray txData=nullptr;
    QByteArray rxData=nullptr;
    //1.创建子线程对象
    QThread *t1;
    //2.创建任务类对象
    Serial *m_work;
    void initComboBox();
    void setboxDisabled(bool);
    void setboxEnabled(bool);
    void initCheckbox();
    void isStop();
    void initUI();
    void getportInfo();
    void isComboBoxSlot();
    void SerialOpen();
    void SerialClose();
    void getTxedit();
    void limitHexinput();
    void txEditchanged();
    void AutoSendData();
    QByteArray addEnter(QByteArray);
private:
    Ui::MainWidget *ui;
    QTimer *sendTimer;
    qint32 portNum=0;
    qint32 txEditnum=0;
    bool timeFlag=false; //时间戳标志位
    bool hexrxFlag=false; //16进制接收标志位
    bool hextxFlag=false; //16进制发送标志位
    bool aeFlag=false; //加入换行符标志位
    bool awFlag=false; //自动换行标志位
    bool trFlag=false; //定时发送标志位
    QByteArray TxBuff=nullptr; //TX缓冲区
    void message(const char *str);
    void saveFile(const QString &s);

signals:
    void SendSerialConfig(Sinfo *info);
    void checkBoxChanged(int,bool);
    void isportInfo();
    void ishexModel();
    void istimeModel();
    void isCloseSerial();
    void isOpenSerial();
    void isSendData(QByteArray,bool);

};
#endif // MAINWIDGET_H
