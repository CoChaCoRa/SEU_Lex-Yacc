#pragma once
#ifndef PARSEYACCFILE_H
#define PARSEYACCFILE_H

#include "headers.h"
#include "structs.h"

using namespace std;

int read_and_parse_yacc()
{
    string filePath = "/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/SeuYacc/yacc.txt";
    ifstream in;

    in.open(filePath, ios::in);
    if (!in)
    {
        cout << "Failed to open yacc.y!!" << endl;
        exit(1);
    }
    string str = "";
    in >> str;
    //读取所有token
    while (str != "%right" && str != "%left" && str != "%%")
    {
        //读到%token，则再往后读一个字符串
        if (str == "%token")
            in >> str;
        tokensVec.push_back(str);
        in >> str;
    }

    while (str != "%%")
    {
        //读取左结合、右结合规则，newSet用来暂时保存当前读到的符号
        vector<string> newSet;
        if (str == "%left")
        {
            in >> str;
            while (str != "%left" && str != "%right" && str != "%%")
            {
                newSet.push_back(str);
                in >> str;
            }
            //一行规则读完，将该条规则加入precedenceRulesStrVec，0表示左结合
            precedenceRulesStrVec.push_back(make_pair(0, newSet));
            newSet.clear();
        }
        if (str == "%right")
        {
            in >> str;
            while (str != "%left" && str != "%right" && str != "%%")
            {
                newSet.push_back(str);
                in >> str;
            }
            precedenceRulesStrVec.push_back(make_pair(1, newSet));
            newSet.clear();
        }
    }
    if (str == "%%")
    {
        //读取产生式
        in >> str;
        while (str != "%%")
        {
            //p为一条产生式，左边为产生式左部的非终结符，右边为产生式右部各个部分组成的vector
            pair<string, vector<string>> p;
            p.first = str;
            in >> str;
            while (str != ";")
            {
                //读到分号则同一个非终结符的所有产生式结束
                if (str == ":")
                    in >> str;
                if (str == "|")
                {
                    //读到|，一条产生式结束，存入producerStrVec
                    producerStrVec.push_back(p);
                    //只清空存储一条产生式右部的vector，左部的非终结符不变
                    p.second.clear();
                    in >> str;
                }
                while (str != "|" && str != ";")
                {
                    //将一条产生式右部的各个部分存入vector
                    p.second.push_back(str);
                    in >> str;
                }
            }
            //将最后一条产生式存入producerStrVec
            producerStrVec.push_back(p);
            in >> str;
        }
    }
    if (str == "%%")
    {
        //读取程序代码
        while (!in.eof())
        {
            //读取一行字符串，默认分隔符为换行符号
            getline(in, str);
            //除去每一行最后的换行符
            str.erase(str.find_last_not_of('\r') + 1);
            funcVec.push_back(str);
        }
    }

    return 0;
}

#endif
