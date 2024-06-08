#include "lalr.h"

LALR::LALR()
{

}

void LALR::reset()
{
    stateNum=0;
    states.clear();
    lrDFA.clear();
    stateNumPro=0;
    statesPro.clear();
    stateMapping.clear();
    lalrDFA.clear();
}

State LALR::CLOSURE(QVector<Grammar> vecg,GrammarSet setg){
    State J(vecg.size(), vecg);
    bool flag = true;
    while(flag){
        flag = false;
        // 循环遍历已有状态的所有文法规则
        for(int i = 0; i < J.stateVec.size(); ++i){
            if(J.stateVec[i].mark == J.stateVec[i].right.size()) continue;
            QString temp = J.stateVec[i].right[J.stateVec[i].mark];
            // 找到当前mark后面的某个非终结符
            if(setg.notFinalSet.contains(temp)){
                for(int j = 0; j < setg.grammarCnt; ++j){
                    // 找到该非终结符的所有文法规则
                    if(setg.grammarSet[j].left == temp){
                        Grammar newG = setg.grammarSet[j];
                        // 转移过来的文法下一个mark到最后或者为"@"直接搬下来
                        if(J.stateVec[i].mark + 1 == J.stateVec[i].right.size()||J.stateVec[i].right[J.stateVec[i].mark+1]=="@"){
                            newG.frontSet = J.stateVec[i].frontSet;
                        }
                        // 下一个字符为终结符则front为终结符
                        else if(!setg.notFinalSet.contains(J.stateVec[i].right[J.stateVec[i].mark+1])){
                            newG.frontSet.insert(J.stateVec[i].right[J.stateVec[i].mark+1]);
                        }
                        // 若下一个字符为非终结符则求FIRST(后续所有字符)
                        else{
                            bool blankstr = true;
                            int k = J.stateVec[i].mark + 1;
                            QSet<QString> tempFirst;
                            while(blankstr && k < J.stateVec[i].right.size()){
                                 QSet<QString> tempSet = setg.notFirstHash[J.stateVec[i].right[k]];
                                 if(!tempSet.contains("@")) blankstr = false;
                                 tempFirst.unite(tempSet);
                                 k++;
                            }
                            newG.frontSet = tempFirst;
                        }
                        int tid=J.insertGrammar(newG);
                        if(tid==-1){
                            J.stateVec.append(newG);
                            J.grammarSize++;
                            flag=true;
                        }
                        else{
                            if(J.stateVec[tid].frontSet!=newG.frontSet){
                                J.stateVec[tid].frontSet.unite(newG.frontSet);
                                flag=true;
                            }
                        }
                    }
                }
            }
        }
    }
    return J;
}

State LALR::GOTO(State state, QString s,GrammarSet setg)
{
    QVector<Grammar> vec;
    for(int i=0;i<state.stateVec.size();++i){
        if(state.stateVec[i].mark>=state.stateVec[i].right.size()) continue;
        if(state.stateVec[i].right[state.stateVec[i].mark] == s){
            Grammar temp = state.stateVec[i];
            temp.mark++;
            vec.append(temp);
        }
    }
    return CLOSURE(vec,setg);
}

int LALR::getStateID(State state)
{
    for(int i=0;i<stateNum;++i){
        if(states[i]==state)
            return i;
    }
    return -1;
}

void LALR::buildDFA(State state,GrammarSet setg)
{
    QVector<QString> vec;
    // 创建当前状态到其他状态的HASH
    lrDFA[getStateID(state)]=QHash<QString,int>();
    for(int i=0;i<state.stateVec.size();++i){
        if(state.stateVec[i].mark<state.stateVec[i].right.size()){
            QString tempstr=state.stateVec[i].right[state.stateVec[i].mark];
            if(!vec.contains(tempstr)) vec.append(tempstr);
        }
    }
    for(auto str:vec){
        qDebug()<<"当前状态:";
        state.display();
        qDebug()<<"通过"<<str<<"转移到";
        State tempState=GOTO(state,str,setg);
        tempState.display();
        qDebug()<<"========";
        bool flag=true;
        for(int i=0;i<stateNum;++i){
            if(states[i]==tempState){
                flag=false;
                break;
            }
        }
        int toID;
        // 不存在该状态则新建再深搜
        if(flag){
            toID=stateNum++;
            states.append(tempState);
            toID=getStateID(tempState);
            lrDFA[getStateID(state)][str]=toID;
            buildDFA(tempState,setg);
        }
        else{
            toID=getStateID(tempState);
            lrDFA[getStateID(state)][str]=toID;
        }
    }
}

void LALR::mergeLRDFA()
{
    // 合并前每个状态单独一个集合
    int *p=new int[stateNum];
    for(int i=0;i<stateNum;++i) p[i]=i;
    // 找到同心项进行合并
    for(int i=0;i<stateNum;++i){
        for(int j=0;j<stateNum;++j){
            if(i==j) continue;
            if(isSimilarState(states[i],states[j])){
                int rooti=findRoot(i,p);
                int rootj=findRoot(j,p);
                if(rooti!=rootj){
                    p[rootj]=rooti;
                }
            }
        }
    }
    // 建立每个根节点id到新状态的id的映射
    QHash<int,int> mp;
    for(int i=0;i<stateNum;++i){
        int root=p[i];
        if(!mp.contains(root)){
            mp[root]=stateNumPro++;
        }
    }
    // 建立LR1状态到LALR1状态的映射
    for(int i=0;i<stateNum;++i){
        int root=p[i];
        stateMapping[i]=mp[root];
    }
    // 合并同心项
    for(int i=0;i<stateNumPro;++i){
        QVector<int> tempVec;
        for(int j=0;j<stateNum;++j)
            if(stateMapping[j]==i)
                tempVec.append(j);
        State tempState=states[tempVec[0]];
        for(int j=1;j<tempVec.size();++j){
            State t=states[tempVec[j]];
            for(int m=0;m<tempState.grammarSize;++m){
                for(int n=0;n<t.grammarSize;++n){
                    if(tempState.stateVec[m].isSimilar(t.stateVec[n])){
                        tempState.stateVec[m].frontSet.unite(t.stateVec[n].frontSet);
                    }
                }
            }
        }
        // 插入新状态
        statesPro.append(tempState);
    }
    // 建立新状态之间的转移
    for(int i=0;i<stateNumPro;++i) lalrDFA[i]=QHash<QString,int>();
    for(int i=0;i<stateNum;++i){
        int hashID=stateMapping[i];
        QHash<QString,int> hashEdge=lrDFA[i];
        for(auto s:hashEdge.keys()){
            int tempID=hashEdge[s];
            lalrDFA[hashID][s]=stateMapping[tempID];
        }
    }
}

bool LALR::isSimilarState(State s1, State s2)
{
    if(s1.grammarSize==s2.grammarSize){
        for(int i=0;i<s1.grammarSize;++i){
            if(s1.stateVec[i].isSimilar(s2.stateVec[i])) continue;
            else return false;
        }
        return true;
    }
    else return false;
}

int LALR::findRoot(int id, int *p)
{
    if(p[id]!=id){
        p[id]=findRoot(p[id],p);
    }
    return p[id];
}

int LALR::isExistConflict()
{
    // 归约-归约冲突判断，存在返回1
    for(int i=0;i<stateNumPro;++i){
        State s=statesPro[i];
        for(int j=0;j<s.grammarSize;++j){
            for(int k=0;k<s.grammarSize;++k){
                if(j==k) continue;
                Grammar gj=s.stateVec[j];
                Grammar gk=s.stateVec[k];
                if(gj.mark==gj.right.size()&&gk.mark==gk.right.size()&&gj.frontSet.intersects(gk.frontSet)){
                    return 1;
                }
            }
        }
    }
    // 移进-归约冲突判断，存在返回2
    for(int i=0;i<stateNumPro;++i){
        QSet<QString> front;
        QSet<QString> back;
        State s=statesPro[i];
        for(int j=0;j<s.grammarSize;++j){
            if(s.stateVec[j].mark==s.stateVec[j].right.size()){
                for(auto s:s.stateVec[j].frontSet) back.insert(s);
            }
            else {
                front.insert(s.stateVec[j].right[s.stateVec[j].mark]);
            }
        }
        if(front.intersects(back)){
            qDebug()<<"存在移进-归约冲突";
            return 2;
        }
    }
    return 0;
}












