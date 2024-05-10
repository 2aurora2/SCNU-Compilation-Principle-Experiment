#ifndef DFA_H
#define DFA_H

#include "nfa.h"

#include <QHash>
#include <QSet>
#include <QQueue>

class DFA
{
public:
    DFA();
    int stateNum;   // 状态数
    int startState;
    QHash<int,QSet<int>> M;   // DFA状态对应NFA状态的集合
    QHash<QSet<int>,int> MT; // NFA状态集合对应DFA状态编号
    QHash<int,QHash<QString,int>> G;   // DFA状态的转换
    QSet<int> endState;  // DFA终态集合
    QSet<QString> identify; // DFA标识符集合

    void restartDFA();
    void fromNFA(NFA nfa);
    void fromDFA(DFA dfa);
    void addEndState(QSet<int> s,int endStateNum);
};

#endif // DFA_H
