#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVector>
#include <QString>
#include <QDebug>
#include <QRegExp>
#include <QStack>
#include <QStandardItemModel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->table_view->setEditTriggers (QAbstractItemView::NoEditTriggers);
    ui->table_view->resizeColumnsToContents();
    ui->table_view->horizontalHeader();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 分析
void MainWindow::on_analysis_btn_clicked()
{
    QStandardItemModel *model=new QStandardItemModel(ui->table_view);
    model->clear();
    ui->table_view->setModel(model);

    // 1. 分割输入框内容得到引入的标识符以及要转换的正则表达式
    QStringList text_input=ui->regex_edit->toPlainText().split('\n',QString::SkipEmptyParts);

    // 对空输入或者无要转换的正则表达式进行特判并提示用户
    if(text_input.size()==0){
        QMessageBox::information(this, "提示", "输入正则表达式不能为空！",
                    QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    nor_regex.clear();
    for(auto i : text_input){
        if(i[0]!='_'){
            int equalIndex=i.indexOf('=');
            if(equalIndex!=-1){
                nor_regex.push_back(i.left(equalIndex).trimmed());
            }
        }
        else{
            int equalIndex=i.indexOf('=');
            if(equalIndex!=-1){
                input_regex=i.mid(equalIndex+1);
            }
        }
    }

    if(input_regex.size()==0){
        QMessageBox::information(this, "提示", "输入正则表达式不能为空！",
                    QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    // 2. 对待转换的正则表达式进行方括号的预处理
    input_regex=regexPrework(input_regex);

    // 3. 对预处理完的正则表达式添加连接符
    input_regex=addConnectSymbol(input_regex,nor_regex);

    // 4. 将正则表达式转为后缀表达式
    regexToPostfix(input_regex);

    // 5. 后缀表达式转NFA
    postfixToNFA();

    // 6. NFA转DFA
    NFAToDFA();

    // 7. DFA最小化
    minimizeDFA();
}

// 展示正则表达式的NFA
void MainWindow::on_nfa_btn_clicked()
{
    if(input_regex.size()==0){
        QMessageBox::information(this, "提示", "输入正则表达式不能为空！",
                    QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    QStandardItemModel *model = new QStandardItemModel(ui->table_view);
    model->clear();
    for(int i=0;i<nfa.stateNum;++i){
        for(int j=0;j<nfa.stateNum;++j){
            if(nfa.stateMartix[i][j].size()>0){
                nfa.identification.insert(nfa.stateMartix[i][j]);
            }
        }
    }
    model->setHorizontalHeaderItem(0,new QStandardItem("状态\\字符"));
    QList<QString> list = QList<QString>(nfa.identification.begin(),nfa.identification.end());
    // 设置表头
    for(int i=0;i<list.size();++i){
        model->setHorizontalHeaderItem(i+1,new QStandardItem(list.at(i)));
    }
    // 设置状态列
    for(int i=0;i<nfa.stateNum;++i){
        model->setItem(i,0,new QStandardItem(QString::number(i)));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        // 表明起始态和终态
        if(i==nfa.startState) model->item(i,0)->setBackground(QBrush(Qt::lightGray));
        if(i==nfa.endState) model->item(i,0)->setBackground(QBrush(Qt::blue));
    }
    for(int i=0;i<nfa.stateNum;++i){
        for(int j=0;j<list.size();++j){
            if(list.at(j)!="eplision"){
                for(int k=0;k<nfa.stateNum;++k){
                    if(nfa.stateMartix[i][k]==list.at(j)){
                        model->setItem(i,j+1,new QStandardItem(QString::number(k)));
                        model->item(i,j+1)->setTextAlignment(Qt::AlignCenter);
                    }
                }
            }
            else{
                QString tmp;
                for(int k=0;k<nfa.stateNum;++k){
                    if(nfa.stateMartix[i][k]=="eplision"){
                        tmp+=QString::number(k);
                        tmp+="、";
                    }
                }
                model->setItem(i,j+1,new QStandardItem(tmp.left(tmp.size()-1)));
                model->item(i,j+1)->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
    ui->table_view->setModel(model);
}

// 展示正则表达式的DFA
void MainWindow::on_dfa_btn_clicked()
{
    if(input_regex.size()==0){
        QMessageBox::information(this, "提示", "输入正则表达式不能为空！",
                    QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    QStandardItemModel *model=new QStandardItemModel(ui->table_view);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("状态集合\\字符"));
    QList<QString> list = QList<QString>(dfa.identify.begin(),dfa.identify.end());
    // 设置表头
    for(int i=0;i<list.size();++i){
        model->setHorizontalHeaderItem(i+1,new QStandardItem(list.at(i)));
    }
    // 设置第一列
    for(int i=1;i<=dfa.stateNum;++i){
        QString colName="{ ";
        for(int j:dfa.M[i]){
            colName+=QString::number(j)+"，";
        }
        model->setItem(i-1,0,new QStandardItem(colName.left(colName.size()-1)+" }"));
        model->item(i-1,0)->setTextAlignment(Qt::AlignCenter);

        if(i==1) model->item(i-1,0)->setBackground(QBrush(Qt::lightGray));
        if(dfa.endState.contains(i)){
            model->item(i-1,0)->setBackground(QBrush(Qt::blue));
        }
    }
    // 设置每个单元格
    for(int i=1;i<=dfa.stateNum;++i){
        for(int j=0;j<list.size();++j){
            if(dfa.G.contains(i)&&dfa.G[i].contains(list[j])){
                QString label="{ ";
                for(int k:dfa.M[dfa.G[i][list[j]]]){
                    label+=QString::number(k)+"，";
                }
                model->setItem(i-1,j+1,new QStandardItem(label.left(label.size()-1)+" }"));
                model->item(i-1,j+1)->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
    ui->table_view->setModel(model);
}

// 展示最小化DFA
void MainWindow::on_min_dfa_btn_clicked()
{
    if(input_regex.size()==0){
        QMessageBox::information(this, "提示", "输入正则表达式不能为空！",
                    QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    QStandardItemModel *model=new QStandardItemModel(ui->table_view);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("状态集合\\字符"));
    QList<QString> list = QList<QString>(mindfa.identify.begin(),mindfa.identify.end());
    // 设置表头
    for(int i=0;i<list.size();++i){
        model->setHorizontalHeaderItem(i+1,new QStandardItem(list.at(i)));
    }
    // 设置第一列
    for(int i=1;i<=mindfa.stateNum;++i){
        model->setItem(i-1,0,new QStandardItem(QString::number(i)));
        model->item(i-1,0)->setTextAlignment(Qt::AlignCenter);
        if(i==mindfa.startState) model->item(i-1,0)->setBackground(QBrush(Qt::lightGray));
        if(mindfa.endState.contains(i)) model->item(i-1,0)->setBackground(QBrush(Qt::blue));
    }
    // 设置每个单元格
    for(int i=1;i<=mindfa.stateNum;++i){
        for(int j=0;j<list.size();++j){
            if(mindfa.G.contains(i)&&mindfa.G[i].contains(list[j])){
                model->setItem(i-1,j+1,new QStandardItem(QString::number(mindfa.G[i][list[j]])));
                model->item(i-1,j+1)->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
    ui->table_view->setModel(model);
}

// 将中括号处理为或运算符拼接的字符串
QString bracketToOr(QString str){
    int group=str.size()/3;
    QString res;
    for(int i=0;i<group;++i){
        int idx=3*i;
        for(char ch=str[idx].unicode();ch<=str[idx+2].unicode();ch++) {
            res.append(ch);
            res.append('|');
        }
    }
    return res.mid(0,res.size()-1);
}

// 判断是否为运算符
bool isOperator(QChar ch){
    if(ch=='*'||ch=='+'||ch=='?'||ch=='|'||ch=='.') return true;
    return false;
}

// 判断是否为字符
bool isChar(QChar ch){
    if(!isOperator(ch)&&ch!='('&&ch!=')') return true;
    return false;
}

// 初始的正则表达式预处理中括号
QString MainWindow::regexPrework(QString regex){
    QString processed_regex;
    for(int i=0;i<regex.size();){
        if(regex[i]!='['){
            if(regex[i]=='\\'){
                    processed_regex+=regex[i++];
                    processed_regex+=regex[i++];
            }
            else processed_regex+=regex[i++];
        }
        else{
            int j;
            for(j=i+1;j<regex.size();++j){
                if(regex[j]==']')
                    break;
            }
            processed_regex+='(';
            processed_regex+=bracketToOr(regex.mid(i+1,j-i-1));
            processed_regex+=')';
            i=j+1;
        }
    }
    return processed_regex;
}

// 预处理后的正则表达式添加连接符
QString MainWindow::addConnectSymbol(QString regex,QVector<QString> nor){
    QString processed_regex;
    for(int i=0;i<regex.size();){
        if(isOperator(regex[i])||regex[i]=='('||regex[i]==')'){
             processed_regex+=regex[i++];
             if(i<regex.size()){
                 if((regex[i-1]==')'||(regex[i-1])=='*'||(regex[i-1])=='?'||(regex[i-1])=='+')&&(regex[i]=='('||isChar(regex[i]))&&regex[i]!='\\')
                     processed_regex+='.';
             }
        }
        else if(regex[i]=='\\'){
                processed_regex+=regex[i++];
                processed_regex+=regex[i++];
                if(i<regex.size()){
                    if(regex[i]=='('||isChar(regex[i])) processed_regex+='.';
                }
        }
        else{
            bool flag=false;
            int len=-1;
            for(auto item:nor){
                if(item[0]==regex[i]&&i+item.size()-1<regex.size()){
                    if(regex.mid(i,item.size())==item){
                        flag=true;
                        len=item.size();
                        break;
                    }
                }
            }
            if(flag){
                processed_regex+=regex.mid(i,len);
                i+=len;
                if(i<regex.size()&&!isOperator(regex[i])&&regex[i]!=')') processed_regex.append('.');
            }
            else{
                processed_regex+=regex[i++];
                if(i<regex.size()&&!isOperator(regex[i])&&regex[i]!=')') processed_regex.append('.');
            }
        }
    }
    return processed_regex;
}

// 正则表达式转为后缀表达式
void MainWindow::regexToPostfix(QString regex){
    QStack<QChar> stk;
    nfa.vec.clear();
    struct Node tmp;
    for(int i=0;i<regex.size();){
        if(regex[i]=='(') stk.push(regex[i++]);
        else if(regex[i]==')'){
            while(!stk.empty()&&stk.top()!='('){
                QChar ch=stk.top();
                tmp.flag=2;
                tmp.ch=ch;
                nfa.vec.push_back(tmp);
                stk.pop();
            }
            if(!stk.empty()) stk.pop();
            ++i;
        }
        else if(isChar(regex[i])){
            tmp.num="";
            if(regex[i]=='\\'&&i+1<regex.size()){
                ++i;
                tmp.flag=1;
                tmp.num+=regex[i++];
                nfa.vec.push_back(tmp);
            }
            else{
                tmp.flag=1;
                while(i<regex.size()&&isChar(regex[i])){
                    tmp.num+=regex[i++];
                }
                nfa.vec.push_back(tmp);
            }
        }
        else{
            if(regex[i]!='|'&&regex[i]!='.'){
                tmp.flag=2;
                tmp.ch=regex[i++];
                nfa.vec.push_back(tmp);
            }
            else{
                if(stk.empty()||stk.top()=='(') stk.push(regex[i++]);
                else{
                    while(!stk.empty()&&stk.top()!='('){
                        tmp.flag=2;
                        tmp.ch=stk.top();
                        nfa.vec.push_back(tmp);
                        stk.pop();
                    }
                    stk.push(regex[i++]);
                }
            }
        }
    }
    while (!stk.empty()) {
        tmp.flag=2;
        tmp.ch=stk.top();
        nfa.vec.push_back(tmp);
        stk.pop();
    }
}

// 后缀表达式转NFA
void MainWindow::postfixToNFA(){
    stateStk.clear();
    nfa.restartNFA();
    for(auto item:nfa.vec){
        if(item.flag==1){
            Edge e=nfa.addRelation(item.num);
            stateStk.push(e);
        }
        else{
            if(item.ch=='*'){
                Edge e=stateStk.top();
                stateStk.pop();
                stateStk.push(nfa.closure(e));
            }
            else if(item.ch=='+'){
                Edge e=stateStk.top();
                stateStk.pop();
                stateStk.push(nfa.positiveClosure(e));
            }
            else if(item.ch=='?'){
                Edge e=stateStk.top();
                stateStk.pop();
                stateStk.push(nfa.optional(e));
            }
            else if(item.ch=='|'){
                Edge e1=stateStk.top();
                stateStk.pop();
                Edge e2=stateStk.top();
                stateStk.pop();
                stateStk.push(nfa.either(e1,e2));
            }
            else if(item.ch=='.'){
                Edge e1=stateStk.top();
                stateStk.pop();
                Edge e2=stateStk.top();
                stateStk.pop();
                stateStk.push(nfa.unionNFA(e2,e1));
            }
        }
    }
    nfa.startState=stateStk.top().s_num;
    nfa.endState=stateStk.top().e_num;

    for(int i=0;i<nfa.stateNum;++i){
        for(int j=0;j<nfa.stateNum;++j){
            if(nfa.stateMartix[i][j].size()>0){
                nfa.identification.insert(nfa.stateMartix[i][j]);
            }
        }
    }
}

// NFA转DFA
void MainWindow::NFAToDFA(){
    dfa.restartDFA();
    dfa.fromNFA(nfa);
}

// DFA最小化
void MainWindow::minimizeDFA(){
    mindfa.restartDFA();
    mindfa.fromDFA(dfa);
}
