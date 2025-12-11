#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include "iteminfo.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QWidget>

class BubbleListPrivate;

/**
 * @brief 聊天气泡列表公开接口类
 * @details 对外提供核心操作接口，管理布局和滚动条，转发私有实现类的信号
 */
class BubbleList: public QWidget
{
    Q_OBJECT
public:
    BubbleList(QWidget* parent = 0);
    ~BubbleList();

public:
    /// 代理私有实现类的公开接口
    void addItem(ItemInfo *item);
    void addItems(QVector<ItemInfo*> items);

    void clear();
    void render();

    void setCurrItem(const int &index);

protected:
    QSize sizeHint() const
    {
        return QSize(SIZE_HINT);
    }

    void resizeEvent(QResizeEvent *);

private:
    /// 私有工具函数
    void initVars(); // 初始化成员变量
    void initWgts(); // 初始化子控件
    void initStgs(); // 初始化样式和配置
    void initConns(); // 初始化信号槽连接

    void calcGeo(); // 计算布局

private Q_SLOTS:
    void setMaximum(int max);

private:
    QHBoxLayout* mainLayout;
    QScrollBar* scrollbar;  //滚动条
    BubbleListPrivate* d;

signals:
    void sig_setCurrentIndex(int currIndex);
    void sig_itemClicked(const QString& str);
    void signalDownloadFile(const QString &fileName);
};

class BubbleListPrivate : public QWidget
{
    Q_OBJECT
public:
    explicit BubbleListPrivate(QWidget *parent = 0);
    ~BubbleListPrivate();

    void addItem(ItemInfo *item);
    void addItems(QVector<ItemInfo*> items);
    void clear();
    void render();
    int itemCount() const
    {
        return m_IIVec.count();
    }


public slots:
    void setCurrentIndex(int curIndex);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *);
    void leaveEvent(QEvent *);
    void showEvent(QShowEvent *);
    void wheelEvent(QWheelEvent *);

private:
    void drawBackground(QPainter* painter);
    void drawItems(QPainter* painter);
    void drawHoverRect(QPainter* painter);

private:
    void initVars();
    void initSettings();
    void calcGeo(); //计算高度
    void wheelUp(); //滚轮向上滚消息
    void wheelDown();   //滚轮向下滚消息

private:
    QVector<ItemInfo*> m_IIVec; //存储文本消息

    int m_currIndex;    //当前滚动到的文本
    int m_selectedIndex;    //选中的消息项索引
    int m_VisibleItemCnt;   //可视区的消息数量
    int m_ItemCounter;  //消息项总数计数器
    int m_nHoverItemIndex;  //悬浮项的消息索引


    QRectF m_HoverRect; //悬浮项的矩形区域

    QString m_strHoverText; //悬浮项的文本内容

    QFont m_font;   //消息文本绘制字体

    bool m_bHover;   //鼠标悬浮状态标记
    bool m_bAllJobsDone;    //初始化完成标记


private Q_SLOTS:
    void SltFileMenuClicked(QAction *action);
Q_SIGNALS:
    void sig_setMaximum(int max);
    void sig_setCurrentIndex(int currIndex);
    void sig_itemClicked(const QString& str);
    void signalDownloadFile(const QString &fileName);

private:
    QMenu *picRightButtonMenu;
    QMenu *fileRightButtonMenu;

};

#endif // CHATBUBBLE_H
