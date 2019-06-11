#pragma once
#ifndef CFG2LRDFA_H
#define CFG2LRDFA_H

#include "structs.h"
#include "First.h"
#include <queue>
using namespace std;

extern int boundT, boundN; // char+tokens：0-boundT，非终结符:boundT+1-boundN
extern map<int, pair<int, int>> indexMap;
//映射：symbol->producerVec的下标。key为symbol，value为<以该symbol为左部的产生式在producerVec中的起始index，数量>
extern ProducerVec producerVec; //int版本，<产生式左部，右部>
extern LRDFA lrdfa;             //LR(1)DFA

//求LR(1)闭包
void LR1_closure(set<LRItem> &lrItemSet);
//求后继项目集
void subset_construct(const set<LRItem> &lrItemSet, map<int, set<LRItem>> &newStateMap);

//求LR(1)闭包
//params:
//lrItemSet, LR项目集Ii
void LR1_closure(set<LRItem> &lrItemSet)
{
    queue<LRItem> q1;
    //新建空队列，并将状态I的所有产生式加入队列
    for (const auto &lrItem : lrItemSet)
    {
        q1.push(lrItem);
    }
    
    pair<int, vector<int>> producer; //要处理的产生式,<产生式左部，右部>
    int position, symbol;            //点的位置，点后的符号
    int predictiveSymbol;            //预测符
    set<int> predictiveSymbolSet;    //预测符集合
    
    while (!q1.empty())
    {
        //取出头部LR项r
        producer = producerVec[q1.front().Label];
        position = q1.front().Dotpos;
        //如果点在末尾则跳过该产生式
        if (position == producer.second.size())
        {
            q1.pop();
            continue;
        }
        else
        {
            symbol = producer.second[position];//取出·后的symbol
            //如果是终结符则不需要扩展
            if (symbol <= boundT)
            {
                q1.pop();
                continue;
            }
            //否则，先找到所有symbol对应的产生式
            auto index = indexMap.find(symbol)->second; //pair<int,int>
            predictiveSymbol = q1.front().Symbol;
            q1.pop();
            //对于每个产生式都新建LRItem r'
            for (int i = 0; i < index.second; i++)
            {
                LRItem newItem;
                newItem.Label = i + index.first;
                
                //求follow(symbol)=first(nextString)
                vector<int> nextString;
                for (int j = position + 1; j < producer.second.size(); j++)
                {
                    nextString.push_back(producer.second[j]);
                }
                //新建一个预测符集合
                predictiveSymbolSet.clear();
                //first(nextString)即为新的预测符集合
                first_string(predictiveSymbolSet, nextString);
                //如果新的预测符集合中包含epsilon,则去掉epsilon再插入producer原有的预测符
                if (predictiveSymbolSet.find(-1) != predictiveSymbolSet.end())
                { //有epsilon
                    predictiveSymbolSet.erase(-1);
                    predictiveSymbolSet.insert(predictiveSymbol);
                }
                //遍历预测符集合中的所有符号
                for (const auto &p : predictiveSymbolSet)
                {
                    //将p设为r'的预测符
                    newItem.Symbol = p;
                    int flag = 0;//r'是否已经存在
                    for (auto &item : lrItemSet)
                    {
                        if (newItem == item)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 1)
                        continue;
                    //将新的LR项加入状态I
                    q1.push(newItem);
                    lrItemSet.insert(newItem);
                }
            }
        }
    }
}

//求后继项目集
//params:
//lrItemSet,LR项目集Ii; newStateMap,<边，后继项目>
void subset_construct(const set<LRItem> &lrItemSet, map<int, set<LRItem>> &newStateMap)
{
    pair<int, vector<int>> producer;
    LRItem newItem;
    int edge; //edge即为点后的符号
    //遍历状态I的所有LRItem
    for (auto &lrItem : lrItemSet)
    {
        producer = producerVec[lrItem.Label];
        //如果点在末尾则跳过
        if (producer.second.size() == lrItem.Dotpos)
            continue;
        edge = producer.second[lrItem.Dotpos];
        newItem = lrItem;
        newItem.Dotpos++; //点右移
        
        auto findIt = newStateMap.find(edge);
        //如果处理过点后的符号s，并入相应的LR项集合中
        if (findIt != newStateMap.end())
        {
            findIt->second.insert(newItem);
        }
        //如果没处理过s，则新建LR项集合后并入
        else
        {
            set<LRItem> newState;
            newState.insert(newItem); //把这个新的加到集合里
            newStateMap.emplace(edge, newState);
        }
    }
}

//根据上下文无关文法构造LR(1)DFA
void cfg2lrdfa()
{
    LRState I0;
    int stateNum = 0;
    //构造S'->·S,$R
    LRItem initProducer;
    initProducer.Symbol = -2; //$R= -2
    initProducer.Label = 1;
    //将S'->·S,$R插入I0
    I0.LRItemsSet.insert(initProducer);
    //对I0求闭包
    LR1_closure(I0.LRItemsSet);
    I0.Num = stateNum++;
    //将I0压入lrdfa
    lrdfa.statesVec.push_back(I0);
    
    queue<int> unhandledStates; //使用队列存放未处理States
    unhandledStates.push(0);
    map<int, set<LRItem>> newStateMap;//<边，后继项目>
    
    while (!unhandledStates.empty())
    {
        newStateMap.clear();
        //取出队列头部状态
        int top = unhandledStates.front();
        unhandledStates.pop();
        //状态间扩展
        subset_construct(lrdfa.statesVec[top].LRItemsSet, newStateMap);
        for (auto &p : newStateMap)
        {
            LR1_closure(p.second); //状态内扩展
            int edgeTo = -1;
            // 检查是否存在相同的状态
            for (auto &s : lrdfa.statesVec)
            {
                //存在则连接
                //相等条件:error1==0 && error2==0
                int error1 = 0, error2 = 0;
                //检验：p包含s
                for (auto &item1 : s.LRItemsSet)
                {
                    int flag1 = 0;//记录当前item是否有相同
                    for (auto &item2 : p.second)
                    {
                        
                        if (item1 == item2)
                        {
                            flag1 = 1;
                            break;
                        }
                    }
                    //有不相等item
                    if (flag1 == 0)
                    {
                        error1 = 1;
                        break;
                    }
                }
                //检验：s包含p
                for (auto &item1 : p.second)
                {
                    int flag2 = 0;//记录当前item是否有相同
                    for (auto &item2 : s.LRItemsSet)
                    {
                        
                        if (item1 == item2)
                        {
                            flag2 = 1;
                            break;
                        }
                    }
                    //有不相等item
                    if (flag2 == 0)
                    {
                        error2 = 1;
                        break;
                    }
                }
                if (error1 == 0 && error2 == 0)
                {
                    edgeTo = s.Num;
                    break;
                }
            }
            if (edgeTo == -1)
            {
                //说明不存在相同状态
                //新建状态
                LRState newState;
                edgeTo = stateNum;
                //先push再修改
                lrdfa.statesVec.push_back(newState);
                unhandledStates.push(stateNum);
                lrdfa.statesVec.back().Num = stateNum++;
                lrdfa.statesVec.back().LRItemsSet = p.second;
            }
            
            //连接Ii->Ij
            lrdfa.statesVec[top].edgesMap.emplace(p.first, edgeTo);
        }
    }
}

#endif
