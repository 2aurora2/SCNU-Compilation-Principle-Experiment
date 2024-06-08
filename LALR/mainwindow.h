#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>

#include "grammarset.h"
#include "infodialog.h"
#include "lalr.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString content;
    GrammarSet grammarSet;
    LALR lr1;

    void prework();
    void resetUI();
    void showFirstResult();
    void showFollowResult();
    void showLR1DFA();
    void showLALR1DFA();
    void showLALR1AnalyseTable();

private:
    Ui::MainWindow *ui;

private slots:
    void openSourceFile();
    void saveSourceFile();
    void analyseGrammar();
};
#endif // MAINWINDOW_H
