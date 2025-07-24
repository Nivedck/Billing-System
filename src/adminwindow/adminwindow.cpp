#include "adminwindow.h"
#include "ui_adminwindow.h"

#include <QSqlQuery>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QSqlError>

AdminWindow::AdminWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminWindow)
{
    ui->setupUi(this);
    this->setFixedSize(600, 400);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "ID" << "Name" << "Price" << "Stock");
    ui->tableWidgetAdmins->setColumnCount(2);
    ui->tableWidgetAdmins->setHorizontalHeaderLabels(QStringList() << "ID" << "Username");

    loadProducts();
    loadAdmins();
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::loadProducts()
{
    ui->tableWidget->setRowCount(0);

    QSqlQuery query("SELECT product_id, name, price, stock_quantity FROM products");

    int row = 0;
    while (query.next()) {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        row++;
    }
}

void AdminWindow::loadAdmins()
{
    ui->tableWidgetAdmins->setRowCount(0);

    QSqlQuery query("SELECT id, username FROM admins");

    int row = 0;
    while (query.next()) {
        ui->tableWidgetAdmins->insertRow(row);
        ui->tableWidgetAdmins->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidgetAdmins->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        row++;
    }
}


void AdminWindow::on_buttonAdd_clicked()
{
    QString name = ui->lineEditName->text();
    double price = ui->lineEditPrice->text().toDouble();
    int stock = ui->lineEditStock->text().toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO products (name, price, stock_quantity) VALUES (?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(stock);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to add product.");
    } else {
        QMessageBox::information(this, "Success", "Product added.");
        loadProducts();
    }


    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditPrice->clear();
    ui->lineEditStock->clear();
}

void AdminWindow::on_buttonUpdate_clicked()
{
    int productId = ui->lineEditCode->text().toInt();
    QString name = ui->lineEditName->text();
    double price = ui->lineEditPrice->text().toDouble();
    int stock = ui->lineEditStock->text().toInt();

    QSqlQuery query;
    query.prepare("UPDATE products SET name = ?, price = ?, stock_quantity = ? WHERE product_id = ?");
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(stock);
    query.addBindValue(productId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to update product.");
    } else if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Not Found", "Product not found.");
    } else {
        QMessageBox::information(this, "Success", "Product updated.");
        loadProducts();
    }

}

void AdminWindow::on_buttonDelete_clicked()
{
    int productId = ui->lineEditCode->text().toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM products WHERE product_id = ?");
    query.addBindValue(productId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to delete product.");
    } else if (query.numRowsAffected() == 0) {
        QMessageBox::information(this, "Not Found", "No product with that ID.");
    } else {
        QMessageBox::information(this, "Deleted", "Product deleted.");
        loadProducts();
    }


    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditPrice->clear();
}

void AdminWindow::on_buttonAddAdmin_clicked()
{
    QString username = ui->lineEditAdminUsername->text();
    QString password = ui->lineEditAdminPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both username and password.");
        return;
    }

    QString passwordHash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery query;
    query.prepare("INSERT INTO admins (username, password_hash) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(passwordHash);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to add admin.");
    } else {
        QMessageBox::information(this, "Success", "Admin added.");
        loadAdmins();
    }

    ui->lineEditAdminUsername->clear();
    ui->lineEditAdminPassword->clear();
}

void AdminWindow::on_buttonDeleteAdmin_clicked()
{
    int selectedRow = ui->tableWidgetAdmins->currentRow();

    if (selectedRow >= 0) {
        int adminId = ui->tableWidgetAdmins->item(selectedRow, 0)->text().toInt();

        QSqlQuery query;
        query.prepare("DELETE FROM admins WHERE id = ?");
        query.addBindValue(adminId);

        if (!query.exec()) {
            QMessageBox::warning(this, "Error", "Failed to delete admin.");
        } else if (query.numRowsAffected() == 0) {
            QMessageBox::information(this, "Not Found", "Admin not found.");
        } else {
            QMessageBox::information(this, "Deleted", "Admin deleted.");
            loadAdmins();
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select an admin to delete.");
    }
}

void AdminWindow::on_buttonClearDatabase_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Database", "Are you sure you want to clear ALL database data? This action cannot be undone.",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    QSqlQuery query;

    // Clear invoice_items first due to foreign key constraints
    if (!query.exec("DELETE FROM invoice_items")) {
        QMessageBox::warning(this, "Error", "Failed to clear invoice items: " + query.lastError().text());
        return;
    }
    if (!query.exec("DELETE FROM invoices")) {
        QMessageBox::warning(this, "Error", "Failed to clear invoices: " + query.lastError().text());
        return;
    }
    if (!query.exec("DELETE FROM products")) {
        QMessageBox::warning(this, "Error", "Failed to clear products: " + query.lastError().text());
        return;
    }
    if (!query.exec("DELETE FROM admins")) {
        QMessageBox::warning(this, "Error", "Failed to clear admins: " + query.lastError().text());
        return;
    }

    // Re-insert default admin
    QString defaultUsername = "admin";
    QString defaultPassword = "admin";
    QString passwordHash = QString(QCryptographicHash::hash(defaultPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

    query.prepare("INSERT INTO admins (username, password_hash) VALUES (?, ?)");
    query.addBindValue(defaultUsername);
    query.addBindValue(passwordHash);
    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to re-insert default admin: " + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success", "All database data cleared and default admin re-inserted.");
    loadProducts();
    loadAdmins();
}
