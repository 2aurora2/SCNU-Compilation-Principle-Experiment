#ifndef NFA_H
#define NFA_H

#include "common.h"
#include <QDebug>
#include <QVector>
#include <QString>


class NFA
{
public:
    NFA();

    int stateNum;  // 状态数
    int maxStateNum;    // 目前最多状态数
    int startState;  // 起始态的状态编号
    int endState;   // 终态的状态编号
    QVector<Node> vec;
    QSet<QString> identification;  // 所有标识符
    QVector<QVector<QString>> stateMartix;

    void restartNFA();

    Edge addRelation(QString relation);
    Edge unionNFA(Edge e1,Edge e2);
    Edge either(Edge e1,Edge e2);
    Edge closure(Edge e);
    Edge positiveClosure(Edge e);
    Edge optional(Edge e);

    void eplisionClosure(QSet<int>& s);

private:
    int additionalState;  // 状态数满后添加的状态数

    void resizeMartix();
};

#endif // NFA_H
