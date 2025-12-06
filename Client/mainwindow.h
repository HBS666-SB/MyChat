#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "basewidget/customwidget.h"
#include "netdb/mysocket.h"

#include <QSystemTrayIcon>
class QButtonGroup;
class QQCell;
namespace Ui {
class MainWindow;
}

class MainWindow : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit MainWindow(MySocket *socket, QWidget *parent = nullptr);
    ~MainWindow();

    void addFriend(const QJsonValue &dataVal);
    void addFriendRequist(const QJsonValue &dataVal);

private slots:
    void sltButtonClicked(int index);
    void SltTrayIcoClicked(QSystemTrayIcon::ActivationReason reason);   //托盘图标
    void SltTrayIconMenuClicked(QAction* action);   //托盘图标菜单
    void SltQuitApp();  //退出应用

    void SltSysmenuCliecked(QAction* action);  //实现系统菜单功能

    void setHead(const QString &headFile);  //设置头像
    void onAddFriendMenuDidSelected(QAction* action);
    void SltFriendsClicked(QQCell *action);
    void onChildPopMenuDidSelected(QAction* action);

    void sltStatus(const quint8 &status,const QJsonValue &dataVal);
private:
    // 添加系统菜单处理
    void InitSysMenu();
    void InitQQListMenu();
    void InitSysTrayIcon();

    Ui::MainWindow *ui;

    QButtonGroup *m_buttonGroup;
    MySocket *m_tcpSocket;

    QSystemTrayIcon *systemTrayIcon;
    bool m_bQuit;   // 主动退出操作时不进行断线匹配

};

#endif // MAINWINDOW_H
