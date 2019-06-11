#pragma once
#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "headers.h"
#include "structs.h"

//将所有的token转换成正整数
//char + tokens: 0-boundT
//非终结符: boundT+1-boundN

void translate_expression()
{
    int count = 0;

    //翻译字符
    for (auto &producer : producerStrVec)
    {
        //在所有产生式的右部中找字符
        for (auto &item : producer.second)
            if (item.size() == 3 && item[0] == '\'' && item[2] == '\'')
            {
                item = item[1];
                //如果在tokensMap中已经找到了这个字符，就不再将其插入tokensMap
                //如果map中没有要查找的数据，它返回的迭代器等于end函数返回的迭代器
                if (tokensMap.find(item) != tokensMap.end())
                    continue;
                //在tokensMap中加入映射关系
                //emplace使用参数自动构造tokensMap内部对象
                //如果有重复的key值则自动舍弃
                tokensMap.emplace(item, count);
                count++;
            }
    }

    //翻译token
    for (auto &token : tokensVec)
    {
        tokensMap.emplace(token, count);
        count++;
    }
    //终结符的结束位置
    boundT = count - 1;

    //翻译非终结符
    for (auto &producer : producerStrVec)
    {
        if (tokensMap.find(producer.first) != tokensMap.end())
            continue;
        tokensMap.emplace(producer.first, count);
        count++;
    }
    //非终结符的结束位置
    boundN = count - 1;

    //构建左、右结合运算符优先级列表
    for (auto &s : precedenceRulesStrVec)
    {
        //precedenceRulesStrVec：vector<pair<int, vector<string>>>
        vector<int> newSet;
        //将该条规则中的每个token翻译成数字
        for (auto &e : s.second)
            newSet.push_back(tokensMap.find(e)->second);
        precedenceRulesVec.push_back(make_pair(s.first, newSet));
    }

    //为了产生式标号从1开始，先在产生式vector中加入一条空白产生式
    producerVec.push_back(pair<int, vector<int>>(-1, vector<int>()));
    //S‘->S产生式，产生式右部为第一个非终结符
    producerVec.push_back(pair<int, vector<int>>(-1, vector<int>({boundT + 1})));

    //翻译产生式
    int preleftInt = boundT + 1, counter = 0;
    int temp;                 //记录当前翻译的产生式左部
    vector<int> tempRightVec; //记录当前翻译的产生式右部
    for (auto &producer : producerStrVec)
    {
        //翻译产生式右部
        tempRightVec.clear();
        for (auto &item : producer.second)
            tempRightVec.push_back(tokensMap[item]);

        //翻译产生式左部
        temp = tokensMap[producer.first];
        //如果是新的产生式左部
        if (temp != preleftInt)
        {
            //记录上一个产生式左部，以及在producerVec中的起始位置和长度
            indexMap.emplace(preleftInt, pair<int, int>(producerVec.size() - counter, counter));
            //为记录下一个产生式而将counter置零
            counter = 0;
            //更新当前产生式左部
            preleftInt = temp;
        }
        //将当前产生式加入producerVec
        producerVec.push_back(pair<int, vector<int>>(temp, tempRightVec));
        //同一个非终结符的产生式数量++
        counter++;
    }
    //记录最后一条产生式左部，以及在producerVec中的起始位置和长度
    indexMap.emplace(preleftInt, pair<int, int>(producerVec.size() - counter, counter));
}

#endif
