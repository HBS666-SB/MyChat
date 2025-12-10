#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "basewidget/customwidget.h"

#include <qqcell.h>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(CustomMoveWidget *parent = nullptr);
    ~ChatWindow();

    void setCell(QQCell *cell, const quint8 &type = 0);
    QString getAddr() const;
    int getUserId();

signals:
    void signalSendMessage(const quint8 &type, const QJsonValue &dataVal);
    void signalClose();

private slots:
    void on_btnWinClose_clicked();

    void on_btnSendMsg_clicked();

private:
    Ui::ChatWindow *ui;

    QQCell *m_cell = nullptr;

    quint8 m_nChatType;        // 聊天类型，群组聊天或私人聊天


};

#endif // CHATWINDOW_H
