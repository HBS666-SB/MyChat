#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setQss(QString(":/qss/resource/qss/default.css"));
    setWindowFlags(Qt::FramelessWindowHint);
    ui->GCStackedWidget->setCurrentIndex(0);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->btnFrind,0);
    m_buttonGroup->addButton(ui->btnGroup,1);
    m_buttonGroup->addButton(ui->btnConversation,2);
    m_buttonGroup->addButton(ui->btnApplay,3);

    connect(m_buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(sltButtonClicked(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sltButtonClicked(int index)
{
    ui->GCStackedWidget->setCurrentIndex(index);
}
