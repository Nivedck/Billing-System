#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::username() const
{
    return ui->lineEditUsername->text();
}

QString LoginDialog::password() const
{
    return ui->lineEditPassword->text();
}
