#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<QString>
#include <QVector>
#include <QPair>
#include<unordered_map>

class Tokenizer
{
public:
    Tokenizer();
    QVector<QPair<QString, QString>> tokenize(QString line);

private:
    std::unordered_map<QString,QString> mapping;
    bool pastIsInclude=false; // 上一个词是否为#include

    void prework();

    void numberHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp);
    void operatorHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp);
    void directiveHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp);
    void stringHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp);
    void keyWordHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp);

    bool isOperator(char ch);
    bool isBracket(char ch);
    bool isKeyword(char ch);
};

#endif // TOKENIZER_H
