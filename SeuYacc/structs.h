#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
#include <map>
#include <set>
using namespace std;

typedef vector<pair<string, vector<string>>> ProducerStrVec; //字符版本，<产生式左部，右部>
typedef vector<pair<int, vector<int>>> ProducerVec;          //int版本，<产生式左部，右部>

//存储LR(1)文法产生式标号以及此时点的位置的结构体
struct LRItem
{
    int Dotpos = 0; //点的位置
    int Label = -1; //产生式标号
    int Symbol;     //预测符，为方便后续操作，产生式和预测符设为一对一关系
    bool operator==(const LRItem &rLRItem) const
    {
        if (Dotpos != rLRItem.Dotpos)
            return false;
        else if (Label != rLRItem.Label)
            return false;
        else if (Symbol != rLRItem.Symbol)
            return false;
        else
            return true;
    }
    bool operator<(const LRItem &rLRItem) const
    {
        return Label < rLRItem.Label || Dotpos < rLRItem.Dotpos || Symbol < rLRItem.Symbol;
    }
};

//存储LR(1)文法状态的结构体
struct LRState
{
    int Num = -1;           //状态号
    map<int, int> edgesMap; //<终结符，目标状态号>
    set<LRItem> LRItemsSet; //存储文法状态中的产生式
};

//存储LR（1）DFA的结构体
struct LRDFA
{
    int Start = 0;             //起始状态
    vector<LRState> statesVec; //存储所有的LRState
};

//全局变量
string start;
// char + tokens：0-boundT,非终结符:boundT+1-boundN
// startInt 初始状态号
int boundT, boundN, startInt;

//映射：symbol->producerVec的下标。key为symbol，value为<以该symbol为左部的产生式在producerVec中的起始index，数量>
map<int, pair<int, int>> indexMap;

ProducerVec producerVec;       //int版本产生式，用于编号下标寻址
ProducerStrVec producerStrVec; //string版本产生式

//映射：symbol->first集，<symbol,first集>
map<int, set<int>> firstMap;

//前面0表示左，1表示右，后面是规则，字符串版本
vector<pair<int, vector<string>>> precedenceRulesStrVec;
//前面0表示左，1表示右，后面是规则
vector<pair<int, vector<int>>> precedenceRulesVec;

LRDFA lrdfa;
LRDFA lalrdfa;

//存储所有的token
vector<string> tokensVec;
//存储每一行用户程序
vector<string> funcVec;
//存储每一条产生式
vector<string> productionsVec;
//映射：tokens -> 标号，<tokens,count>
map<string, int> tokensMap;

//void*为存储的字符，int为大小
vector<pair<void *, int>> table_vec;

#endif
