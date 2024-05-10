#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionopen,SIGNAL(triggered()),this,SLOT(openSourceFile()));
    connect(ui->actionsave,SIGNAL(triggered()),this,SLOT(saveSourceFile()));
    connect(ui->actionanalyse,SIGNAL(triggered()),this,SLOT(analyseSourceFile()));

    ui->errLabel->hide();

    // 读取mapping文件
    QFile file(":/file/mapping.json");
    if (file.open(QIODevice::ReadOnly)){
            QJsonDocument mappingDoc = QJsonDocument::fromJson(file.readAll());
            QJsonObject mappingObj = mappingDoc.object();

            QList<QString> jsonKeys = mappingObj.keys();
            for(const QString& key : jsonKeys){
                QString tokenType=mappingObj.value(key).toString();
                if(tokenType=="KEYWORD")
                    mapping[key]=KEYWORD;
                else if(tokenType=="OPERATOR")
                    mapping[key]=OPERATION;
                else
                    mapping[key]=SEPARATOR;
            }
    }
    else {
        qDebug() << "无法打开mapping.json";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 源代码分行存储
void MainWindow::separateLine()
{
    QString temp;
    lines.clear();
    for(int i=0;i<content.size();++i){
        if(content[i]!='\n') temp+=content[i];
        else{
            lines.push_back(temp);
            temp.clear();
        }
    }
    lines.push_back(temp);
}

// 词法分析
void MainWindow::wordAnalyse()
{
    int curRow=0,pos=0;
    QString line=lines[curRow];
    QString temp;
    Token tokenTmp;
    tokens.clear();

    while(curRow<lines.size()){
        while(pos<line.size()&&line[pos]==32) pos++;
        if(pos==line.size()&&curRow+1==lines.size()) break;
        if(pos==line.size()){
            line=lines[++curRow];
            pos=0;
            continue;
        }
        // 注释
        if(line[pos]=='{'){
            bool flag=false;
            pos++;
            temp.clear();
            while(!flag){
                while(pos<line.size()&&line[pos]!='}') temp+=line[pos++];
                if(pos==line.size()&&curRow+1==lines.size()) break;
                if(pos==line.size()){
                    line=lines[++curRow];
                    pos=0;
                }
                else{
                    flag=true;
                    tokenTmp.token=temp;
                    tokenTmp.tokenType=COMMENT;
                    tokens.push_back(tokenTmp);
                    pos++;
                }
            }
        }
        // 单个字符
        else if(line[pos]=='\''){
            tokenTmp.token.clear();
            tokenTmp.token+=line[++pos];
            tokenTmp.tokenType=LETTER;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
            tokenTmp.token.clear();
            pos+=2;
        }
        // 正则表达式
        else if(line[pos]=='='&&pos+1<line.size()&&line[pos+1]=='='){
            tokenTmp.token="==";
            tokenTmp.tokenType=OPERATION;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
            pos+=2;
            while(pos<line.size()&&line[pos]==32) pos++;
            while(pos<line.size()&&line[pos]!=';'){
                if(line[pos]=='\''){
                    tokenTmp.token.clear();
                    tokenTmp.token+=line[++pos];
                    tokenTmp.tokenType=LETTER;
                    tokenTmp.row=curRow;
                    tokens.push_back(tokenTmp);
                    pos+=2;
                }
                else if(line[pos].isLetter()){
                    temp.clear();
                    while(line[pos].isLetter()) temp+=line[pos++];
                    tokenTmp.token=temp;
                    tokenTmp.tokenType=IDENTIFIER;
                    tokenTmp.row=curRow;
                    tokens.push_back(tokenTmp);
                }
                else{
                    temp.clear();
                    temp+=line[pos++];
                    tokenTmp.token=temp;
                    if(temp=="("||temp==")") tokenTmp.tokenType=SEPARATOR;
                    else tokenTmp.tokenType=OPERATION;
                    tokenTmp.row=curRow;
                    tokens.push_back(tokenTmp);
                }
            }
        }
        // *、/、%、^运算符
        else if(line[pos]=='*'||line[pos]=='/'||line[pos]=='%'||line[pos]=='^'){
            tokenTmp.token.clear();
            tokenTmp.token+=line[pos++];
            tokenTmp.tokenType=OPERATION;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
        }
        // <、>、<=、>=、<>、=、:=运算符
        else if(line[pos]=='<'||line[pos]=='>'||line[pos]==':'||line[pos]=='='){
            temp.clear();
            temp+=line[pos++];
            if((temp=="<"||temp==">")&&pos<line.size()&&line[pos]=='=') temp+=line[pos++];
            else if(temp=="<"&&pos<line.size()&&line[pos]=='>') temp+=line[pos++];
            else if(temp==":"&&pos<line.size()&&line[pos]=='=') temp+=line[pos++];
            tokenTmp.token=temp;
            tokenTmp.tokenType=OPERATION;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
        }
        // +、-、++、--运算符
        else if(line[pos]=='+'||line[pos]=='-'){
            temp.clear();
            if(tokens.size()>0&&(tokens.back().tokenType!=1||(tokens.back().token!="+"&&tokens.back().token!="-"))){
                temp+=line[pos++];
                tokenTmp.token=temp;
                tokenTmp.tokenType=OPERATION;
                tokenTmp.row=curRow;
                tokens.push_back(tokenTmp);
                continue;
            }
            if(tokens.size()>0&&tokens.back().token.size()==1&&tokens.back().token[0]!=line[pos]){
                temp+=line[pos++];
                tokenTmp.token=temp;
                tokenTmp.tokenType=OPERATION;
                tokenTmp.row=curRow;
                tokens.push_back(tokenTmp);
                continue;
            }
            if(tokens.size()>0&&tokens.back().token.size()==1&&tokens.back().token[0]==line[pos]){
                if(tokens.size()>1&&(tokens[tokens.size()-2].tokenType==2||tokens[tokens.size()-2].tokenType==3)){
                    temp+=line[pos++];
                    tokenTmp.token=temp;
                    tokenTmp.tokenType=OPERATION;
                    tokenTmp.row=curRow;
                    tokens.push_back(tokenTmp);
                    continue;
                }
                else{
                    temp+=line[pos];
                    temp+=line[pos++];
                    tokens.removeLast();
                    tokenTmp.token=temp;
                    tokenTmp.tokenType=OPERATION;
                    tokenTmp.row=curRow;
                    tokens.push_back(tokenTmp);
                    continue;
                }
            }
            temp+=line[pos++];
            tokenTmp.token=temp;
            tokenTmp.tokenType=OPERATION;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
        }
        // 整型数值
        else if(line[pos].isDigit()){
            temp.clear();
            int _size=tokens.size();
            if(_size>1&&tokens[_size-2].tokenType==1&&(tokens[_size-1].token=="+"||tokens[_size-1].token=="-")){
                temp+=tokens[_size-1].token;
                tokens.removeLast();
            }
            while(pos<line.size()&&line[pos].isDigit()) temp+=line[pos++];
            tokenTmp.token=temp;
            tokenTmp.tokenType=NUMBER;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
        }
        // 标识符/关键字
        else if(line[pos].isLetter()){
            temp.clear();
            while (line[pos].isLetter()) temp+=line[pos++];
            tokenTmp.token=temp;
            if(mapping.find(temp)!=mapping.end()) tokenTmp.tokenType=mapping[temp];
            else tokenTmp.tokenType=IDENTIFIER;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
            tokenTmp.token.clear();
        }
        // 分隔符
        else if(line[pos]==';'||line[pos]=='('||line[pos]==')'){
            temp.clear();
            temp+=line[pos++];
            tokenTmp.token=temp;
            tokenTmp.tokenType=SEPARATOR;
            tokenTmp.row=curRow;
            tokens.push_back(tokenTmp);
        }
        else pos++;
    }
}

// 去除注释token
void MainWindow::delCommentToken()
{
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](Token value) {
            return value.tokenType == 5;
        }), tokens.end());
}

// 语法分析
void MainWindow::syntaxAnalyse()
{
    delCommentToken();
    curTokenIdx=0;
    curToken=tokens[0];
    errVec.clear();
    syntaxRoot=program();
}

// 分词结果展示
void MainWindow::showWordAnalyseRes()
{
    QStandardItemModel *model = new QStandardItemModel(ui->tableView);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("Token"));
    model->setHorizontalHeaderItem(1,new QStandardItem("类型"));

    for(int i=0;i<tokens.size();++i){
        model->setItem(i,0,new QStandardItem(tokens[i].token));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        model->setItem(i,1,new QStandardItem(typeList[tokens[i].tokenType]));
        model->item(i,1)->setTextAlignment(Qt::AlignCenter);
    }

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// 插入语法树结点
QStandardItem* MainWindow::insertTreeNode(QString str, QStandardItem *parent)
{
    QStandardItem *item = new QStandardItem;
    item->setEditable(false);
    item->setText(str);
    parent->appendRow(item);
    return item;
}

void MainWindow::dfs(TokenTreeNode *t,QStandardItem *parent)
{
    if(!t) return;
    QStandardItem* tmp=insertTreeNode(t->token.token,parent);
    for(int i=0;i<t->sonVec.size();++i){
        dfs(t->sonVec[i],tmp);
    }
    for(int i=0;i<t->broVec.size();++i){
        dfs(t->broVec[i],parent);
    }
}

// 语法分析结果展示
void MainWindow::showSyntaxAnalyseRes()
{
    QStandardItemModel *model = new QStandardItemModel(ui->treeView);
    model->clear();
    if(!errVec.empty()){
        ui->treeView->setModel(model);
        ui->errLabel->show();

        QStandardItemModel *_model = new QStandardItemModel(ui->errTableView);
        _model->clear();
        _model->setHorizontalHeaderItem(0,new QStandardItem("错误信息"));
        _model->setHorizontalHeaderItem(1,new QStandardItem("行"));

        for(int i=0;i<errVec.size();++i){
            _model->setItem(i,0,new QStandardItem(errVec[i].info));
            _model->item(i,0)->setTextAlignment(Qt::AlignCenter);
            _model->setItem(i,1,new QStandardItem(QString::number(errVec[i].row)));
            _model->item(i,1)->setTextAlignment(Qt::AlignCenter);
        }

        ui->errTableView->setModel(_model);
        ui->errTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    else{
        QStandardItemModel *_model = new QStandardItemModel(ui->errTableView);
        _model->clear();
        ui->errTableView->setModel(_model);
        ui->errLabel->hide();

        QStandardItem *item = new QStandardItem;
        item->setText(syntaxRoot->token.token);
        item->setEditable(false);
        model->appendRow(item);
        dfs(syntaxRoot->sonVec[0],item);

        ui->treeView->setModel(model);
        ui->treeView->setHeaderHidden(true);
    }
}

// 字符匹配
void MainWindow::matchChar(QString str)
{
    Token temp=curToken;
    if(curToken.token==str){
        getToken();
        qDebug()<<"匹配到"<<str<<"，下一个token为"<<curToken.token;
    }
    else ERROR("unexpected token \" "+ temp.token+" \".");
}

// 类型匹配
Token MainWindow::matchType(TokenType tt)
{
    Token temp=curToken;
    if(curToken.tokenType==tt){
        getToken();
        qDebug()<<"匹配到"<<temp.token<<"，下一个token为"<<curToken.token;
    }
    else ERROR("unexpected tokenType \" "+ curToken.token+"->"+typeList[curToken.tokenType]+" \".");
    return temp;
}

// 获取下一个token
void MainWindow::getToken()
{
    if(curTokenIdx==tokens.size()-1) return;
    curTokenIdx++;
    curToken=tokens[curTokenIdx];
}

// 出错函数
void MainWindow::ERROR(QString msg)
{
    errVec.push_back(ErrorInfo("Syntax error at line "+QString::number(curToken.row)+" : "+msg,curToken.row));
}

TokenTreeNode *MainWindow::program()
{
    TokenTreeNode * root=new TokenTreeNode("start");
    (root->sonVec).push_back(stmtSequence());
    qDebug()<<"匹配结束";
    return root;
}

TokenTreeNode *MainWindow::stmtSequence()
{
    TokenTreeNode *t=statement();
    TokenTreeNode *p=t;
    while(curToken.tokenType==6&&curToken.token==";"){
        matchChar(";");
        TokenTreeNode *q=statement();
        if(q!=nullptr){
            if(t==nullptr) t=p=q;
            else{
                p->broVec.push_back(q);
                p=q;
            }
        }
    }
    return t;
}

TokenTreeNode *MainWindow::statement()
{
    if(curToken.token=="if") return ifStmt();
    else if(curToken.token=="repeat") return repeatStmt();
    else if(curToken.tokenType==2) return assignStmt();
    else if(curToken.token=="read") return readStmt();
    else if(curToken.token=="write") return writeStmt();
    else if(curToken.token=="for") return forStmt();
    else if(curToken.token=="while") return whileStmt();
    else{
        ERROR("unexpected token \" "+ curToken.token+" \".");
        return nullptr;
    }
}

TokenTreeNode *MainWindow::ifStmt()
{
    TokenTreeNode *t=new TokenTreeNode("if");
    matchChar("if");
    matchChar("(");
    t->sonVec.push_back(exp());
    matchChar(")");
    t->sonVec.push_back(stmtSequence());
    if(curToken.token=="else"){
        t->sonVec.push_back(stmtSequence());
    }
    matchChar("endif");
    return t;
}

TokenTreeNode *MainWindow::repeatStmt()
{
    TokenTreeNode *t=new TokenTreeNode("repeat");
    matchChar("repeat");
    t->sonVec.push_back(stmtSequence());
    matchChar("until");
    t->sonVec.push_back(exp());
    return t;
}

TokenTreeNode *MainWindow::assignStmt()
{
    Token temp=matchType(IDENTIFIER);
    TokenTreeNode *t=new TokenTreeNode(temp.token);
    TokenTreeNode *p=assignSubStmt();
    if(p){
        p->sonVec.push_front(t);
    }
    return p;
}

TokenTreeNode *MainWindow::assignSubStmt()
{
    TokenTreeNode *t=nullptr;
    if(curToken.token==":="){
        t=new TokenTreeNode(":=");
        matchChar(":=");
        t->sonVec.push_back(exp());
    }
    else if(curToken.token=="=="){
        t=new TokenTreeNode("==");
        matchChar("==");
        t->sonVec.push_back(regex());
    }
    else ERROR("unexpected token \" "+ curToken.token+" \".");
    return t;
}

TokenTreeNode *MainWindow::readStmt()
{
    TokenTreeNode *temp=new TokenTreeNode("read");
    matchChar("read");
    temp->sonVec.push_back(new TokenTreeNode(matchType(IDENTIFIER).token));
    return temp;
}

TokenTreeNode *MainWindow::writeStmt()
{
    TokenTreeNode *temp=new TokenTreeNode("write");
    matchChar("write");
    temp->sonVec.push_back(writeSubStmt());
    return temp;
}

TokenTreeNode *MainWindow::writeSubStmt()
{
    if(curToken.tokenType==4){
        return new TokenTreeNode(curToken.token);
    }
    else return exp();
}

TokenTreeNode *MainWindow::forStmt()
{
   TokenTreeNode *t=new TokenTreeNode("for");
   matchChar("for");
   matchChar("(");
   t->sonVec.push_back(assignStmt());
   matchChar(";");
   t->sonVec.push_back(exp());
   matchChar(";");
   t->sonVec.push_back(selfExp());
   matchChar(")");
   t->sonVec.push_back(stmtSequence());
   matchChar("endfor");
   return t;
}

TokenTreeNode *MainWindow::whileStmt()
{
    TokenTreeNode *t=new TokenTreeNode("while");
    matchChar("while");
    matchChar("(");
    t->sonVec.push_back(exp());
    matchChar(")");
    t->sonVec.push_back(stmtSequence());
    matchChar("endwhile");
    return t;
}

TokenTreeNode *MainWindow::exp()
{
    TokenTreeNode *t=simpleExp();
    if(curToken.tokenType==1&&(curToken.token=="<"||curToken.token==">"||curToken.token=="<="||curToken.token==">="||curToken.token=="="||curToken.token=="<>")){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        getToken();
        p->sonVec.push_back(t);
        p->sonVec.push_back(simpleExp());
        return p;
    }
    return t;
}

TokenTreeNode *MainWindow::simpleExp()
{
    TokenTreeNode *t=term();
    while(curToken.tokenType==1&&(curToken.token=="+"||curToken.token=="-")){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        p->sonVec.push_back(term());
        t=p;
    }
    return t;
}

TokenTreeNode *MainWindow::term()
{
    TokenTreeNode *t=power();
    while(curToken.tokenType==1&&(curToken.token=="*"||curToken.token=="/"||curToken.token=="%")){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        p->sonVec.push_back(power());
        t=p;
    }
    return t;
}

TokenTreeNode *MainWindow::power()
{
    TokenTreeNode *t=factor();
    while(curToken.tokenType==1&&curToken.token=="^"){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        p->sonVec.push_back(factor());
        t=p;
    }
    return t;
}

TokenTreeNode *MainWindow::factor()
{
    TokenTreeNode *t=nullptr;
    if(curToken.tokenType==1&&curToken.token=="("){
        matchChar("(");
        t=exp();
        matchChar(")");
    }
    else if(curToken.tokenType==3){
        Token temp=matchType(NUMBER);
        t=new TokenTreeNode(temp.token);
    }
    else t=selfExp();
    return t;
}

TokenTreeNode *MainWindow::selfExp()
{
    TokenTreeNode *t=nullptr;
    TokenTreeNode *p;
    while(curToken.tokenType==1&&(curToken.token=="++"||curToken.token=="--")){
        if(t){
            TokenTreeNode *q=new TokenTreeNode(curToken.token);
            t->sonVec.push_back(q);
            t=q;
        }
        else {
            t=new TokenTreeNode(curToken.token);
            p=t;
        }
        qDebug()<<curToken.token<<"=========";
        matchChar(curToken.token);
    }
    Token temp=matchType(IDENTIFIER);
    if(t) {
        t->sonVec.push_back(new TokenTreeNode(temp.token));
        return p;
    }
    else return new TokenTreeNode(temp.token);
}

TokenTreeNode *MainWindow::regex()
{
    TokenTreeNode *t=orregex();
    while(curToken.tokenType==1&&curToken.token=="|"){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        p->sonVec.push_back(orregex());
        t=p;
    }
    return t;
}

TokenTreeNode *MainWindow::orregex()
{
    TokenTreeNode *t=andregex();
    while(curToken.tokenType==1&&curToken.token=="&"){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        p->sonVec.push_back(andregex());
        t=p;
    }
    return t;
}

TokenTreeNode *MainWindow::andregex()
{
    TokenTreeNode *t=singlere();
    if(curToken.tokenType==1&&(curToken.token=="#"||curToken.token=="?")){
        TokenTreeNode *p=new TokenTreeNode(curToken.token);
        matchChar(curToken.token);
        p->sonVec.push_back(t);
        return p;
    }
    return t;
}

TokenTreeNode *MainWindow::singlere()
{
    TokenTreeNode *t=nullptr;
    if(curToken.tokenType==6&&curToken.token=="("){
        matchChar("(");
        t=regex();
        matchChar(")");
        return t;
    }
    else if(curToken.tokenType==4){
        Token temp=matchType(LETTER);
        t=new TokenTreeNode(temp.token);
        return t;
    }
    else if(curToken.tokenType==2){
        Token temp=matchType(IDENTIFIER);
        t=new TokenTreeNode(temp.token);
        return t;
    }
    else {
        ERROR("unexpected tokenType \" "+ typeList[curToken.tokenType]+" \".");
        return t;
    }
}

// 打开源文件
void MainWindow::openSourceFile(){
    QString filePath = QFileDialog::getOpenFileName(this,"请选择一个Tiny源文件", "", "All Files (*.*);; Text Files (*.txt)");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "提示", "打开源文件失败，请重试！", QMessageBox::Yes);
    }
    QTextStream in(&file);
    content = in.readAll();
    ui->tinyFileEdit->setText(content);
}

// 保存源文件
void MainWindow::saveSourceFile()
{
    QString currentTime = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString tinySourceTxt = ui->tinyFileEdit->toPlainText();

    QString fileName = "Tiny-" + currentTime + ".txt";
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << tinySourceTxt;
        file.close();
        QMessageBox::information(nullptr, "提示", "源文件保存为 "+fileName+" 成功！", QMessageBox::Yes);
    } else {
        QMessageBox::warning(nullptr, "提示", "源文件保存失败！", QMessageBox::Yes);
    }
}

// 分析与展示
void MainWindow::analyseSourceFile()
{
    content=ui->tinyFileEdit->toPlainText();
    if(content.size()==0){
        QMessageBox::warning(nullptr, "提示", "无可分析的源程序！", QMessageBox::Yes);
    }
    else{
        separateLine();
        wordAnalyse();
        showWordAnalyseRes();
        syntaxAnalyse();
        showSyntaxAnalyseRes();
    }
}

