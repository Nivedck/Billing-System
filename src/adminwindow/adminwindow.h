#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QDialog>

namespace Ui {
class AdminWindow;
}

class AdminWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

private slots:
    void on_buttonDelete_clicked();
    void on_buttonUpdate_clicked();
    void on_buttonAdd_clicked();
    void on_buttonAddAdmin_clicked();
    void on_buttonDeleteAdmin_clicked();
    void on_buttonClearDatabase_clicked();


private:
    Ui::AdminWindow *ui;
    void loadProducts();
    void loadAdmins();
};

#endif
