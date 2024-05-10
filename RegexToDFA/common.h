#ifndef COMMON_H
#define COMMON_H

#include <QChar>
#include <QString>
#include <QDebug>

struct Node{
    int flag; // 1为操作数，2为运算符
    QChar ch;
    QString num;
};

struct Edge{
    int s_num,e_num;
    Edge(){

    }
    Edge(int s,int e){
        s_num=s;
        e_num=e;
    }
};

// test
//nat=[0-9]+
//signedNat=(+|-)?nat
//_number=signedNat(\.nat)?(EsignedNat)?

//test
//letter=[a-zA-Z]
//digit=[0-9]
//_ID=letter(letter|digit)*

#endif // COMMON_H
