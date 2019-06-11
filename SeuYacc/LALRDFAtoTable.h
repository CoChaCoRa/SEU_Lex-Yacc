#pragma once
#ifndef LALRDFATOTABLE_H
#define LALRDFATOTABLE_H

#include "headers.h"
#include "structs.h"

void lalrdfa_to_table()
{
    //next：每一行是一个状态的转移动作，通过行号+终结或者非终结符取值得到下一状态
    //移进大于0，规约小于0
    //boundN+1: 终结符+非终结符个数，再加一个#，所以是boundN+2
    int *next = new int[lalrdfa.statesVec.size() * (boundN + 2)];
    //将next中当前位置后面的n个字节用0替换
    memset(next, 0, lalrdfa.statesVec.size() * (boundN + 2) * sizeof(int));
    //producerData: 装所有的产生式的单词形式
    int *producerData = new int[producerVec.size() * 10];
    memset(producerData, 0, producerVec.size() * 10 * sizeof(int));
    //通过产生式标号获取产生式，[2*产生式标号]=首地址，[2*产生式标号+1]=长度(长度不包括左边的那个)，用在producer_data中查找。
    int *index = new int[2 * producerVec.size()];
    memset(index, 0, 2 * producerVec.size() * sizeof(int));
    //反向翻译表，索引为production号，值为string。
    string *productions = new string[boundT + 1];

    //遍历存储所有的产生式，构建producerData表
    int count = 0; //当前存储到哪个位置
    for (int i = 0; i < producerVec.size(); i++)
    {
        producerData[count] = producerVec[i].first;
        //产生式左部的位置
        index[2 * i] = count;
        //产生式右部的长度
        index[2 * i + 1] = producerVec[i].second.size();
        count++;
        //存储产生式右部
        for (int j = 0; j < producerVec[i].second.size(); j++)
        {
            producerData[count] = producerVec[i].second[j];
            count++;
        }
    }
    //构建反向翻译表
    for (int i = 0; i < producerStrVec.size(); ++i)
    {
        string temp_str = producerStrVec[i].first + " ->";
        for (auto it = producerStrVec[i].second.begin(); it != producerStrVec[i].second.end(); it++)
        {
            temp_str += " ";
            temp_str += *it;
        }
        productionsVec.push_back(temp_str);
    }
    //遍历所有的状态，构建next表
    for (int i = 0; i < lalrdfa.statesVec.size(); i++)
    {
        //使用state.Num *(boundN+2) 作为行号
        //这样对每个状态处理就是对每一行进行处理

        //首先处理移进的情况
        for (auto &action : lalrdfa.statesVec[i].edgesMap)
        {
            if (action.first <= boundT)
                //如果是终结符，则在#列之前完成
                next[lalrdfa.statesVec[i].Num * (boundN + 2) + action.first] = action.second;
            else
                //如果是非终结符，则在#列之后完成
                next[lalrdfa.statesVec[i].Num * (boundN + 2) + action.first + 1] = action.second;
        }

        //处理归约的情况，对一个状态中的每个产生式进行操作
        for (auto &item : lalrdfa.statesVec[i].LRItemsSet)
        {
            //如果是点在最后的item，则会有归约
            if (item.Dotpos == producerVec[item.Label].second.size())
            {
                //取出预测分析符，预测分析符都是终结符
                int predictiveSymbol = item.Symbol;
                //处理#的情况
                if (predictiveSymbol == -2)
                    predictiveSymbol = boundT + 1; //找到#列
                //解决移进归约冲突,有移进动作
                if (next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol] > 0)
                {
                    int last;                           //产生式中最后一个终结符
                    int predPrior = -1, lastPrior = -1; //predPrior记录预测符的优先级，lastPrior记录每个item最后一个终结符的优先级
                    for (int m = producerVec[item.Label].second.size() - 1; m >= 0; m--)
                    {
                        //从最后开始找，找到该产生式中最后一个终结符
                        if (producerVec[item.Label].second[m] <= boundT)
                        {
                            last = producerVec[item.Label].second[m];
                            break;
                        }
                    }
                    //在优先级规则表中定位最后一个终结符和预测符的优先级顺序
                    for (int p = 0; p < precedenceRulesVec.size(); p++)
                    {
                        //每一条都形如：pair<int, vector<string>>
                        //前项表示左结合/右结合，后一项是具体符号集合
                        for (int q = 0; q < precedenceRulesVec[p].second.size(); q++)
                        {
                            //找到预测符
                            if (item.Symbol == precedenceRulesVec[p].second[q])
                            {
                                //行数代表优先级，数字越大优先级越高
                                predPrior = p; //记录预测符优先级
                            }
                            //找到最后一个终结符
                            if (last == precedenceRulesVec[p].second[q])
                            {
                                //行数代表优先级，数字越大优先级越高
                                lastPrior = p; //记录最后一个终结符的优先级
                            }
                        }
                    }
                    //如果有优先级规则，就按照规则，否则就做移进
                    if (predPrior != -1 && lastPrior != -1)
                    {
                        //最后一个终结符优先级高，则归约，否则移进
                        if (lastPrior >= predPrior)
                            next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol] = -item.Label;
                    }
                }
                //解决归约归约冲突，已有归约选项
                else if (next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol] < 0)
                {
                    //比较产生式优先级，选择优先级高的（号码比较小的）做归约
                    if ((-next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol]) > item.Label)
                    {
                        next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol] = -item.Label;
                    }
                }
                else
                    //没有冲突，则把归约使用的产生式对应的编号填入表中
                    next[lalrdfa.statesVec[i].Num * (boundN + 2) + predictiveSymbol] = -item.Label;
            }
        }
    }

    //装载vec
    table_vec.push_back(pair<int *, int>(next, (boundN + 2) * lalrdfa.statesVec.size()));
    table_vec.push_back(pair<int *, int>(producerData, 10 * producerVec.size()));
    table_vec.push_back(pair<int *, int>(index, 2 * producerVec.size()));
}

#endif
