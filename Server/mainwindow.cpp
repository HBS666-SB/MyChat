#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setQss(QString(":/qss/resource/qss/default.css"));
    ui->stackedWidgetFunc->setCurrentIndex(0);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->btnUserInfoPage,1);
    m_buttonGroup->addButton(ui->btnScorePage,0);
    m_buttonGroup->addButton(ui->btnScoreInsertPage,2);
    m_buttonGroup->addButton(ui->btnUsersPage,3);
    m_buttonGroup->addButton(ui->btnPasswordPage,4);
    m_buttonGroup->addButton(ui->btnDataBackup,5);

    connect(m_buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(sltButtonClicked(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnQuit_clicked()
{
    close();
}

void MainWindow::sltButtonClicked(int index)
{
    ui->stackedWidgetFunc->setCurrentIndex(index);
}
