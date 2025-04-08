#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adminwindow.h" // include header
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToDatabase();
    loadProductsFromDatabase();
    setupAutoComplete();
    onProductNameEntered();


    connect(ui->lineEditProductName, &QLineEdit::returnPressed, this, &MainWindow::onProductNameEntered);




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

void MainWindow::onProductNameEntered()
{
    QString name = ui->lineEditProductName->text();

    QSqlQuery query;
    query.prepare("SELECT code, price FROM products WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        int code = query.value(0).toInt();
        double price = query.value(1).toDouble();

        // Call your existing function to add item to cart using code
        addProductToCart(code, name, price);
    } else {
        QMessageBox::warning(this, "Not Found", "Product not found.");
    }

    ui->lineEditProductName->clear();
}

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
    int code = ui->lineEditProductName->text().toInt();
    int qty = ui->lineEditQuantity->text().toInt();

    // Optional: You can add basic input validation here if needed

    cart[code] += qty;
    updateCartDisplay();
    updateTotals();

    // 🔽 Clear inputs
    ui->lineEditProductName->clear();
    ui->lineEditQuantity->clear();

    // Optional: Set focus back to code input for quicker entry
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

