#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct Product {
    QString name;
    double price;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_buttonAddToCart_clicked();
    void on_buttonClearCart_clicked();

private:
    Ui::MainWindow *ui;
    QMap<int, Product> productCatalog;  // Key = Product Code
    QMap<int, int> cart; // Key = Product Code, Value = Quantity
    void updateCartDisplay();
    void updateTotals();
    void connectToDatabase();
    void loadProductsFromDatabase();

};
#endif // MAINWINDOW_H
