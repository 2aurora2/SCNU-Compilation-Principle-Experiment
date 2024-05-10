#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QVector>

enum TokenType{
    KEYWORD,        // 关键字
    OPERATION,     // 运算符
    IDENTIFIER,      // 标识符
    NUMBER,         // 数值
    LETTER,            // 字符
    COMMENT,     // 注释
    SEPARATOR    // 分隔符
};

struct Token{
    TokenType tokenType;
    QString token;
    int row;
};

struct TokenTreeNode{
    Token token;
    QVector<TokenTreeNode*> broVec;
    QVector<TokenTreeNode*> sonVec;

    TokenTreeNode();

    TokenTreeNode(QString con){
        token.token=con;
    }
};

struct ErrorInfo{
    QString info;
    int row;

    ErrorInfo();
    ErrorInfo(QString s,int r):info(s),row(r){}
};

#endif // COMMON_H
