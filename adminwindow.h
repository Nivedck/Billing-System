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


private:
    Ui::AdminWindow *ui;
    void loadProducts();    // ✅ This line is required!
};

#endif // ADMINWINDOW_H
