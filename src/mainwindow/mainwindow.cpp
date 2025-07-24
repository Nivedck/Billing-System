#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adminwindow.h"
#include "logindialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QStringListModel>
#include <QCompleter>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , adminWindow(nullptr)
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
               "product_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL UNIQUE, "
               "price REAL NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS invoices ("
               "invoice_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "timestamp TEXT NOT NULL, "
               "total_amount REAL NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS invoice_items ("
               "invoice_item_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "invoice_id INTEGER NOT NULL, "
               "product_id INTEGER NOT NULL, "
               "quantity INTEGER NOT NULL, "
               "price_at_purchase REAL NOT NULL, "
               "FOREIGN KEY(invoice_id) REFERENCES invoices(invoice_id), "
               "FOREIGN KEY(product_id) REFERENCES products(product_id))");

    query.exec("CREATE TABLE IF NOT EXISTS admins ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT NOT NULL UNIQUE, "
               "password_hash TEXT NOT NULL)");

    // Add a default admin if the table is empty
    query.exec("SELECT COUNT(*) FROM admins");
    if (query.next() && query.value(0).toInt() == 0) {
        QString defaultUsername = "admin";
        QString defaultPassword = "admin";
        QString passwordHash = QString(QCryptographicHash::hash(defaultPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

        query.prepare("INSERT INTO admins (username, password_hash) VALUES (?, ?)");
        query.addBindValue(defaultUsername);
        query.addBindValue(passwordHash);
        query.exec();
    }
}

void MainWindow::loadProductsFromDatabase()
{
    ui->tableCatalog->setRowCount(0); // Clear existing rows
    productCatalog.clear(); // Clear existing product data

    QSqlQuery query("SELECT product_id, name, price FROM products");
    while (query.next()) {
        int productId = query.value(0).toInt();
        QString name = query.value(1).toString();
        double price = query.value(2).toDouble();

        productCatalog[productId] = {name, price};

        int row = ui->tableCatalog->rowCount();
        ui->tableCatalog->insertRow(row);
        ui->tableCatalog->setItem(row, 0, new QTableWidgetItem(QString::number(productId)));
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
        QMessageBox::warning(this, "Input Error", "Please enter a product ID or name.");
        return;
    }

    bool ok;
    int productId = input.toInt(&ok);

    if (ok) {
        if (productCatalog.contains(productId)) {
            cart[productId] += qty;
            updateCartDisplay();
            updateTotals();
        } else {
            QMessageBox::warning(this, "Not Found", "Product ID not found.");
        }
    } else {
        QSqlQuery query;
        query.prepare("SELECT product_id, price FROM products WHERE name = ?");
        query.addBindValue(input);

        if (query.exec() && query.next()) {
            int productId = query.value(0).toInt();
            double price = query.value(1).toDouble();
            cart[productId] += qty;

            if (!productCatalog.contains(productId)) {
                productCatalog[productId] = {input, price};
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
        int productId = it.key();
        int qty = it.value();
        const Product& product = productCatalog[productId];

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
        int productId = it.key();
        int qty = it.value();
        subtotal += productCatalog[productId].price * qty;
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

    double subtotal = 0.0;
    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int productId = it.key();
        int qty = it.value();
        subtotal += productCatalog[productId].price * qty;
    }
    double tax = subtotal * 0.05;
    double totalAmount = subtotal + tax;

    QSqlQuery query;
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    query.prepare("INSERT INTO invoices (timestamp, total_amount) VALUES (?, ?)");
    query.addBindValue(date);
    query.addBindValue(totalAmount);
    if (!query.exec()) {
        qDebug() << "Insert into invoices failed:" << query.lastError().text();
        return;
    }

    int invoiceId = query.lastInsertId().toInt();

    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int productId = it.key();
        int qty = it.value();
        const Product& product = productCatalog[productId];

        query.prepare("INSERT INTO invoice_items (invoice_id, product_id, quantity, price_at_purchase) "
                      "VALUES (?, ?, ?, ?)");
        query.addBindValue(invoiceId);
        query.addBindValue(productId);
        query.addBindValue(qty);
        query.addBindValue(product.price);

        if (!query.exec()) {
            qDebug() << "Insert into invoice_items failed:" << query.lastError().text();
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
        int productId = it.key();
        int qty = it.value();
        const Product &product = productCatalog[productId];
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

        int productIdToRemove = -1;
        for (auto it = productCatalog.begin(); it != productCatalog.end(); ++it) {
            if (it.value().name == productName) {
                productIdToRemove = it.key();
                break;
            }
        }

        if (productIdToRemove != -1) {
            cart.remove(productIdToRemove);
        }

        ui->tableCart->removeRow(selectedRow);
        updateTotals();
    } else {
        QMessageBox::warning(this, "No Selection", "Please select an item to remove.");
    }
}

void MainWindow::on_buttonAdmin_clicked()
{
    LoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.username();
        QString password = dialog.password();

        QSqlQuery query;
        query.prepare("SELECT password_hash FROM admins WHERE username = ?");
        query.addBindValue(username);

        if (query.exec() && query.next()) {
            QString storedHash = query.value(0).toString();
            QString enteredHash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

            if (storedHash == enteredHash) {
                AdminWindow *admin = new AdminWindow(this);
                admin->exec();
                loadProductsFromDatabase();
            } else {
                QMessageBox::warning(this, "Access Denied", "Incorrect password.");
            }
        } else {
            QMessageBox::warning(this, "Access Denied", "User not found.");
        }
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
