#include "nfa.h"

NFA::NFA()
{
    // NFA初始化
    stateNum=0;
    maxStateNum=100;
    additionalState=20;
    stateMartix.resize(100);
    for(int i=0;i<stateMartix.size();++i){
        stateMartix[i].resize(100);
    }
}

void NFA::restartNFA()
{
    stateNum=0;
    maxStateNum=100;
    identification.clear();
    stateMartix.clear();
    stateMartix.resize(100);
    for(int i=0;i<stateMartix.size();++i){
        stateMartix[i].resize(100);
    }
}

// 添加一个新关系
Edge NFA::addRelation(QString relation){
    if(stateNum+2>=maxStateNum) resizeMartix();
    int s=stateNum,e=stateNum+1;
    stateNum+=2;
    stateMartix[s][e]=relation;
    return Edge(s,e);
}

// 连接运算
Edge NFA::unionNFA(Edge e1,Edge e2){
    stateMartix[e1.e_num][e2.s_num]="eplision";
    return Edge(e1.s_num,e2.e_num);
}

// 或运算
Edge NFA::either(Edge e1,Edge e2){
    if(stateNum+2>=maxStateNum) resizeMartix();
    int s=stateNum,e=stateNum+1;
    stateNum+=2;
    stateMartix[s][e1.s_num]="eplision";
    stateMartix[s][e2.s_num]="eplision";
    stateMartix[e1.e_num][e]="eplision";
    stateMartix[e2.e_num][e]="eplision";
    return Edge(s,e);
}

// 闭包运算
Edge NFA::closure(Edge e){
    if(stateNum+2>=maxStateNum) resizeMartix();
    int s=stateNum,_e=stateNum+1;
    stateNum+=2;
    stateMartix[s][e.s_num]="eplision";
    stateMartix[s][_e]="eplision";
    stateMartix[e.e_num][e.s_num]="eplision";
    stateMartix[e.e_num][_e]="eplision";
    return Edge(s,_e);
}

// 正闭包
Edge NFA::positiveClosure(Edge e){
    return unionNFA(e,closure(e));
}

// 可选运算
Edge NFA::optional(Edge e){
    if(stateNum+2>=maxStateNum) resizeMartix();
    int s=stateNum,_e=stateNum+1;
    stateNum+=2;
    stateMartix[s][e.s_num]="eplision";
    stateMartix[s][_e]="eplision";
    stateMartix[e.e_num][_e]="eplision";
    return Edge(s,_e);
}

// 增加邻接矩阵的大小
void NFA::resizeMartix(){
    QVector<QVector<QString>>originalMatrix=stateMartix;

    maxStateNum+=additionalState;
    stateMartix.resize(maxStateNum);
    for(int i=0;i<maxStateNum;++i){
        stateMartix[i].resize(maxStateNum);
    }

    for (int i = 0; i < maxStateNum-additionalState; ++i) {
        for (int j = 0; j < maxStateNum-additionalState; ++j) {
            stateMartix[i][j] = originalMatrix[i][j];
        }
    }
}

// 求某个集合eplision闭包
void NFA::eplisionClosure(QSet<int>& s){
    bool flag=false;
    while(!flag){
        flag=true;
        QSet<int> tmp;
        for(int i:s){
            for(int j=0;j<stateNum;++j){
                if(stateMartix[i][j]=="eplision"){
                    if(!s.contains(j)){
                        tmp.insert(j);
                        flag=false;
                    }
                }
            }
        }
        s=s.unite(tmp);
    }
}
