#include "loginwidget.h"
#include <QApplication>
//#include "uipage/chatwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWidget w;
//    ChatWindow w;
    w.show();
    return a.exec();
}
