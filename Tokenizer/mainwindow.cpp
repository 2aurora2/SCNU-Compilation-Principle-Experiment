#include "mainwindow.h"
#include "tokenizer.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QPair>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建分词器
    tokenizer = new Tokenizer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 读取上传文件
void MainWindow::on_upload_btn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件", "", "文本文件 (*.cpp *.h)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");

            QString content,line;
            line = stream.readLine();
            // 清空上次上传的文件内容
            lines.clear();
            // 清空上次分析结果
            resultMap.clear();
            ui->result->setText("");

            while(!line.isNull()){
                content+=line+'\n';
                if(line.size()>0){
                    lines.push_back(line);
                }
                line = stream.readLine();
            }
            file.close();

            // 显示文件文本
            ui->content->setText(content);
        } else {
            qDebug() << "无法打开文件：" << filePath;
        }
    }
}

// 分词并展示结果
void MainWindow::on_tokenizer_btn_clicked()
{
    for(auto line:lines){
        QVector<QPair<QString, QString>> lineRes = tokenizer->tokenize(line);

        for(auto it=lineRes.begin();it!=lineRes.end();++it){
            // qDebug()<<it->first<<" "<<it->second;
            resultMap.append(*it);
        }
    }
    QString result;
    for(auto it: resultMap){
        result+=it.first+"    "+it.second+"\n";
    }
    // 显示mapping结果
    ui->result->setText(result);
}
