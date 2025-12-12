#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "basewidget/customwidget.h"
#include <QStandardItemModel>

#include <QTime>
#include <qqcell.h>

#include <basewidget/chatbubble.h>

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
//    QString getAddr() const;
    int getUserId();
    QString getUserName();
    QString getUserHead();
    void AddMessage(const QJsonValue &jsonVal);


signals:
    void signalSendMessage(const quint8 &type, const QJsonValue &dataVal);
    void signalClose();

protected:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
//    QString GetHeadPixmap(const QString &name) const;

private slots:
    // 文件
//    void SltFileRecvFinished(const quint8 &type, const QString &filePath);
//    void SltUpdateProgress(quint64 bytes, quint64 total);
//    void SltDownloadFiles(const QString &fileName);

    void on_btnWinClose_clicked();

    void on_btnSendMsg_clicked();


    void on_btnSendFile_clicked();
    void on_btnClose_clicked();


private:
    Ui::ChatWindow *ui;

    QQCell *m_cell = nullptr;
    QStandardItemModel *m_model = nullptr;

//    ClientFileSocket *m_tcpFileSocket; //文件传输
//    QString m_strFileName;
//    QTime m_updateTime;

//    quint8 m_nFileType;
    quint8 m_nChatType;        // 聊天类型，群组聊天或私人聊天
     bool m_isEnterSend = true;

};

#endif // CHATWINDOW_H
