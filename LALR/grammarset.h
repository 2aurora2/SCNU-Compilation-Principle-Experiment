#ifndef GRAMMARSET_H
#define GRAMMARSET_H

#include <QString>
#include <QVector>
#include <QSet>
#include <QDebug>

// 产生式
class Grammar{
public:
    QString left;
    QVector<QString> right;
    int mark;
    QSet<QString> frontSet;

    Grammar():mark(0){}

    // 构造文法项的输出
    QString display(){
        QString s;
        s+=left+"->";
        for(int j=0;j<right.size();++j){
            if(j==mark) s+=".";
            s+=right[j];
        }
        s+="    ";
        for(auto look:frontSet) s+=look+",";
        return s;
    }

    // 判断两个文法项是否完全一样
    bool operator==(const Grammar g) const{
        if(left==g.left&&right.size()==g.right.size()&&mark==g.mark&&frontSet==g.frontSet){
            for(int i=0;i<right.size();++i){
                if(right[i]==g.right[i]) continue;
                else return false;
            }
            return true;
        }
        else return false;
    }

    // 判断两个文法项是否核心一样
    bool isSimilar(Grammar g){
        if(left==g.left&&right.size()==g.right.size()&&mark==g.mark){
            for(int i=0;i<right.size();++i){
                if(right[i]==g.right[i]) continue;
                else return false;
            }
            return true;
        }
        else return false;
    }
};

// 文法规则集合
class GrammarSet
{
public:
    GrammarSet();
    void reset();
    int getFirstAllSize();
    void comFirst();
    int getFollowAllSize();
    void comNotFollow();
    int getGrammarID(Grammar g);
    void delLeftRecursion();


    int grammarCnt;  // 文法规则数量
    QString S;  // 开始符号
    QSet<QString> tokens; // 存储终结符和非终结符的集合
    QVector<Grammar>grammarSet;  // 文法规则集合
    QSet<QString> notFinalSet;  // 非终结符集合
    QHash<QString,QSet<QString>> notFirstHash;  // first集合
    QHash<QString,QSet<QString>> notFollowHash;  // 非终结符follow集合
};

#endif // GRAMMARSET_H
