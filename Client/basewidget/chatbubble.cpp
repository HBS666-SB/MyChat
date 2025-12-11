#include "chatbubble.h"

#include <QPainter>
#include <qevent.h>

BubbleList::BubbleList(QWidget *parent) :
    QWidget (parent)
{
    initVars();
    initWgts();
    initStgs();
    initConns();
}

BubbleList::~BubbleList()
{
    d->clear();
}
void BubbleList::initVars()
{

}

void BubbleList::initWgts()
{
    mainLayout = new QHBoxLayout(this);
    scrollbar = new QScrollBar(this);
    d = new BubbleListPrivate(this);
    this->setMinimumWidth(300);
    mainLayout->addWidget(d);
    mainLayout->addWidget(scrollbar);
    setLayout(mainLayout);
}

void BubbleList::initStgs() // 初始化样式和配置
{
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    scrollbar->setRange(0,0);
    scrollbar->hide();
}

void BubbleList::initConns()
{
    connect(scrollbar,SIGNAL(valueChanged(int)),d,SLOT(setCurrentIndex(int)));
    connect(d,SIGNAL(sig_setMaximum(int)),this,SLOT(setMaximum(int)));
    connect(d,SIGNAL(sig_setCurrentIndex(int)),scrollbar,SLOT(setValue(int)));
    connect(d,SIGNAL(sig_itemClicked(QString)),this,SIGNAL(sig_itemClicked(QString)));
    connect(d,SIGNAL(signalDownloadFile(QString)),this,SIGNAL(signalDownloadFile(QString)));
}


void BubbleList::addItem(ItemInfo *item)
{
    d->addItem(item);
}

void BubbleList::addItems(QVector<ItemInfo *> items)
{

}

void BubbleList::clear()
{

}

void BubbleList::render()
{

}

void BubbleList::setCurrItem(const int &index)
{
    scrollbar->setValue(index);
}

void BubbleList::resizeEvent(QResizeEvent *)
{

}

void BubbleList::calcGeo()
{

}

void BubbleList::setMaximum(int max)
{

}

void setCurrItem(const int &index)
{

}

//*******************私有的********************
BubbleListPrivate::BubbleListPrivate(QWidget *parent)
{
    initVars();
    initSettings();
}

BubbleListPrivate::~BubbleListPrivate()
{

}

void BubbleListPrivate::addItem(ItemInfo *item)
{
    m_IIVec.push_front(item);
    m_currIndex = 0;
    update();
}

void BubbleListPrivate::addItems(QVector<ItemInfo *> items)
{

}

void BubbleListPrivate::clear()
{

}

void BubbleListPrivate::render()
{

}

void BubbleListPrivate::setCurrentIndex(int curIndex)
{

}

void BubbleListPrivate::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent()";
    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
    drawBackground(&painter);
    drawItems(&painter);
}

void BubbleListPrivate::mouseMoveEvent(QMouseEvent *)
{

}

void BubbleListPrivate::mousePressEvent(QMouseEvent *)
{

}

void BubbleListPrivate::mouseDoubleClickEvent(QMouseEvent *e)
{

}

void BubbleListPrivate::resizeEvent(QResizeEvent *)
{

}

void BubbleListPrivate::leaveEvent(QEvent *)
{

}

void BubbleListPrivate::showEvent(QShowEvent *)
{

}

void BubbleListPrivate::wheelEvent(QWheelEvent *)
{

}

void BubbleListPrivate::drawBackground(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    painter->setBrush(QBrush(QColor("#ECECEC")));

    painter->drawRect(rect());
    painter->restore();
}

void BubbleListPrivate::drawItems(QPainter *painter)
{
    if(m_IIVec.count() == 0){
        return;
    }

}

void BubbleListPrivate::drawHoverRect(QPainter *painter)
{

}

void BubbleListPrivate::initVars()
{
    m_currIndex = 0;
    m_VisibleItemCnt = 0;
    m_ItemCounter = 0;

    m_bHover = false;
    m_bAllJobsDone = false;

    m_font = QFont("楷体",12);
    // 右键菜单
    picRightButtonMenu = new QMenu(this);
    picRightButtonMenu->addAction("保存图片");
    picRightButtonMenu->addSeparator();
    picRightButtonMenu->addAction("复制到粘贴板");
    picRightButtonMenu->hide();

    // 右键菜单
    fileRightButtonMenu = new QMenu(this);
    fileRightButtonMenu->addAction("下载文件");
    fileRightButtonMenu->addSeparator();
    fileRightButtonMenu->addAction("打开文件");
    fileRightButtonMenu->addAction("打开文件目录");
    fileRightButtonMenu->hide();


}

void BubbleListPrivate::initSettings()
{
    setMouseTracking(true);
}

void BubbleListPrivate::calcGeo()   //计算高度
{
    m_VisibleItemCnt = height() / (ITEM_HEIGHT + ITEM_SPACE + 10) + 1;
    int InvisibleItemCnt = m_IIVec.count() - m_VisibleItemCnt;
    if(InvisibleItemCnt > 0){
        emit sig_setMaximum(InvisibleItemCnt);
    }
}

void BubbleListPrivate::wheelUp()   //滚轮向上滚动消息
{

}

void BubbleListPrivate::wheelDown() //滚轮向下滚动消息
{

}

void BubbleListPrivate::SltFileMenuClicked(QAction *action)
{

}
