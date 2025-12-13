#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QMutex>
#include <QTimer>
#include <QFile>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QLineEdit>
QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QLineEdit;
class QHBoxLayout;
class QVBoxLayout;
QT_END_NAMESPACE

class CustomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent*) override;
};

//移动窗口类
class CustomMoveWidget : public CustomWidget
{
    Q_OBJECT
public:
    explicit CustomMoveWidget(QWidget *parent = nullptr);
    ~CustomMoveWidget();

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

    QPoint mousePoint;
    bool m_isPressed;

public:
    void setQss(const QString &filePath);

};

#endif // CUSTOMWIDGET_H
