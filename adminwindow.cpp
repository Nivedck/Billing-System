#include "adminwindow.h"
#include "ui_adminwindow.h"

#include <QSqlQuery>
#include <QMessageBox>

AdminWindow::AdminWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminWindow)
{
    ui->setupUi(this);
    this->setFixedSize(600, 400);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Code" << "Name" << "Price");

    loadProducts();    // Optional size adjustment
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::loadProducts()
{
    ui->tableWidget->setRowCount(0);  // Clear existing rows

    QSqlQuery query("SELECT code, name, price FROM products");

    int row = 0;
    while (query.next()) {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
}


void AdminWindow::on_buttonAdd_clicked()
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
        QMessageBox::information(this, "Success", "Product added.");
        loadProducts();  // 🔥 Add this line
    }


    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditPrice->clear();
}

void AdminWindow::on_buttonUpdate_clicked()
{
    int code = ui->lineEditCode->text().toInt();
    QString name = ui->lineEditName->text();
    double price = ui->lineEditPrice->text().toDouble();

    QSqlQuery query;
    query.prepare("UPDATE products SET name = ?, price = ? WHERE code = ?");
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(code);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to update product.");
    } else if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Not Found", "Product not found.");
    } else {
        QMessageBox::information(this, "Success", "Product updated.");
        loadProducts();  // 🔥 Add this
    }

}

void AdminWindow::on_buttonDelete_clicked()
{
    int code = ui->lineEditCode->text().toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM products WHERE code = ?");
    query.addBindValue(code);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to delete product.");
    } else if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Not Found", "No product with that code.");
    } else {
        QMessageBox::information(this, "Deleted", "Product deleted.");
        loadProducts();  // 🔥 Add this
    }


    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditPrice->clear();
}
