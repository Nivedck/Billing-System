#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adminwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QStringListModel>
#include <QCompleter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToDatabase();
    loadProductsFromDatabase();
    setupAutoComplete();

    connect(ui->lineEditProductName, &QLineEdit::returnPressed, this, &MainWindow::tryAddToCart);
    connect(ui->lineEditQuantity, &QLineEdit::returnPressed, this, &MainWindow::tryAddToCart);
    connect(ui->buttonCheckout, &QPushButton::clicked, this, &MainWindow::on_buttonCheckout_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("products.db");

    if (!db.open()) {
        qDebug() << "Error: Could not connect to database.";
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS products ("
               "code INTEGER PRIMARY KEY, "
               "name TEXT NOT NULL, "
               "price REAL NOT NULL)");
}

void MainWindow::loadProductsFromDatabase()
{
    QSqlQuery query("SELECT code, name, price FROM products");
    while (query.next()) {
        int code = query.value(0).toInt();
        QString name = query.value(1).toString();
        double price = query.value(2).toDouble();

        productCatalog[code] = {name, price};

        int row = ui->tableCatalog->rowCount();
        ui->tableCatalog->insertRow(row);
        ui->tableCatalog->setItem(row, 0, new QTableWidgetItem(QString::number(code)));
        ui->tableCatalog->setItem(row, 1, new QTableWidgetItem(name));
        ui->tableCatalog->setItem(row, 2, new QTableWidgetItem(QString("₹ %1").arg(price, 0, 'f', 2)));
    }

    ui->tableCatalog->verticalHeader()->setVisible(false);
    ui->tableCatalog->setShowGrid(false);
}

void MainWindow::setupAutoComplete()
{
    model = new QStringListModel(this);
    completer = new QCompleter(this);
    completer->setModel(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->lineEditProductName->setCompleter(completer);

    QStringList names;
    QSqlQuery query("SELECT name FROM products");
    while (query.next()) {
        names << query.value(0).toString();
    }
    model->setStringList(names);
}

void MainWindow::addProductToCart(int code, const QString &name, double price)
{
    int qty = 1;
    cart[code] += qty;

    if (!productCatalog.contains(code)) {
        productCatalog[code] = {name, price};
    }

    updateCartDisplay();
    updateTotals();
}

void MainWindow::on_buttonAddToCart_clicked()
{
    tryAddToCart();
}

void MainWindow::tryAddToCart()
{
    QString input = ui->lineEditProductName->text().trimmed();
    QString qtyStr = ui->lineEditQuantity->text().trimmed();
    int qty = qtyStr.toInt();
    if (qty <= 0) qty = 1;

    if (input.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a product code or name.");
        return;
    }

    bool ok;
    int code = input.toInt(&ok);

    if (ok) {
        if (productCatalog.contains(code)) {
            cart[code] += qty;
            updateCartDisplay();
            updateTotals();
        } else {
            QMessageBox::warning(this, "Not Found", "Product code not found.");
        }
    } else {
        QSqlQuery query;
        query.prepare("SELECT code, price FROM products WHERE name = ?");
        query.addBindValue(input);

        if (query.exec() && query.next()) {
            int code = query.value(0).toInt();
            double price = query.value(1).toDouble();
            cart[code] += qty;

            if (!productCatalog.contains(code)) {
                productCatalog[code] = {input, price};
            }

            updateCartDisplay();
            updateTotals();
        } else {
            QMessageBox::warning(this, "Not Found", "Product name not found.");
        }
    }

    ui->lineEditProductName->clear();
    ui->lineEditQuantity->clear();
    ui->lineEditProductName->setFocus();
}

void MainWindow::updateCartDisplay()
{
    ui->tableCart->setRowCount(0);
    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int code = it.key();
        int qty = it.value();
        const Product& product = productCatalog[code];

        int row = ui->tableCart->rowCount();
        ui->tableCart->insertRow(row);
        ui->tableCart->setItem(row, 0, new QTableWidgetItem(product.name));
        ui->tableCart->setItem(row, 1, new QTableWidgetItem(QString::number(qty)));
        ui->tableCart->setItem(row, 2 ,new QTableWidgetItem(QString("₹ %1").arg(product.price, 0, 'f', 2)));
        double total = product.price * qty;
        ui->tableCart->setItem(row, 3, new QTableWidgetItem(QString("₹ %1").arg(total, 0, 'f', 2)));
    }

    ui->tableCart->verticalHeader()->setVisible(false);
    ui->tableCart->setShowGrid(false);
}

void MainWindow::updateTotals()
{
    double subtotal = 0.0;

    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int code = it.key();
        int qty = it.value();
        subtotal += productCatalog[code].price * qty;
    }

    double tax = subtotal * 0.05;
    double total = subtotal + tax;

    ui->labelSubtotal->setText(QString("Subtotal: ₹ %1").arg(subtotal, 0, 'f', 2));
    ui->labelTax->setText(QString("Tax (5%): ₹ %1").arg(tax, 0, 'f', 2));
    ui->labelTotal->setText(QString("Total: ₹ %1").arg(total, 0, 'f', 2));
}

void MainWindow::on_buttonClearCart_clicked()
{
    cart.clear();
    ui->tableCart->setRowCount(0);
    updateTotals();
}

void MainWindow::on_buttonCheckout_clicked()
{
    if (cart.isEmpty()) {
        QMessageBox::information(this, "Checkout", "Cart is empty.");
        return;
    }

    QSqlQuery query;
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int code = it.key();
        int qty = it.value();
        const Product& product = productCatalog[code];
        double total = product.price * qty;

        query.prepare("INSERT INTO sales (date, product_name, quantity, price, total) "
                      "VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(date);
        query.addBindValue(product.name);
        query.addBindValue(qty);
        query.addBindValue(product.price);
        query.addBindValue(total);

        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
        }
    }

    showBillInMessageBox();
    QMessageBox::information(this, "Checkout", "Purchase completed and saved!");
    cart.clear();
    updateCartDisplay();
    updateTotals();
}

void MainWindow::showBillInMessageBox()
{
    QString bill;
    bill += "========== Supermarket Bill ==========\n";
    bill += QString("Date: %1\n\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    bill += "Item\tQty\tPrice\tTotal\n";
    bill += "--------------------------------------\n";

    double subtotal = 0.0;
    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int code = it.key();
        int qty = it.value();
        const Product &product = productCatalog[code];
        double total = product.price * qty;
        subtotal += total;

        bill += QString("%1\t%2\t₹%3\t₹%4\n")
                    .arg(product.name)
                    .arg(qty)
                    .arg(product.price, 0, 'f', 2)
                    .arg(total, 0, 'f', 2);
    }

    double tax = subtotal * 0.05;
    double grandTotal = subtotal + tax;

    bill += "--------------------------------------\n";
    bill += QString("Subtotal:\t\t\t₹%1\n").arg(subtotal, 0, 'f', 2);
    bill += QString("Tax (5%%):\t\t\t₹%1\n").arg(tax, 0, 'f', 2);
    bill += QString("Total:\t\t\t₹%1\n").arg(grandTotal, 0, 'f', 2);
    bill += "======================================\n";
    bill += "      Thank you for shopping with us!\n";

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("Bill");
    msgBox->setTextFormat(Qt::PlainText);
    msgBox->setText(bill);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setMinimumSize(600, 400);
    msgBox->setStyleSheet("QLabel{min-width: 580px; min-height: 360px;}");
    msgBox->exec();
}

void MainWindow::on_removeButton_clicked()
{
    int selectedRow = ui->tableCart->currentRow();

    if (selectedRow >= 0) {
        QString productName = ui->tableCart->item(selectedRow, 0)->text();

        int codeToRemove = -1;
        for (auto it = productCatalog.begin(); it != productCatalog.end(); ++it) {
            if (it.value().name == productName) {
                codeToRemove = it.key();
                break;
            }
        }

        if (codeToRemove != -1) {
            cart.remove(codeToRemove);
        }

        ui->tableCart->removeRow(selectedRow);
        updateTotals();
    } else {
        QMessageBox::warning(this, "No Selection", "Please select an item to remove.");
    }
}

void MainWindow::on_buttonAdmin_clicked()
{
    if (adminWindow && adminWindow->isVisible()) {
        adminWindow->raise();
        adminWindow->activateWindow();
        return;
    }

    bool ok;
    QString password = QInputDialog::getText(this, "Admin Login", "Enter admin password:", QLineEdit::Password, "", &ok);
    if (ok && password == "nivedck") {
        adminWindow = new AdminWindow(this);
        adminWindow->show();
    } else if (ok) {
        QMessageBox::warning(this, "Access Denied", "Incorrect password.");
    }
}

void MainWindow::on_buttonRefreshCatalog_clicked()
{
    ui->tableCatalog->setRowCount(0);
    productCatalog.clear();
    loadProductsFromDatabase();
}

void MainWindow::recalculateTotals() {
    double subtotal = 0.0;

    int rowCount = ui->tableCart->rowCount();

    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *totalItem = ui->tableCart->item(row, 3);
        if (totalItem) {
            subtotal += totalItem->text().toDouble();
        }
    }

    double tax = subtotal * 0.05;
    double grandTotal = subtotal + tax;

    ui->labelSubtotal->setText(QString::number(subtotal, 'f', 2));
    ui->labelTax->setText(QString::number(tax, 'f', 2));
    ui->labelTotal->setText(QString::number(grandTotal, 'f', 2));
}
