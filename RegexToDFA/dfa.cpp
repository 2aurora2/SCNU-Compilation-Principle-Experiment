#include "dfa.h"

DFA::DFA()
{
    restartDFA();
}

// 初始化DFA
void DFA::restartDFA(){
    stateNum=0;
    M.clear();
    MT.clear();
    G.clear();
    identify.clear();
    endState.clear();
}

// 实现NFA到DFA的转换
void DFA::fromNFA(NFA nfa){
    QSet<int> startEplision;
    startEplision.insert(nfa.startState);
    // 求出始态的eplision闭包
    nfa.eplisionClosure(startEplision);

    stateNum=1;
    M.insert(stateNum,startEplision);
    MT.insert(startEplision,stateNum);
    addEndState(startEplision,nfa.endState);

    QQueue<QSet<int>> que;
    que.push_back(startEplision);
    while(!que.empty()){
        QSet<int> cur=que.front();
        que.pop_front();

        // 遍历每一个非eplision标识符
        for(QString ID:nfa.identification){
            if(ID=="eplision") continue;
            identify.insert(ID);

            QSet<int> tmp;
            for(int i:cur){
                for(int j=0;j<nfa.stateNum;++j){
                    if(nfa.stateMartix[i][j]==ID){
                        tmp.insert(j);
                    }
                }
            }
            // 添加集合中每个元素的eplision闭包
            nfa.eplisionClosure(tmp);
            if(tmp.empty()) continue;
            if(!MT.contains(tmp)){
                que.push_back(tmp);
                stateNum+=1;
                M.insert(stateNum,tmp);
                MT.insert(tmp,stateNum);
                addEndState(tmp,nfa.endState);
            }
            if(!G.contains(MT[cur])){
                G[MT[cur]]=QHash<QString,int>();
            }
            G[MT[cur]].insert(ID,MT[tmp]);
        }
    }
}

// DFA最小化
void DFA::fromDFA(DFA dfa){
    // 找到非终态集合
    QSet<int> notEndState;
    for(int i=1;i<=dfa.stateNum;++i){
        if(!dfa.endState.contains(i)){
            notEndState.insert(i);
        }
    }
    // 创建划分集合队列
    QQueue<QSet<int>> que;
    if(!notEndState.empty()) que.push_back(notEndState);
    if(!dfa.endState.empty()) que.push_back(dfa.endState);

    int lastSetNum=0,cnt=0;
    if(!notEndState.empty()) lastSetNum++;
    if(!dfa.endState.empty()) lastSetNum++;

    while (true) {
        QSet<int> queItem=que.front();
        // 标记当前集合是否被划分
        bool flag=false;
        for(QString identifyItem:dfa.identify){
            // 存放当前弹出集合每个元素能到达的集合
            QSet<QSet<int>> finishSet;
            // 存储<到达的某个集合，对应哪些元素到达>的映射
            QHash<QSet<int>,QSet<int>> setHash;
            for(int i:queItem){
                if(!dfa.G[i].contains(identifyItem)){
                    QSet<int> emptySet;
                    finishSet.insert(emptySet);
                    setHash[emptySet].insert(i);
                }
                else{
                    int j=dfa.G[i][identifyItem];
                    for(QSet<int> k:que){
                        if(k.contains(j)){
                            finishSet.insert(k);
                            setHash[k].insert(i);
                            break;
                        }
                    }
                }
            }
            if(finishSet.size()<=1) continue;
            else{
                for(QSet<int> finishItem:finishSet){
                    que.push_back(setHash[finishItem]);
                }
                flag=true;
                break;
            }
        }

        que.pop_front();
        // 没有被划分重新push进去队列
        if(!flag) que.push_back(queItem);

        cnt++;
        // 扫过一遍队列
        if(lastSetNum==cnt){
            if(lastSetNum==que.size()){
                break;
            }
            lastSetNum=que.size();
            cnt=0;
        }
    }

    // 构造标识符集合
    identify=dfa.identify;

    // 为新的集合进行编号和mapping
    for(QSet<int> queItem:que){
        if(queItem.empty()) continue;
        stateNum++;
        M[stateNum]=queItem;
        MT[queItem]=stateNum;
    }

    // 构造初态和终态
    for(QSet<int> queItem:que){
        if(queItem.contains(1)){
            startState=MT[queItem];
            break;
        }
    }
    for(int end:dfa.endState){
        for(QSet<int> queItem:que){
            if(queItem.contains(end)){
                endState.insert(MT[queItem]);
            }
        }
    }

    // 构造G
    for(int i=1;i<=stateNum;++i){
        int j=*(M[i].begin());
        for(QString identifyItem:identify){
            int k=dfa.G[j][identifyItem];
            for(int l=1;l<=stateNum;++l){
                if(M[l].contains(k)){
                    G[i][identifyItem]=l;
                    break;
                }
            }
        }
    }
}

// 添加DFA的终态集合
void DFA::addEndState(QSet<int> s,int endStateNum){
    if(s.contains(endStateNum)){
        endState.insert(MT[s]);
    }
}
