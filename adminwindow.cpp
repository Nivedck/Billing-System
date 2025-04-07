#include "adminwindow.h"
#include "ui_adminwindow.h"

AdminWindow::AdminWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminWindow)  // ✅ Correct initializer
{
    ui->setupUi(this);
    this->setFixedSize(400, 300);  // optional: fix size
}

AdminWindow::~AdminWindow()
{
    delete ui;
}
