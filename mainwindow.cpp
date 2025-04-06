#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeProductCatalog();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeProductCatalog() {
    productCatalog[101] = {"Apple", 10.0};
    productCatalog[102] = {"Milk", 25.0};
    productCatalog[103] = {"Bread", 20.0};
    productCatalog[104] = {"Eggs", 5.0};
    productCatalog[105] = {"Butter", 40.0};
    productCatalog[106] = {"Rice", 60.0};
    productCatalog[107] = {"Sugar", 30.0};
    productCatalog[108] = {"Salt", 15.0};
    productCatalog[109] = {"Soap", 35.0};
    productCatalog[110] = {"Tea", 50.0};

    // Optional: display catalog in the left QTableWidget
    for (auto it = productCatalog.begin(); it != productCatalog.end(); ++it) {
        int row = ui->tableCatalog->rowCount();
        ui->tableCatalog->insertRow(row);
        ui->tableCatalog->setItem(row, 0, new QTableWidgetItem(QString::number(it.key())));
        ui->tableCatalog->setItem(row, 1, new QTableWidgetItem(it.value().name));
        ui->tableCatalog->setItem(row, 2, new QTableWidgetItem(QString("₹ %1").arg(it.value().price, 0, 'f', 2)));
    }
}

void MainWindow::on_buttonAddToCart_clicked()
{
    int code = ui->lineEditCode->text().toInt();
    int qty = ui->lineEditQuantity->text().toInt();

    // Optional: You can add basic input validation here if needed

    cart[code] += qty;
    updateCartDisplay();
    updateTotals();

    // 🔽 Clear inputs
    ui->lineEditCode->clear();
    ui->lineEditQuantity->clear();

    // Optional: Set focus back to code input for quicker entry
    ui->lineEditCode->setFocus();
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

