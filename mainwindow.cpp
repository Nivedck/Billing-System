#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adminwindow.h" // include header
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime> // ✅ Required for QDateTime




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

void MainWindow::onProductNameEntered() {}


void MainWindow::addProductToCart(int code, const QString &name, double price)
{
    int qty = 1; // Default quantity as 1 when added via name search

    // If already in cart, increase qty
    cart[code] += qty;

    // Ensure the product is in the catalog map
    if (!productCatalog.contains(code)) {
        productCatalog[code] = {name, price};
    }

    updateCartDisplay();
    updateTotals();
}



void MainWindow::setupAutoComplete()
{
    model = new QStringListModel(this);
    completer = new QCompleter(this);
    completer->setModel(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->lineEditProductName->setCompleter(completer);

    // Fetch product names from DB
    QStringList names;
    QSqlQuery query("SELECT name FROM products");
    while (query.next()) {
        names << query.value(0).toString();
    }
    model->setStringList(names);
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





void MainWindow::on_buttonAddToCart_clicked()
{
    tryAddToCart();
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

    // 👇 Show the bill popup before clearing the cart
    showBillInMessageBox();

    QMessageBox::information(this, "Checkout", "Purchase completed and saved!");

    cart.clear();
    updateCartDisplay();
    updateTotals();
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

    double tax = subtotal * 0.05; // 5% tax
    double total = subtotal + tax;

    ui->labelSubtotal->setText(QString("Subtotal: ₹ %1").arg(subtotal, 0, 'f', 2));
    ui->labelTax->setText(QString("Tax (5%): ₹ %1").arg(tax, 0, 'f', 2));
    ui->labelTotal->setText(QString("Total: ₹ %1").arg(total, 0, 'f', 2));
}




void MainWindow::on_buttonClearCart_clicked()
{
    cart.clear();                         // Clear the cart data
    ui->tableCart->setRowCount(0);        // Clear the table
    updateTotals();
}


void MainWindow::on_buttonAdmin_clicked()
{
    bool ok;
    QString password = QInputDialog::getText(this, "Admin Login",
                                             "Enter admin password:",
                                             QLineEdit::Password,
                                             "", &ok);
    if (ok && password == "admin123") {
        AdminWindow *admin = new AdminWindow(this);
        admin->exec();
        loadProductsFromDatabase();       // Show as modal dialog
    } else if (ok) {
        QMessageBox::warning(this, "Access Denied", "Incorrect password.");
    }
}


void MainWindow::on_buttonRefreshCatalog_clicked()
{
    ui->tableCatalog->setRowCount(0); // Clear existing rows
    productCatalog.clear();           // Clear current catalog
    loadProductsFromDatabase();       // Reload from DB
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
        // Input is a product code
        if (productCatalog.contains(code)) {
            cart[code] += qty;
            updateCartDisplay();
            updateTotals();
        } else {
            QMessageBox::warning(this, "Not Found", "Product code not found.");
        }
    } else {
        // Input is a product name
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

void MainWindow::showBillInMessageBox()
{
    QString bill = "========== Supermarket Bill ==========\n";
    double subtotal = 0.0;

    for (auto it = cart.begin(); it != cart.end(); ++it) {
        int code = it.key();
        int qty = it.value();
        const Product& product = productCatalog[code];
        double total = product.price * qty;
        subtotal += total;

        bill += QString("%1\n  Qty: %2  Price: ₹%3  Total: ₹%4\n")
                    .arg(product.name)
                    .arg(qty)
                    .arg(product.price, 0, 'f', 2)
                    .arg(total, 0, 'f', 2);
    }

    double tax = subtotal * 0.05;
    double grandTotal = subtotal + tax;

    bill += "--------------------------------------\n";
    bill += QString("Subtotal: ₹%1\n").arg(subtotal, 0, 'f', 2);
    bill += QString("Tax (5%%): ₹%1\n").arg(tax, 0, 'f', 2);
    bill += QString("Total: ₹%1\n").arg(grandTotal, 0, 'f', 2);
    bill += "======================================";

    QMessageBox::information(this, "Bill", bill);
}


