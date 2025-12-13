#include "facedialog.h"
#include "ui_facedialog.h"


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
}

faceDialog::~faceDialog()
{
    delete ui;
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
