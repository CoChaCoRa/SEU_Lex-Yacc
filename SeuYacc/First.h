#pragma once
#ifndef FIRST_H
#define FIRST_H

#include "structs.h"
using namespace std;

/*extern map<int, set<int>> firstMap;      //first集
 extern int boundT, boundN;                  //终结符bound，非终结符bound
 extern map<int, pair<int, int>> indexMap; //key为symbol，value为<以该symbol为左部的产生式在producerVec中的起始index，数量>
 extern ProducerVec producerVec;              //int版本，<产生式左部，右部>*/

//求符号symbol的first集
void first_symbol(set<int> &firstSet, const int &symbol, set<int> &handlingSymbol);
//将集合right并入left
void sets_union(set<int> &left, const set<int> &right);
//求序列symbolsVec的first集
void first_string(set<int> &firstSet, const vector<int> &symbols);
//求first集
void first_operation();

//求符号symbol的first集,递归算法
//params：
//firstSet，first集； symbol，对应symbol； handlingSymbol，实时记录函数激活树路径上出现过的非终结符,防止左递归导致的栈溢出问题
void first_symbol(set<int> &firstSet, const int &symbol, set<int> &handlingSymbol)
{
    //已经求过first集
    if (firstMap.find(symbol) != firstMap.end())
    {
        sets_union(firstSet, firstMap[symbol]);
        return;
    }
    
    if (symbol == -1)
        return; //symbol==epsilon，直接返回
    //symbol是终结符，插入symbol并直接返回
    else if (symbol <= boundT)
    {
        firstSet.insert(symbol);
        return;
    }
    else
    { //symbol是非终结符
        handlingSymbol.insert(symbol);
        
        auto indexPair = indexMap[symbol];
        //遍历所有symbol在左边的产生式
        for (int i = indexPair.first; i < indexPair.first + indexPair.second; ++i)
        {
            auto producer = producerVec[i];
            //symbol->epsilon
            if (producer.second.size() == 1 && producer.second[0] == -1)
            {
                firstSet.insert(-1);
                continue;
            }
            
            int j = 1;
            set<int> tempSet;
            //遍历产生式右部
            while (j <= producer.second.size())
            {
                tempSet.clear();
                //x不能为handlingSymbol,防止左递归
                if (handlingSymbol.find(producer.second[j - 1]) != handlingSymbol.end())
                {
                    break;
                }
                //对于产生式右部的x求first
                first_symbol(tempSet, producer.second[j - 1], handlingSymbol);
                
                //tempSet包含epsilon，但未遍历到最后一项
                if (tempSet.find(-1) != tempSet.end() && j != producer.second.size())
                {
                    tempSet.erase(-1); //删除epsilon
                    sets_union(firstSet, tempSet);
                    ++j; //求下一个x的first
                    continue;
                }
                //tempSet不包含epsilon，直接并入
                if (tempSet.find(-1) == tempSet.end())
                {
                    sets_union(firstSet, tempSet);
                    break;
                }
                //tempSet包含epsilon，且遍历到最后一项
                if (tempSet.find(-1) != tempSet.end() && j == producer.second.size())
                {
                    firstSet.insert(-1);
                    break;
                }
            }
        }
    }
}

//将集合right并入left
//params:
//left,first集； right,tempset
void sets_union(set<int> &left, const set<int> &right)
{
    left.insert(right.cbegin(), right.cend());
}

//求序列symbolsVec的first集
//params：
//firstSet，first集； symbolsVec，序列；
void first_string(set<int> &firstSet, const vector<int> &symbolsVec)
{
    int i = 1;
    //序列为空
    if (symbolsVec.size() == 0)
    {
        firstSet.insert(-1);
        return;
    }
    while (i <= symbolsVec.size())
    {
        auto &tempRef = firstMap[symbolsVec[i - 1]];
        
        //tempRef包含epsilon，但未遍历到最后一项
        if (tempRef.find(-1) != tempRef.end() && i != symbolsVec.size())
        {
            tempRef.erase(-1); //删除epsilon
            sets_union(firstSet, tempRef);
            ++i; //求下一个x的first
            continue;
        }
        //tempRef不包含epsilon，直接并入
        if (tempRef.find(-1) == tempRef.end())
        {
            sets_union(firstSet, tempRef);
            break;
        }
        //tempRef包含epsilon，且遍历到最后一项
        if (tempRef.find(-1) != tempRef.end() && i == symbolsVec.size())
        {
            firstSet.insert(-1);
            break;
        }
    }
}

//求first集
void first_operation()
{
    set<int> firstSet, stackSet;
    
    for (int symbol = -1; symbol <= boundN; symbol++)
    {
        firstSet.clear();
        stackSet.clear();
        //求symbol的first集
        first_symbol(firstSet, symbol, stackSet);
        //将symbol的first集写入firstMap
        firstMap.emplace(symbol, firstSet);
    }
}

#endif
