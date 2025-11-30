#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "basewidget/customwidget.h"

class QButtonGroup;

namespace Ui {
class MainWindow;
}

class MainWindow : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnQuit_clicked();
    void sltButtonClicked(int index);

private:
    Ui::MainWindow *ui;

    QButtonGroup *m_buttonGroup;
};

#endif // MAINWINDOW_H
