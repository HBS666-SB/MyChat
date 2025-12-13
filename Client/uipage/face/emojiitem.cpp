#include "emojiitem.h"

EmojiItem::EmojiItem(const QString &fileName, QWidget *parent)
    : QLabel(parent), m_fileName(fileName)
{
    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);  // 忽略默认尺寸限制
    setMinimumSize(0, 0);
    setAlignment(Qt::AlignCenter); // 居中显示

    setScaledContents(true);
    // 初始化显示图片
    initGifMovie();

}

EmojiItem::~EmojiItem()
{

}

void EmojiItem::initGifMovie()
{
    if(m_movie){
        m_movie->stop();
        delete m_movie;
    }
    m_movie = new QMovie(m_fileName);
    if(!m_movie->isValid()){
        setText("图片加载失败");
        return;
    }
    m_movie->setScaledSize(this->size());
    setMovie(m_movie);
    m_movie->start();
    m_movie->stop();
}

void EmojiItem::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    if(m_movie) {
        m_movie->setScaledSize(this->size());
    }
}

void EmojiItem::enterEvent(QEvent *event)
{
    if(!m_movie->isValid()){
        return;
    }
    m_movie->start();
    return QLabel::enterEvent(event);
}

void EmojiItem::leaveEvent(QEvent *event)
{
    if(!m_movie->isValid()){
        return;
    }
    m_movie->stop();
    m_movie->jumpToFrame(0);
    return QLabel::enterEvent(event);
}
