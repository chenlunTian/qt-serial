#include "mainwidget.h"
#include <QApplication>
#include <QIcon>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;   
    w.setWindowTitle("Qt上位机V0.0.1");
    w.setWindowIcon(QIcon(":/image/image/main.ico"));  //添加图标及窗口标题
    w.show();
    return a.exec();
}
