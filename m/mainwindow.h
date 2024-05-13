#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addButton1_clicked();

    void on_addButton2_clicked();

    void on_deleteButton1_clicked();

    void on_editButton1_clicked();

    void on_saveButton_clicked();

    void on_searchButton1_clicked();

    void on_searchButton2_clicked();

    void on_editButton2_clicked();

    void on_saveButton2_clicked();

    void on_cancelButton1_clicked();

    void on_cancelButton2_clicked();


    void on_deleteButton2_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase DB_Connection;
    QSqlDatabase db;

    void updateRecord(int row);
    void clearInputFields();

    void updateRecord2(int row);
    void clearInputFields2();
    void onTabChanged(int index);
};
#endif // MAINWINDOW_H
