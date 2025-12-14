#ifndef FACEDIALOG_H
#define FACEDIALOG_H

#include "emojiitem.h"
#include "basewidget/customwidget.h"

namespace Ui {
class faceDialog;
}

class faceDialog : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit faceDialog(QWidget *parent = nullptr);
    ~faceDialog();
    int getSelectFaceIndex();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_pushButton_clicked();

signals:
    void signalSelectFaceIndex(int index);

private:
    Ui::faceDialog *ui;

    QList<EmojiItem*> m_emojiList;
    void addEmojiItem(QString fileName);
public slots:
    void sltClose();


public:
    int m_selectFaceIndex;

};

#endif // FACEDIALOG_H
