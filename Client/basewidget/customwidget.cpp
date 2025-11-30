#include "customwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

CustomWidget::CustomWidget(QWidget *parent) : QWidget(parent)
{

}

void CustomWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

CustomMoveWidget::CustomMoveWidget(QWidget *parent) : CustomWidget(parent)
{
    m_isPressed = false;
}
CustomMoveWidget::~CustomMoveWidget()
{

}

void CustomMoveWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_isPressed = true;
        mousePoint = event->pos();
        return;
    }
    return QWidget::mousePressEvent(event);

}
void CustomMoveWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_isPressed = false;
        return;
    }
    return QWidget::mouseReleaseEvent(event);
}
void CustomMoveWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(m_isPressed && (event->buttons() & Qt::LeftButton)){
        QPoint movePoint = event->globalPos() - mousePoint;
        this->move(movePoint);
        return;
    }
    return QWidget::mouseMoveEvent(event);
}

void CustomMoveWidget::setQss(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Qss文件打开失败";
        return;
    }
    QTextStream stream(&file);
    setStyleSheet(stream.readAll());
    file.close();
}
