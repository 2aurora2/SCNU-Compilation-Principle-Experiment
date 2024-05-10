#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QMap>
#include <QStandardItemModel>

#include "common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void separateLine();
    void wordAnalyse();
    void delCommentToken();
    void syntaxAnalyse();
    void showWordAnalyseRes();
    QStandardItem* insertTreeNode(QString str,QStandardItem *parent);
    void dfs(TokenTreeNode* t,QStandardItem *parent);
    void showSyntaxAnalyseRes();

    QString content;
    QVector<QString> lines;
    QMap<QString,TokenType> mapping;
    QVector<Token> tokens;
    QString typeList[7]={"关键字","运算符","标识符","数值","字符","注释","分隔符"};

    int curTokenIdx;
    Token curToken;
    TokenTreeNode *syntaxRoot;
    QVector<ErrorInfo> errVec;

    void matchChar(QString ch);
    Token matchType(TokenType tt);
    void getToken();
    void ERROR(QString msg);
    TokenTreeNode *program();
    TokenTreeNode *stmtSequence();
    TokenTreeNode *statement();
    TokenTreeNode *ifStmt();
    TokenTreeNode *repeatStmt();
    TokenTreeNode *assignStmt();
    TokenTreeNode *assignSubStmt();
    TokenTreeNode *readStmt();
    TokenTreeNode *writeStmt();
    TokenTreeNode *writeSubStmt();
    TokenTreeNode *forStmt();
    TokenTreeNode *whileStmt();
    TokenTreeNode *exp();
    TokenTreeNode *simpleExp();
    TokenTreeNode *term();
    TokenTreeNode *power();
    TokenTreeNode *factor();
    TokenTreeNode *selfExp();
    TokenTreeNode *regex();
    TokenTreeNode *orregex();
    TokenTreeNode *andregex();
    TokenTreeNode *singlere();

private:
    Ui::MainWindow *ui;

private slots:
    void openSourceFile();
    void saveSourceFile();
    void analyseSourceFile();
};
#endif // MAINWINDOW_H
