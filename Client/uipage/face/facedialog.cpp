#include "facedialog.h"
#include "ui_facedialog.h"
#include <QDebug>

faceDialog::faceDialog(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::faceDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //加载图片
    for(int i = 0; i < 132; i++){
        //添加表情
        QString fileName = QString(":/resource/face/%1.gif").arg(i + 1);
        addEmojiItem(fileName);
    }
    m_selectFaceIndex = -1;
    ui->tableWidget->viewport()->installEventFilter(this);
}

faceDialog::~faceDialog()
{
    delete ui;
}

int faceDialog::getSelectFaceIndex()
{
    return m_selectFaceIndex;
}

bool faceDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tableWidget->viewport() && event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            return QWidget::eventFilter(watched, event);
        }

        QModelIndex index = ui->tableWidget->indexAt(mouseEvent->pos());
        if (!index.isValid()) { // 点击空白处不处理
            return QWidget::eventFilter(watched, event);
        }

        int currentRow = index.row();
        int currentCol = index.column();
        int currentIndex = currentRow * 12 + currentCol;

        if (currentIndex != m_selectFaceIndex) {
            m_selectFaceIndex = currentIndex;
            this->hide();
            emit signalSelectFaceIndex(m_selectFaceIndex);
            qDebug() << "鼠标点击触发：" << m_selectFaceIndex;
        }
        return true;
    }
    return QWidget::eventFilter(watched, event); // 其他事件正常传递
}

void faceDialog::addEmojiItem(QString fileName)
{
    int row = m_emojiList.size() / (ui->tableWidget->columnCount());
    int column = m_emojiList.size() % (ui->tableWidget->columnCount());
    //添加到哪个位置
    QTableWidgetItem *item = new QTableWidgetItem;

    ui->tableWidget->setItem(row, column, item);

    EmojiItem *em = new EmojiItem(fileName);
    ui->tableWidget->setCellWidget(row, column, em);

    m_emojiList.append(em);
}

void faceDialog::sltClose()
{
    this->close();
}

void faceDialog::on_pushButton_clicked()
{
    this->hide();
}
