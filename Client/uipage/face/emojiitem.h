#ifndef EMJOIITEM_H
#define EMJOIITEM_H

#include <QLabel>
#include <QMovie>

class EmojiItem : public QLabel
{
    Q_OBJECT
public:
    explicit EmojiItem(const QString &fileName, QWidget *parent = nullptr);
    ~EmojiItem();

    void initGifMovie();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;


signals:

public slots:

private:
    QString m_fileName;
    QMovie *m_movie = nullptr;
};

#endif // EMJOIITEM_H
