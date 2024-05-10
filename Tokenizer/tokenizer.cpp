#include "tokenizer.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QChar>
#include <QVector>
#include <QPair>
#include <QString>

Tokenizer::Tokenizer()
{
    prework();
}

// 读取mapping.json预处理mapping变量
void Tokenizer::prework(){
    QFile file(":/json/mapping.json");
    if (file.open(QIODevice::ReadOnly)){
            QJsonDocument mappingDoc = QJsonDocument::fromJson(file.readAll());
            QJsonObject mappingObj = mappingDoc.object();

            QList<QString> jsonKeys = mappingObj.keys();
            for(const QString& key : jsonKeys){
                mapping[key]=mappingObj.value(key).toString();
                // qDebug()<<key<<" "<<mapping[key];
            }
    }else {
        qDebug() << "无法打开mapping.json";
    }
}

// 分词并返回分词结果
QVector<QPair<QString, QString>> Tokenizer::tokenize(QString line){
    // 存储一行分词mapping
    QVector<QPair<QString, QString>> tmp;

    int pos=0;
    while(pos<line.size()&&(line[pos]==32||line[pos]==39)) pos++;

    if(pos==line.size()) return tmp;

    // line-comment 整行注释
    if(line[pos]=='/'&&(line[pos+1]=='/'||line[pos+1]=='*')){
        tmp.append(qMakePair(line.mid(pos),QString("注释")));
        return tmp;
    }
    while (pos<line.size()) {
        // 字符串
        if(line[pos].unicode()=='\''||line[pos].unicode()=='\"'){
            stringHandle(line,pos,tmp);
        }
        // 预编译指令
        else if(line[pos].unicode()=='#'){
            directiveHandle(line,pos,tmp);
        }
        // digit 数值
        else if(line[pos].isDigit()){
            numberHandle(line,pos,tmp);
        }
        // 尾部注释
        else if(line[pos].unicode()=='/'&&(line[pos+1].unicode()=='/'||line[pos+1].unicode()=='*')){
            tmp.append(qMakePair(line.mid(pos),QString("注释")));
            return tmp;
        }
        // 运算符
        else if(isOperator(line[pos].unicode())){
            operatorHandle(line,pos,tmp);
        }
        // 大中小括号
        else if(isBracket(line[pos].unicode())){
            QString bracket=line[pos++]+"";
            tmp.append(qMakePair(bracket,QString("括号")));
        }
        else if(line[pos].unicode()==';'){
            QString separator=line[pos++]+"";
            tmp.append(qMakePair(separator,QString("分隔符")));
        }
        // 标识符+关键字
        else if(isKeyword(line[pos].unicode())){
            keyWordHandle(line,pos,tmp);
        }
        else pos++;
    }
    return tmp;
}

void Tokenizer::numberHandle(QString line,int& pos,QVector<QPair<QString, QString>>& tmp){
    QString num;
    while(pos<line.size()&&line[pos].isDigit()) num+=line[pos++];
    // 十六进制
    if(line[pos]=='x'){
        num+=line[pos++];
        while (pos<line.size()&&(line[pos].isDigit()||(line[pos].toLower()>='a'&&line[pos].toLower()<='f'))) {
            num+=line[pos++];
        }
    }
    // 二进制
    else if(line[pos]=='b'){
        num+=line[pos++];
        while (pos<line.size()&&(line[pos]=='0'||line[pos]=='1')) {
            num+=line[pos++];
        }
    }
    // 科学计数法
    else if(line[pos].toLower()=='e'||line[pos]=='.'){
        num+=line[pos++];
        while (pos<line.size()&&line[pos].isDigit()) {
            num+=line[pos++];
        }
    }
    tmp.append(qMakePair(num,QString("数值")));
}

void Tokenizer::operatorHandle(QString line, int &pos,QVector<QPair<QString, QString>> &tmp){
    QString op;
    // 头文件
    if(line[pos]=='<'&&pastIsInclude){
        while(pos<line.size()&&line[pos].unicode()!='>') op+=line[pos++];
        op+=line[pos++];
        tmp.append(qMakePair(op,QString("头文件")));
        pastIsInclude=false;
    }
    else{
        while(pos<line.size()&&isOperator(line[pos].unicode())) op+=line[pos++];
        tmp.append(qMakePair(op,QString("运算符")));
    }
}

void Tokenizer::directiveHandle(QString line, int &pos, QVector<QPair<QString, QString>> &tmp){
    QString directive;
    while (pos<line.size()&&mapping.find(directive)==mapping.end()) {
        directive+=line[pos++];
    }
    tmp.append(qMakePair(directive,mapping[directive]));
    if(directive=="#include") pastIsInclude=true;
}

void Tokenizer::stringHandle(QString line, int &pos, QVector<QPair<QString, QString>> &tmp){
    QString str;
    if(line[pos]=='\''){
        str+=line[pos++];
        while(pos<line.size()&&line[pos].unicode()!='\'') str+=line[pos++];
        str+=line[pos++];
        tmp.append(qMakePair(str,QString("字符串")));
    }
    else{
        str+=line[pos++];
        while(pos<line.size()&&line[pos].unicode()!='\"') str+=line[pos++];
        str+=line[pos++];
        // 头文件
        if(pastIsInclude){
             tmp.append(qMakePair(str,QString("头文件")));
             pastIsInclude=false;
        }
        else tmp.append(qMakePair(str,QString("字符串")));
    }
}

void Tokenizer::keyWordHandle(QString line, int &pos, QVector<QPair<QString, QString>> &tmp){
    QString keyword;
    while (pos<line.size()&&(line[pos].unicode()=='_'||line[pos].isDigit()||line[pos].isLetter())) {
        keyword+=line[pos++];
    }
    if(mapping.find(keyword)!=mapping.end())
        tmp.append(qMakePair(keyword,QString("关键字")));
    else
        tmp.append(qMakePair(keyword,QString("标识符")));
}

bool Tokenizer::isOperator(char ch){
    return ch=='+'||ch=='-'||ch=='*'||ch=='/'||ch=='^'||ch=='&'||ch=='|'||ch=='%'||ch=='~'||ch=='<'||ch=='>'||ch=='='||ch==','||ch=='.'||ch==':'||ch=='?';
}

bool Tokenizer::isBracket(char ch){
    return ch=='('||ch==')'||ch=='{'||ch=='}'||ch=='['||ch==']';
}

bool Tokenizer::isKeyword(char ch){
    return ch=='_'||isdigit(ch)||isalpha(ch);
}
