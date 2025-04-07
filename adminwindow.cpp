#include "adminwindow.h"
#include "ui_adminwindow.h"

#include <QSqlQuery>
#include <QMessageBox>

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

#include <QSqlQuery>
#include <QMessageBox>

void AdminWindow::on_buttonAddProduct_clicked()
{
    int code = ui->lineEditCode->text().toInt();
    QString name = ui->lineEditName->text();
    double price = ui->lineEditPrice->text().toDouble();

    QSqlQuery query;
    query.prepare("INSERT INTO products (code, name, price) VALUES (?, ?, ?)");
    query.addBindValue(code);
    query.addBindValue(name);
    query.addBindValue(price);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to add product.");
    } else {
        QMessageBox::information(this, "Success", "Product added successfully.");
    }

    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditPrice->clear();
}

void AdminWindow::on_buttonDeleteProduct_clicked()
{
    int code = ui->lineEditDeleteCode->text().toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM products WHERE code = ?");
    query.addBindValue(code);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to delete product.");
    } else if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Not Found", "No product with that code.");
    } else {
        QMessageBox::information(this, "Deleted", "Product deleted successfully.");
    }

    ui->lineEditDeleteCode->clear();
}

