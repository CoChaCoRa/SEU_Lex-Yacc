#pragma once
#ifndef LRDFATOLALRDFA_H
#define LRDFATOLALRDFA_H

#include "headers.h"
#include "structs.h"

//合并同心项
void lrdfa_to_lalrdfa()
{
    //同心集合，pair前项set<pair<int,int>>是core，后项set<int>是前项的同心集的状态号
    //pair<int,int>：前者是item中每一行产生式的标号，后者是产生式中的点位置
    vector<pair<set<pair<int, int>>, set<int>>> coreVec;
    //对每个状态求core，然后将core相同的匹配到一起
    for (auto &state : lrdfa.statesVec)
    {
        //当前状态的core
        set<pair<int, int>> stateCore;
        for (auto &lritem : state.LRItemsSet)
        {
            //把state中的每个item的产生式和点位置记录下来加入core
            stateCore.insert(make_pair(lritem.Label, lritem.Dotpos));
        }
        bool exist = false; //判断该状态的core是否已存在于coreVec中
        for (auto &core : coreVec)
        {
            //当前状态的core已存在于coreVec中，则将当前状态的标号加入同心集
            if (core.first == stateCore)
            {
                exist = true;
                core.second.insert(state.Num);
            }
        }
        if (!exist)
        {
            //当前状态的core不存在于coreVec中，则将该状态作为一个新的core
            //同心集中首先加入自己
            set<int> newStateNum;
            newStateNum.insert(state.Num);
            coreVec.push_back(make_pair(stateCore, newStateNum));
        }
    }

    map<int, int> indexMap; //旧、新状态标号映射表
    int count = 0;          //用来记录心的状态标号
    //建立lalrdfa
    for (auto &core : coreVec)
    {
        LRState newState;
        //更新同心集状态的状态号
        newState.Num = count;
        count++;
        //更新同心集状态的动作，将当前core的同心集里的第一个状态的动作作为新状态的动作
        newState.edgesMap = lrdfa.statesVec[*core.second.begin()].edgesMap;
        for (auto &state : core.second)
        {
            //利用每一个state里的item更新同心集状态里lritem
            for (auto &lritem : lrdfa.statesVec[state].LRItemsSet)
                newState.LRItemsSet.insert(lritem);
            //将同心集里的状态标号映射为新的状态标号
            indexMap.emplace(state, newState.Num);
        }
        lalrdfa.statesVec.push_back(newState);
    }

    //利用旧、新状态标号映射表更新lalrdfa里的动作
    for (auto &state : lalrdfa.statesVec)
        for (auto &edge : state.edgesMap)
            //在indexMap里找到旧的状态，然后更新成新的同心集状态
            edge.second = indexMap.find(edge.second)->second;
    //更新初始状态
    lalrdfa.Start = indexMap.find(lrdfa.Start)->second;
}

#endif
