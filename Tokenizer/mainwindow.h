#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tokenizer.h"

#include <QMainWindow>
#include <QVector>
#include <QPair>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_upload_btn_clicked();

    void on_tokenizer_btn_clicked();

private:
    Ui::MainWindow *ui;
    QVector<QString> lines;
    Tokenizer* tokenizer;

    QVector<QPair<QString, QString>> resultMap;
};
#endif // MAINWINDOW_H
