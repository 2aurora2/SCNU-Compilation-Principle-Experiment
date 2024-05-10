#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dfa.h"
#include "nfa.h"

#include "common.h"
#include <QStack>
#include <QMainWindow>
#include <QStandardItemModel>

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
    void on_analysis_btn_clicked();

    void on_nfa_btn_clicked();

    void on_dfa_btn_clicked();

    void on_min_dfa_btn_clicked();

private:
    Ui::MainWindow *ui;

    QString input_regex;
    QVector<QString> nor_regex;
    NFA nfa;
    DFA dfa;
    DFA mindfa;
    QStack<Edge> stateStk;

    void setTableHeader(QVector<QString> headers);
    void addTableItem(int row, QVector<QString> items);
    QString regexPrework(QString regex);
    QString addConnectSymbol(QString regex,QVector<QString> nor);
    void regexToPostfix(QString regex);
    void postfixToNFA();
    void NFAToDFA();
    void minimizeDFA();
};
#endif // MAINWINDOW_H
