#include "grammarset.h"

GrammarSet::GrammarSet()
{

}

void GrammarSet::reset()
{
    grammarCnt=0;
    grammarSet.clear();
    notFinalSet.clear();
    notFirstHash.clear();
    notFollowHash.clear();
}

int GrammarSet::getFirstAllSize()
{
    int size=0;
    for(auto s:tokens) size+=notFirstHash[s].size();
    return size;
}

void GrammarSet::comFirst()
{
    for(auto s:tokens){
        notFirstHash[s]=QSet<QString>();
        if(!notFinalSet.contains(s)) notFirstHash[s].insert(s);
    }
    int lastSize=-1,newSize=getFirstAllSize();
    while (lastSize!=newSize) {
        for(int i=0;i<grammarCnt;++i){
            bool blankStr=true;
            int j=0;
            QString notF=grammarSet[i].left;
            while(blankStr&&j<grammarSet[i].right.size()){
                QSet<QString> tmpSet=notFirstHash[grammarSet[i].right[j]];
                notFirstHash[notF].unite(tmpSet);
                if(!tmpSet.contains("@")) blankStr=false;
                j++;
            }
        }
        lastSize=newSize;
        newSize=getFirstAllSize();
    }
}

int GrammarSet::getFollowAllSize()
{
    int size=0;
    for(auto s:notFinalSet) size+=notFollowHash[s].size();
    return size;
}

void GrammarSet::comNotFollow()
{
    for(auto notF:notFinalSet){
        notFollowHash[notF]=QSet<QString>();
    }
    notFollowHash[S].insert("$");
    int lastsize=-1,newsize=getFollowAllSize();
    while(lastsize!=newsize){
        for(int i=0;i<grammarCnt;++i){
            for(int j=0;j<grammarSet[i].right.size();++j){
                if(notFinalSet.contains(grammarSet[i].right[j])){
                    // 非终结符位于规则最右部
                    if(j==grammarSet[i].right.size()-1){
                        notFollowHash[grammarSet[i].right[j]].unite(notFollowHash[grammarSet[i].left]);
                    }
                    // 非终结符不位于规则最右部
                    else{
                        bool blankStr=true;
                        int k=j+1;
                        QSet<QString> tmpSet;
                        QSet<QString> emptySet;
                        emptySet.insert("@");
                        while (blankStr&&k<grammarSet[i].right.size()) {
                            QSet<QString> tmpFirst=notFirstHash[grammarSet[i].right[k]];
                            if(!tmpFirst.contains("@")) blankStr=false;
                            tmpSet.unite(tmpFirst);
                            k++;
                        }
                        QSet<QString> tmpSetPro=tmpSet.subtract(emptySet);
                        notFollowHash[grammarSet[i].right[j]].unite(tmpSetPro);
                        if(tmpSet.contains("@")){
                                notFollowHash[grammarSet[i].right[j]].unite(notFollowHash[grammarSet[i].left]);
                        }
                    }
                }
            }
        }
        lastsize=newsize;
        newsize=getFollowAllSize();
    }
}

int GrammarSet::getGrammarID(Grammar g)
{
    for(int i=0;i<grammarCnt;++i){
        if(grammarSet[i].left==g.left&&grammarSet[i].right.size()==g.right.size()){
            bool flag=true;
            for(int j=0;j<g.right.size();++j){
                if(g.right[j]==grammarSet[i].right[j]) continue;
                else{
                    flag=false;
                    break;
                }
            }
            if(flag) return i;
        }
    }
    return -1;
}

void GrammarSet::delLeftRecursion()
{
    // 找出存在左递归的非终结符
    QSet<QString> leftReStr;
    for(auto s:notFinalSet)
        for(int j=0;j<grammarSet.size();++j)
            if(grammarSet[j].left==s&&grammarSet[j].right[0]==s){
                leftReStr.insert(s);
                notFinalSet.insert(s+"\'");
                break;
            }
    QVector<Grammar> newGrammarSet;
    // 找出存在左递归的终结符的所有文法规则并修改
    for(int j=0;j<grammarSet.size();++j)
        if(!leftReStr.contains(grammarSet[j].left)) continue;
        else{
            if(grammarSet[j].right[0]!=grammarSet[j].left){
                grammarSet[j].right.append(grammarSet[j].left+"\'");
            }
            else{
                grammarSet[j].left=grammarSet[j].left+"\'";
                grammarSet[j].right.removeFirst();
                grammarSet[j].right.append(grammarSet[j].left+"\'");
            }
        }
    Grammar t;
    for(auto s:leftReStr){
        t.left=s+"\'";
        t.right.clear();
        t.right.append("@");
        grammarSet.append(t);
        grammarCnt++;
    }
}
