#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include "basewidget/customwidget.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

private slots:
    void on_btnLogin_clicked();

private:
    Ui::LoginWidget *ui;
};

#endif // LOGINWIDGET_H
