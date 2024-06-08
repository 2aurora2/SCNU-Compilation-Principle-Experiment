#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    grammarSet.reset();

    connect(ui->open,SIGNAL(triggered()),this,SLOT(openSourceFile()));
    connect(ui->save,SIGNAL(triggered()),this,SLOT(saveSourceFile()));
    connect(ui->rule,SIGNAL(triggered()),this,SLOT(analyseGrammar()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::prework()
{
    // 清空上次文法相关数据
    grammarSet.reset();
    lr1.reset();
    content=ui->grammarTextEdit->toPlainText();

    // 预处理第一步：存储文法规则
    QStringList lines = content.split('\n');
    for(int i=0;i<lines.size();++i){
        QStringList tmp = lines[i].split(' ');
        Grammar grammar;
        grammar.left = tmp[0];
        if(!tmp.contains("|")){
            if(i==0) grammarSet.S = tmp[0];
            for(int j=2;j<tmp.size();++j) grammar.right.append(tmp[j]);
            grammarSet.grammarSet.append(grammar);
        }
        else{
            if(i==0){
                grammarSet.S = tmp[0]+"\'\'";
                Grammar start;
                start.left=grammarSet.S;
                start.right.append(tmp[0]);
                grammarSet.grammarSet.append(start);
            }
            for(int j=2;j<tmp.size();++j){
                if(tmp[j]=="|"){
                    grammarSet.grammarSet.append(grammar);
                    grammar.right.clear();
                }
                else grammar.right.append(tmp[j]);
            }
            grammarSet.grammarSet.append(grammar);
        }
    }
    grammarSet.grammarCnt=grammarSet.grammarSet.size();
    for(int i=0;i<grammarSet.grammarCnt;++i){
        grammarSet.notFinalSet.insert(grammarSet.grammarSet[i].left);
        grammarSet.tokens.insert(grammarSet.grammarSet[i].left);
        for(auto s:grammarSet.grammarSet[i].right) grammarSet.tokens.insert(s);
    }
}

void MainWindow::resetUI()
{
    ui->firstTableView->setModel(new QStandardItemModel());
    ui->followTableView->setModel(new QStandardItemModel());
    ui->lr1_DFA->setModel(new QStandardItemModel());
    ui->lalr1_DFA->setModel(new QStandardItemModel());
    ui->lalr1_Table->setModel(new QStandardItemModel());
}

void MainWindow::showFirstResult()
{
    QStandardItemModel *model = new QStandardItemModel(ui->firstTableView);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("非终结符"));
    model->setHorizontalHeaderItem(1,new QStandardItem("First集合"));

    int i=0;
    for(auto s:grammarSet.notFinalSet){
        model->setItem(i,0,new QStandardItem(s));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        QStringList list=grammarSet.notFirstHash[s].values();
        QString res=list.join("，");
        model->setItem(i,1,new QStandardItem(res));
        model->item(i,1)->setTextAlignment(Qt::AlignCenter);
        i++;
    }

    ui->firstTableView->setModel(model);
    ui->lr1_DFA->resizeRowsToContents();
    ui->lr1_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::showFollowResult()
{
    QStandardItemModel *model = new QStandardItemModel(ui->followTableView);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("非终结符"));
    model->setHorizontalHeaderItem(1,new QStandardItem("Follow集合"));

    int i=0;
    for(auto s:grammarSet.notFinalSet){
        model->setItem(i,0,new QStandardItem(s));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        QStringList list=grammarSet.notFollowHash[s].values();
        QString res=list.join("，");
        model->setItem(i,1,new QStandardItem(res));
        model->item(i,1)->setTextAlignment(Qt::AlignCenter);
        i++;
    }

    ui->followTableView->setModel(model);
    ui->lr1_DFA->resizeRowsToContents();
    ui->lr1_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::showLR1DFA()
{
    QStandardItemModel *model = new QStandardItemModel(ui->lr1_DFA);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("状态ID"));

    // 找到所有终结符
    QVector<QString> finalStr;
    for(int i=0;i<grammarSet.grammarCnt;++i){
        QVector<QString> vec=grammarSet.grammarSet[i].right;
        for(auto s:vec){
            if(!grammarSet.notFinalSet.contains(s)) finalStr.append(s);
        }
    }
    // 找到所有非终结符
    QVector<QString> notFinalStr;
    for(auto s:grammarSet.notFinalSet) notFinalStr.append(s);

    // 展示终结符的列名
    int t=1;
    for(auto final:finalStr){
        model->setHorizontalHeaderItem(t,new QStandardItem(final));
        t++;
    }
    // 展示非终结符的列名
    QBrush redBrush(Qt::red);
    for(auto notFinal:notFinalStr){
        QStandardItem *item = new QStandardItem(notFinal);
        item->setForeground(redBrush);
        model->setHorizontalHeaderItem(t,item);
        t++;
    }

    for(int i=0;i<lr1.states.size();++i){
        State tempState=lr1.states[i];
        // 设置状态ID
        model->setItem(i,0,new QStandardItem(QString::number(i)+":\n"+tempState.display()));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        int k=1;
        for(auto final:finalStr){
            if(lr1.lrDFA[i].contains(final)){
                int toID=lr1.lrDFA[i][final];
                model->setItem(i,k,new QStandardItem(QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
        for(auto final:notFinalStr){
            if(lr1.lrDFA[i].contains(final)){
                int toID=lr1.lrDFA[i][final];
                model->setItem(i,k,new QStandardItem(QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
    }
    ui->lr1_DFA->setModel(model);
    ui->lr1_DFA->resizeRowsToContents();
    ui->lr1_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::showLALR1DFA()
{
    QStandardItemModel *model = new QStandardItemModel(ui->lalr1_DFA);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("状态ID"));

    // 找到所有终结符
    QVector<QString> finalStr;
    for(int i=0;i<grammarSet.grammarCnt;++i){
        QVector<QString> vec=grammarSet.grammarSet[i].right;
        for(auto s:vec){
            if(!grammarSet.notFinalSet.contains(s)) finalStr.append(s);
        }
    }
    // 找到所有非终结符
    QVector<QString> notFinalStr;
    for(auto s:grammarSet.notFinalSet) notFinalStr.append(s);

    // 展示终结符的列名
    int t=1;
    for(auto final:finalStr){
        model->setHorizontalHeaderItem(t,new QStandardItem(final));
        t++;
    }
    // 展示非终结符的列名
    QBrush redBrush(Qt::red);
    for(auto notFinal:notFinalStr){
        QStandardItem *item = new QStandardItem(notFinal);
        item->setForeground(redBrush);
        model->setHorizontalHeaderItem(t,item);
        t++;
    }

    for(int i=0;i<lr1.stateNumPro;++i){
        State tempState=lr1.statesPro[i];
        // 设置状态ID
        model->setItem(i,0,new QStandardItem(QString::number(i)+":\n"+tempState.display()));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        int k=1;
        for(auto final:finalStr){
            if(lr1.lalrDFA[i].contains(final)){
                int toID=lr1.lalrDFA[i][final];
                model->setItem(i,k,new QStandardItem(QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
        for(auto final:notFinalStr){
            if(lr1.lalrDFA[i].contains(final)){
                int toID=lr1.lalrDFA[i][final];
                model->setItem(i,k,new QStandardItem(QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
    }
    ui->lalr1_DFA->setModel(model);
    ui->lalr1_DFA->resizeRowsToContents();
    ui->lalr1_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::showLALR1AnalyseTable()
{
    QStandardItemModel *model = new QStandardItemModel(ui->lalr1_Table);
    model->clear();
    model->setHorizontalHeaderItem(0,new QStandardItem("状态ID"));

    // 找到所有终结符
    QVector<QString> finalStr;
    for(int i=0;i<grammarSet.grammarCnt;++i){
        QVector<QString> vec=grammarSet.grammarSet[i].right;
        for(auto s:vec){
            if(!grammarSet.notFinalSet.contains(s)) finalStr.append(s);
        }
    }
    // 找到所有非终结符
    QVector<QString> notFinalStr;
    for(auto s:grammarSet.notFinalSet) notFinalStr.append(s);

    // 展示终结符的列名
    int t=1;
    for(auto final:finalStr){
        model->setHorizontalHeaderItem(t,new QStandardItem(final));
        t++;
    }
    model->setHorizontalHeaderItem(t++,new QStandardItem("$"));
    // 展示非终结符的列名
    QBrush redBrush(Qt::red);
    for(auto notFinal:notFinalStr){
        QStandardItem *item = new QStandardItem(notFinal);
        item->setForeground(redBrush);
        model->setHorizontalHeaderItem(t,item);
        t++;
    }
    // 展示分析表
    for(int i=0;i<lr1.stateNumPro;++i){
        State tempState=lr1.statesPro[i];
        // 设置状态ID
        model->setItem(i,0,new QStandardItem(QString::number(i)));
        model->item(i,0)->setTextAlignment(Qt::AlignCenter);
        int k=1;
        bool f=false; // 标记是否有转移
        for(auto final:finalStr){
            if(lr1.lalrDFA[i].contains(final)){
                int toID=lr1.lalrDFA[i][final];
                model->setItem(i,k,new QStandardItem("s"+QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
                f=true;
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
        int t=k++; // 标记“$”所在位置
        for(auto final:notFinalStr){
            if(lr1.lalrDFA[i].contains(final)){
                int toID=lr1.lalrDFA[i][final];
                model->setItem(i,k,new QStandardItem(QString::number(toID)));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
                f=true;
            }
            else{
                model->setItem(i,k,new QStandardItem(" "));
                model->item(i,k)->setTextAlignment(Qt::AlignCenter);
            }
            k++;
        }
        if(!f){
            int grammarID=grammarSet.getGrammarID(tempState.stateVec[0]);
            // 若为第一条规则则accept
            if(grammarID==0){
                model->setItem(i,t,new QStandardItem("Accept"));
                model->item(i,t)->setTextAlignment(Qt::AlignCenter);
            }
            else{
                QSet<QString> tempSet=tempState.stateVec[0].frontSet;
                for(auto s:tempSet){
                    if(s=="$"){
                        model->setItem(i,t,new QStandardItem("r"+QString::number(grammarID)));
                        model->item(i,t)->setTextAlignment(Qt::AlignCenter);
                    }
                    else{
                        int check;
                        for(check=0;check<finalStr.size();++check){
                            if(finalStr[check]==s) break;
                        }
                        model->setItem(i,check,new QStandardItem("r"+QString::number(grammarID)));
                        model->item(i,check)->setTextAlignment(Qt::AlignCenter);
                    }
                }
            }
        }
    }
    ui->lalr1_Table->setModel(model);
    ui->lalr1_Table->resizeRowsToContents();
    ui->lalr1_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::openSourceFile()
{
    qDebug()<<"Exec function: openSourceFile";
    QString filePath = QFileDialog::getOpenFileName(this, "请选择一个文法文件", "", "Text Files(*.txt)");

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(nullptr, "提示", "打开文件失败，请重试！", QMessageBox::Yes);
    }
    QTextStream in(&file);
    content = in.readAll();
    ui->grammarTextEdit->setText(content);
}

void MainWindow::saveSourceFile()
{
    qDebug()<<"Exec function: saveSourceFile";
    QString currentTime = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString grammarTxt = ui->grammarTextEdit->toPlainText();

    QString fileName = "Grammar-" + currentTime + ".txt";
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << grammarTxt;
        file.close();
        QMessageBox::information(nullptr, "提示", "文件保存为 "+fileName+" 成功！", QMessageBox::Yes);
    } else {
        QMessageBox::warning(nullptr, "提示", "文件保存失败！", QMessageBox::Yes);
    }
}

void MainWindow::analyseGrammar()
{
    qDebug()<<"Exec function: analyseGrammar";
    resetUI();
    prework();
    // 非终结符的First集合
    grammarSet.comFirst();
    showFirstResult();
    // 非终结符的Follow集合
    grammarSet.comNotFollow();
    showFollowResult();
    grammarSet.delLeftRecursion();
    // LR(1)的DFA图
    Grammar firstG=grammarSet.grammarSet[0];
    firstG.frontSet.insert("$");
    QVector<Grammar> temp;
    temp.append(firstG);
    State firstState=lr1.CLOSURE(temp,grammarSet);
    // 添加初态
    lr1.states.append(firstState);
    lr1.stateNum++;
    qDebug()<<"LR1初态完成建立";
    // 生成LR1整个DFA
    lr1.buildDFA(firstState,grammarSet);
    showLR1DFA();
    // LALR(1)的DFA图
    lr1.mergeLRDFA();
    showLALR1DFA();
    // LALR(1)的分析表
    int res=lr1.isExistConflict();
    if(res==1) QMessageBox::warning(nullptr, "提示", "LALR1的DFA存在归约-归约冲突，不属于LALR1文法！", QMessageBox::Yes);
    else if(res==2) QMessageBox::warning(nullptr, "提示", "LALR1的DFA存在移进-归约冲突，不属于LALR1文法！", QMessageBox::Yes);
    else{
        showLALR1AnalyseTable();
    }
}
