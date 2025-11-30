#ifndef ANIMATIONSTACKEDWIDGET_H
#define ANIMATIONSTACKEDWIDGET_H

#include <QPropertyAnimation>
#include <QStackedWidget>

class AnimationStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    enum AnimationType
    {
        TopToBottom,    //从上到下
        BottomToTop,    //从下到上
        LeftToRight,    //从左到右
        RightToLeft,    //从右到左
    };

    AnimationStackedWidget(QWidget *parent = nullptr);
    virtual ~AnimationStackedWidget();

    //获取类的对象名
    static QString getClassName();
    //根据索引启动动画
    void start(int index);
    //设置上一个和当前的索引
    void setIndex(int previous,int current);
    //设置动画的起始和结束距离以及动画类型
    void setLength(int length, AnimationType type);
    //设置动画时长
    void setDuration(int duration);
    //获取动画时长
    void getDuration() const;

private slots:
    //动画数值发生变化
    void valueChanged(const QVariant &value);
    //动画执行完成
    void animationFinished();

protected:
    //重写事件函数
    virtual void paintEvent(QPaintEvent *event) override;
    //绘制上一个部件
    void renderPreviousWidget(QPainter &painter,QTransform &transform);
    //绘制当前部件
    void renderCurrentWidget(QPainter &painter, QTransform &transform);

    bool m_isAnimating;
    float m_currentValue;
    int m_currentIndex, m_previousIndex;
    AnimationType m_type;
    QPropertyAnimation *m_animation;

};

#endif // ANIMATIONSTACKEDWIDGET_H
