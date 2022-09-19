#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "serial.h"
#include <QDebug>
#include <QIcon>
#include <QTextBlock>
#include <QMessageBox>
#include "CodeType.h"
#include <QDateTime>
#include <QProcessEnvironment>
#include <QFileDialog>
MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    //0.初始化ui界面配置qss样式
    initUI();

    //1.创建子线程对象，动态分配空间，指定父对象
    t1 = new QThread(this);
    //2.创建任务类对象,动态分配空间，不能指定父对象
    m_work = new Serial;//防止内存泄漏,关闭窗口时 delete m_work;
    //3.将任务对象移动到某个子线程中
    m_work->moveToThread(t1);
    //4.获取串口设备信息
    getportInfo();
    //添加更新按钮信号与槽，用于更新串口设备信息
    connect(ui->upBtn,&QPushButton::clicked,this,&MainWidget::getportInfo);
    //5.设置串口参数配置数组
    initComboBox();
    //添加串口参数数组的信号与槽，用于更新串口参数配置数组
    isComboBoxSlot();
    //6.设置checkbox参数
    initCheckbox();
    //7.打开线程并运行m_work的SerialOpen()和SerialClose()
    connect(ui->OpenComBtn,&QPushButton::clicked,this,[=]{
        this->SerialOpen();
        t1->start();
    });
    qRegisterMetaType<Sinfo>("Sinfo");
    //添加传递串口参数配置的信号与槽，用于传递串口参数配置
    connect(this,&MainWidget::SendSerialConfig,m_work,&Serial::RecvSerialConfig);
    //添加打开串口的信号与槽，用于打开串口
    connect(this,&MainWidget::isOpenSerial,m_work,&Serial::SerialOpen);
    //添加串口打开失败标志
    connect(m_work,&Serial::isnoSerialOpen,this,[=](){
        message("串口不存在，或被占用!");
        ui->OpenComBtn->setText(tr("打开串口"));
        setboxEnabled(true);
        ui->Info->clear();
    });
    //添加关闭串口的信号与槽，用于释放串口
    connect(this,&MainWidget::isCloseSerial,m_work,&Serial::SerialClose);
    //8.从m_work处传递回来的Serialinfo显示在ui界面上
    connect(m_work,&Serial::SetInfo,this,[=](QString info){
        ui->Info->setText(info);
    });

    //9.获取TxEdit的内容,并通过Tx发送出去
    ui->TxEdit->setAcceptRichText(false);//禁用富文本输入
    connect(ui->TxEdit,&QTextEdit::textChanged,this,[=](){
        if(hextxFlag==true)   //hex模式
        {
            limitHexinput(); //限制输入
        }
        getTxedit(); //获取内容
    });
    connect(ui->TxBtn,&QPushButton::clicked,this,[=](){
        if((hextxFlag==true)&&(txEditnum%3==1))
        {//hex模式
            qDebug() << "txData.length1 " << txData.length();
            message("请在最后1位前补零！");
            return ;
        }
        qDebug() << "txData.length " << txData.length();
        emit isSendData(addEnter(txData),hextxFlag); //发送数据以及hex标志位信号
    });
    //传递数据给m_work,用于串口发送
    connect(this,&MainWidget::isSendData,m_work,&Serial::SendData);
    //切换为hex模式时清空发送区
    connect(this,&MainWidget::ishexModel,this,[=](){
        if(hextxFlag==true){
            ui->AEBox->setDisabled(true);
            ui->AEBox->setCheckState(Qt::Unchecked);
            aeFlag=0;
        }
        else{
            ui->AEBox->setEnabled(true);
            ui->AEBox->setCheckState(Qt::Unchecked);
            aeFlag=0;
        }
        txEditchanged();
        getTxedit();    //更新数据
    });
    //添加自动发送
    sendTimer = new QTimer;
    AutoSendData();
    connect(this,&MainWidget::istimeModel,this,[=]{
        if(trFlag==true){
            sendTimer->setInterval(ui->TVBox->value());
            sendTimer->start();
            ui->TVBox->setDisabled(true);
            ui->TxBtn->setDisabled(true);
        }
        else
        {
            sendTimer->stop();
            ui->TVBox->setEnabled(true);
            ui->TxBtn->setEnabled(true);
        }
    });
    //添加发送清理按钮信号与槽，用于清理发送区数据
    connect(ui->ClearTxBtn,&QPushButton::clicked,this,[=](){
        ui->TxEdit->clear();
    });
    //10.添加接收数据
    connect(m_work,&Serial::isRecvData,this,[=](QByteArray data){
        ui->RxEdit->moveCursor(QTextCursor::End);
        if(hexrxFlag==true){
            ui->RxEdit->insertPlainText(data.toHex(' ').toUpper());
            ui->RxEdit->insertPlainText(" ");
        }
        else{
            if(timeFlag==true){
                //currentDateTime() 可以获取当前系统时间
                QDateTime time = QDateTime::currentDateTime();
                //将时间对面按照一定的格式转换为字符串对象
                QString str = time.toString("[yyyy/MM/dd hh:mm:ss]");
                ui->RxEdit->moveCursor(QTextCursor::End);
                ui->RxEdit->insertPlainText(str);
            }
            if(awFlag==true){
                ui->RxEdit->moveCursor(QTextCursor::End);
                ui->RxEdit->insertPlainText("\n");
            }
            data=GetCodeType(data,info->Encode);
            ui->RxEdit->insertPlainText(data);
        }
    });
    //添加接收清理按钮信号与槽，用于清理接收区数据
    connect(ui->ClearRxBtn,&QPushButton::clicked,this,[=](){
        ui->RxEdit->clear();

    });
    //11.保存数据
    connect(ui->SaveRxBtn,&QPushButton::clicked,this,[=](){
        saveFile(ui->RxEdit->toPlainText());
    });
    qDebug() << "GUI的线程的线程地址: " << QThread::currentThread();
    //当按窗口右上角X时，窗口触发destroyed()，析构子线程。
    connect(this,&MainWidget::destroyed,this,&MainWidget::isStop);
}

MainWidget::~MainWidget()
{
    isStop();
    //    delete m_work;
    delete sendTimer;
    delete aIntValidator;
    delete info;
    delete [] comInfo;
    delete ui;
}

void MainWidget::initComboBox()
{
    //限定波特率下拉框只能输入数字，最高为1M
    this->aIntValidator = new QIntValidator;//防止内存泄漏,关闭窗口时 delete aIntValidator;
    this->aIntValidator->setRange(0, 1000000);
    ui->comBaudRate->setValidator(aIntValidator);
    if(info!=nullptr)   //删除原先内存空间
    {
        delete info;
    }
    this->info = new Sinfo;//防止内存泄漏,关闭窗口时 delete info;
    this->info->comName=ui->comPortName->currentText();  //设置串口号
    this->info->baudRate=ui->comBaudRate->currentText().toInt();  //设置波特率
    this->info->dataBits=ui->comDataBits->currentIndex();  //设置数据位
    this->info->parity=ui->comParity->currentIndex();      //设置检验位
    this->info->stopBits=ui->comStopBits->currentIndex();  //设置停止位
    this->info->flowControl=0 ;                           //设置流控位，默认值为0无流控
    this->info->Encode=ui->comEncode->currentIndex();      //设置编码格式

}

void MainWidget::setboxDisabled(bool flag)
{
    ui->comPortName->setDisabled(flag);
    ui->comBaudRate->setDisabled(flag);
    ui->comDataBits->setDisabled(flag);
    ui->comParity->setDisabled(flag);
    ui->comStopBits->setDisabled(flag);
    ui->comEncode->setDisabled(flag);
    ui->upBtn->setDisabled(flag);
    ui->SaveRxBtn->setDisabled(flag);
}

void MainWidget::setboxEnabled(bool flag)
{
    ui->comPortName->setEnabled(flag);
    ui->comBaudRate->setEnabled(flag);
    ui->comDataBits->setEnabled(flag);
    ui->comParity->setEnabled(flag);
    ui->comStopBits->setEnabled(flag);
    ui->comEncode->setEnabled(flag);
    ui->upBtn->setEnabled(flag);
    ui->SaveRxBtn->setEnabled(flag);
}

void MainWidget::initCheckbox()
{
    //时间戳标志位
    connect(ui->TimeBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->TimeBox->isChecked())
            timeFlag=true;
        else
            timeFlag=false;
    });
    //16进制接收标志位
    connect(ui->HexRxBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->HexRxBox->isChecked())
        {
            hexrxFlag =true;
            ui->TimeBox->setDisabled(true);
            ui->TimeBox->setCheckState(Qt::Unchecked);
            timeFlag=false;
            ui->AWBox->setDisabled(true);
            ui->AWBox->setCheckState(Qt::Unchecked);
            awFlag=false;
        }
        else
        {
            hexrxFlag =false;
            ui->TimeBox->setEnabled(true);
            ui->AWBox->setEnabled(true);
        }
    });
    //16进制发送标志位
    connect(ui->HexTxBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->HexTxBox->isChecked())
            hextxFlag =true;
        else
            hextxFlag =false;
        emit ishexModel();
    });
    //自动换行标志位
    connect(ui->AWBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->AWBox->isChecked())
            awFlag =true;
        else
            awFlag =false;
    });
    //定时发送标志位
    connect(ui->TRBox,&QCheckBox::stateChanged,this,[=](){
        if(ui->TRBox->isChecked())
            trFlag =true;
        else
            trFlag =false;

        emit istimeModel();
    });
    //加入换行符标志位
    connect(ui->AEBox,&QCheckBox::stateChanged,this,[=]{
        if(ui->AEBox->isChecked())
            aeFlag =true;
        else
            aeFlag =false;
    });
}

void MainWidget::isStop()
{
    t1->quit();
    t1->wait();
}

void MainWidget::initUI()
{
    QFile file(":/qss/qss/QMessageBox.qss");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream filetext(&file);
        QString stylesheet = filetext.readAll();
        qApp->setStyleSheet(stylesheet);
    }
    //设置刷新按钮样式
    ui->upBtn->setStyleSheet("QPushButton{border-image: url(:/image/image/updata.ico);}"
                             "QPushButton:pressed{border-image: url(:/image/image/updata1.ico);}");
    ui->upBtn->setFocusPolicy(Qt::NoFocus);

}

void MainWidget::getportInfo()
{
    qint32 comCnt=0;
    //要删除原有的内存空间
    if(comInfo!=nullptr)
    {
        delete [] comInfo;
    }
    if(portStringList.length()!=0)
    {
        portStringList.clear();
        qDebug()<<"portStringList is clear";
    }
    if(ui->comPortName->count()!=0)
    {
        ui->comPortName->clear();
        qDebug()<<"ui->comPortName is clear";
    }
    //获取串口设备数量
    comCnt = QSerialPortInfo::availablePorts().length();
    if(comCnt!=0)
    {
        comInfo = new ScomInfo[comCnt];//动态分配空间,防止内存泄漏,关闭窗口时 delete [] comInfo;
        //获取串口信息
        portNum=0;
        foreach (const QSerialPortInfo &qspinfo, QSerialPortInfo::availablePorts())
        {
            //                portStringList.clear();
            this->comInfo[portNum].SerialName=qspinfo.portName();
            //                this->comInfo[portNum].ProductCode=qspinfo.productIdentifier();
            //                this->comInfo[portNum].SystemPosition=qspinfo.systemLocation();
            //                this->comInfo[portNum].SerialNumStr=qspinfo.serialNumber();
            this->comInfo[portNum].DescribeStr=qspinfo.description();
            //                this->comInfo[portNum].Manufacturer=qspinfo.manufacturer();
            //                this->comInfo[portNum].SupplierCode=qspinfo.vendorIdentifier();
            portStringList+=qspinfo.portName();
            portNum++;
        }
        ui->comPortName->addItems(portStringList);
    }
    else{
        message("未检测到串口!");
    }
}

void MainWidget::isComboBoxSlot()
{
    connect(ui->comPortName, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this,[=](const QString &text){
        /* ... */
        this->info->comName=text;  //设置串口号
        qDebug()<<"comName"<<text;
    });
    connect(ui->comBaudRate, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this,[=](const QString &text){
        /* ... */
        this->info->baudRate=text.toInt();  //设置串口波特率
        qDebug()<<"baudRate"<<text;
    });
    connect(ui->comDataBits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,[=](int index){
        /* ... */
        this->info->dataBits=index;  //设置串口数据位
        qDebug()<<"dataBits"<<index;
    });
    connect(ui->comParity, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,[=](int index){
        /* ... */
        this->info->parity=index;  //设置串口校验位
        qDebug()<<"parity"<<index;
    });
    connect(ui->comStopBits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,[=](int index){
        /* ... */
        this->info->stopBits=index;  //设置串口停止位
        qDebug()<<"stopBits"<<index;
    });
    connect(ui->comEncode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,[=](int index){
        /* ... */
        this->info->Encode=index;  //设置串口编码格式
        qDebug()<<"Encode"<<index;
    });
    /**预留流控控制**/
    //    connect(ui->comFlowControl, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //        this,[=](int index){
    //        /* ... */
    //        this->info->flowControl=index;  //设置串口流控位
    //        qDebug()<<"Encode"<<index;
    //    });
}
//打开串口设备
void MainWidget::SerialOpen(void)
{
    //设置串口打开按钮文本样式，并失/使能box
    if(ui->OpenComBtn->text() == tr("打开串口")){
        ui->OpenComBtn->setText(tr("关闭串口"));
        setboxDisabled(true);
        qDebug()<<"btn is true";
        //读取选择的串口配置
        emit SendSerialConfig(info);
        emit isOpenSerial();
    }
    else{
        ui->OpenComBtn->setText(tr("打开串口"));
        if(ui->TRBox->isChecked())
        {
            ui->TRBox->setCheckState(Qt::Unchecked);
            trFlag=false;
        }
        setboxEnabled(true);
        this->SerialClose();
        ui->Info->clear();
    }
}
//关闭串口设备
void MainWidget::SerialClose(void)
{
    ui->OpenComBtn->setChecked(false);
    qDebug()<<"btn is false";
    emit isCloseSerial();
}

void MainWidget::getTxedit()
{
    txEditnum=ui->TxEdit->toPlainText().toUtf8().length();
    qDebug() <<"ui->TxEdit->toPlainText().length()"<<ui->TxEdit->toPlainText().toUtf8().length();
    if(hextxFlag==true)
    {
        this->txData=QByteArray::fromHex(ui->TxEdit->toPlainText().toUtf8());
    }
    else
        this->txData=ui->TxEdit->toPlainText().toUtf8();   //更新tx数据
}

void MainWidget::limitHexinput()
{
    //获取光标前面的一个字符，判断是否符合16进制的输入格式要求
    int CurLineNum=ui->TxEdit->textCursor().blockNumber(); //光标所在行号，序号从0开始
    int mPosInBlock=ui->TxEdit->textCursor().positionInBlock();  //光标所在行的位置
    static int lastPosInBlock=0;      //上一次最后空格的位置
    if(ui->TxEdit->toPlainText().length()==0){
        lastPosInBlock=0;  //内容为空时清零空格位置
        qDebug()<<"mPosInBlock"<<mPosInBlock<<"lastPosInBlock"<<lastPosInBlock;
    }
    //当前行文本
    QString CurLineText=ui->TxEdit->document()->findBlockByLineNumber(CurLineNum).text();
    char InputChar=0;
    if(mPosInBlock!=0) //如果光标所在行的位置为0，InputChar为0，不用获取
    {
        InputChar=CurLineText.at(mPosInBlock-1).toLatin1();//获取光标前字符
    }
    if(InputChar==' '
            ||(InputChar>='0'&&InputChar<='9')
            ||(InputChar>='a'&&InputChar<='f')
            ||(InputChar>='A'&&InputChar<='F'))
    {//限制输入为 空格 和 16进制数
        if((mPosInBlock>lastPosInBlock)&&(mPosInBlock>=2)){
            char InputChar1=0;
            InputChar1=CurLineText.at(mPosInBlock-2).toLatin1();//获取光标前2字符
            if(InputChar1!=' ')
            {
                ui->TxEdit->blockSignals(true);     //阻塞TxEdit的信号(主要是textChanged())
                ui->TxEdit->insertPlainText(" ");   //每两个字符自动加入空格(会触发textChanged()需阻塞)
                ui->TxEdit->blockSignals(false);    //取消阻塞的信号
                ui->TxEdit->setTextCursor(ui->TxEdit->textCursor()); //重设光标位置
                lastPosInBlock=ui->TxEdit->textCursor().positionInBlock();  //更新空格的位置
                qDebug() <<"lastPosInBlock"<<lastPosInBlock;
            }
        }
        if(InputChar==' ')
        {   //如果输入为空格，更新空格的位置
            lastPosInBlock=mPosInBlock;
            qDebug()<<"mPosInBlock"<<mPosInBlock<<"lastPosInBlock"<<lastPosInBlock;
            return ;
        }
        else{
            if(mPosInBlock-lastPosInBlock < 2)
            {   //光标位置如果距离上次的空格小于2 直接输入否则自动加入空格
                qDebug()<<"mPosInBlock"<<mPosInBlock<<"lastPosInBlock"<<lastPosInBlock;
                return;
            }
            else{
                ui->TxEdit->blockSignals(true);     //阻塞TxEdit的信号(主要是textChanged())
                ui->TxEdit->insertPlainText(" ");   //每两个字符自动加入空格(会触发textChanged()需阻塞)
                ui->TxEdit->blockSignals(false);    //取消阻塞的信号
                ui->TxEdit->setTextCursor(ui->TxEdit->textCursor()); //重设光标位置
                lastPosInBlock=ui->TxEdit->textCursor().positionInBlock();  //更新空格的位置
                qDebug() <<"lastPosInBlock"<<lastPosInBlock;
            }
        }
    }
    else
    {
        ui->TxEdit->textCursor().deletePreviousChar();
        ui->TxEdit->setTextCursor(ui->TxEdit->textCursor());
    }
}

void MainWidget::txEditchanged()
{
    if(hextxFlag==true) //说明txdata之前存储的是非16进制数
    {                   //此时要转为16进制数
        txData=SetCodeType(txData,info->Encode); //先根据编码转换数据编码格式
        ui->TxEdit->blockSignals(true);     //阻塞TxEdit的信号(主要是textChanged())
        ui->TxEdit->setPlainText(txData.toHex(' ').toUpper());  //转换后的在TxEdit中显示
        ui->TxEdit->blockSignals(false);    //取消阻塞的信号

    }
    else //16进制根据编码转换为文本
    {
        txData=GetCodeType(txData,info->Encode);   //转换成unicode
        ui->TxEdit->blockSignals(true);     //阻塞TxEdit的信号(主要是textChanged())
        ui->TxEdit->setPlainText(txData);
        ui->TxEdit->blockSignals(false);    //取消阻塞的信号
    }

}

void MainWidget::AutoSendData()
{
    connect(sendTimer,&QTimer::timeout,this,[=]{
        if((hextxFlag==true)&&(txEditnum%3==1))
        {//hex模式
            qDebug() << "txData.length1 " << txData.length();
            message("请在最后1位前补零！");
            ui->TRBox->setCheckState(Qt::Unchecked);
            sendTimer->stop();
            return ;
        }
        if(ui->OpenComBtn->text() == tr("关闭串口"))
        {
            emit isSendData(addEnter(txData),hextxFlag); //发送数据以及hex标志位信号
        }
    });
}

QByteArray MainWidget::addEnter(QByteArray Data)
{
    if(aeFlag==true)
    {
        if(hextxFlag==false)
        {
            QString str = Data;
            qDebug()<<"str"<<str;
            Data=str.append("\n").toUtf8();
            Data=SetCodeType(Data,info->Encode); //根据编码转换数据编码格式
        }
    }
    qDebug()<<"Data"<<Data;
    return Data;
}

void MainWidget::message(const char *str)
{
    QMessageBox msgBox;
    msgBox.setText(tr(str));
    msgBox.setWindowTitle(tr("Qt上位机V0.0.1"));
    msgBox.setWindowIcon(QIcon(":/image/image/main.ico"));
    msgBox.exec();
}

void MainWidget::saveFile(const QString &s)
{

    QString _document = QProcessEnvironment::systemEnvironment().value("USERPROFILE");
    QString dirpath =
            QFileDialog::getSaveFileName(this, tr("Qt上位机V0.0.1"),
                                         _document,
                                         QString(tr("File (*.txt)")),
                                         Q_NULLPTR,
                                         QFileDialog::ShowDirsOnly |
                                         QFileDialog::DontResolveSymlinks);
    if(dirpath!=NULL)
    {
        QFile file(dirpath);
        //方式：Append为追加，WriteOnly，ReadOnly
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::critical(NULL,
                                  tr("提示"),
                                  tr("无法创建文件"));
            return ;
        }
        QTextStream out(&file);
        out<<s;
    }
}

