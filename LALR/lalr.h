#ifndef LALR_H
#define LALR_H

#include "grammarset.h"

class State{
public:
    int grammarSize=0;
    QVector<Grammar> stateVec;

    State(){}
    State(int size,QVector<Grammar> vec):grammarSize(size),stateVec(vec){}

    QString display(){
        QVector<Grammar> gvec=stateVec;
        QString res;
        for(int i=0;i<gvec.size();++i){
            QString s="[";
            s+=gvec[i].left;
            s+=" -> ";
            for(int j=0;j<gvec[i].right.size();++j){
                if(j==gvec[i].mark) s+=".";
                s+=gvec[i].right[j];
            }
            if(gvec[i].mark==gvec[i].right.size()) s+=".";
            s+="    ";
            for(auto look:gvec[i].frontSet) s+=look+" ,";
            s.chop(1);
            s+="]";
            res+=s+'\n';
        }
        return res;
    }

    // 新增时判断是否已有当前核心一样的规则
    int insertGrammar(Grammar g){
        for(int i=0;i<grammarSize;++i){
            if(stateVec[i].isSimilar(g)){
                return i;
            }
        }
        return -1;
    }
    // 重载相等运算符
    bool operator==(const State s) const{
        if(s.grammarSize==grammarSize){
            for(int i=0;i<grammarSize;++i){
                bool flag=false;
                for(int j=0;j<grammarSize;++j){
                    if(stateVec[j]==s.stateVec[i]){
                        flag=true;
                        break;
                    }
                }
                if(!flag) return false;
            }
            return true;
        }
        else return false;
    }
};

class LALR
{
public:
    LALR();

    int stateNum; // LR1的DFA状态数
    QVector<State> states; // LR1的所有状态
    QHash<int,QHash<QString,int>> lrDFA; // LR1的DFA存储

    int stateNumPro; // LALR1的DFA状态数
    QVector<State> statesPro; // LALR1的所有状态
    QHash<int,int> stateMapping; // LR1状态id到LALR1状态id的映射
    QHash<int,QHash<QString,int>> lalrDFA; // LALR1的DFA存储

    // 清空所有存储状态
    void reset();
    // 构造DFA每个状态
    State CLOSURE(QVector<Grammar> vecg,GrammarSet setg);
    // 构造某个状态到某个状态的跳转
    State GOTO(State state,QString s,GrammarSet setg);
    // 获取状态对应ID
    int getStateID(State state);
    // 构造LR1整个DFA
    void buildDFA(State state,GrammarSet setg);

    // 合并LR1的DFA
    void mergeLRDFA();
    // 判断两个状态同心项是否相同
    bool isSimilarState(State s1,State s2);
    // 找到每个状态所处结合的根节点id
    int findRoot(int id,int *p);

    // 判断是否存在归约-归约冲突以及移进-归约冲突
    int isExistConflict();
};

#endif // LALR_H
