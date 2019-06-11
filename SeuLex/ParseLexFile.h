//
//
//读入并解析.l文件
//
//

#include "headers.h"
#include "structs.h"
#include "GlobalData.h"
using namespace std;

//字符串分割函数
vector<string> split(const string &str, const string &delim);
//去除字符串的首尾空格
string &trim(string &s);

//读入并解析.l文件
void read_and_parse_lex(vector<string> &P1, map<string, string> &reMap, vector<Rules> &rules, vector<string> &P4)
{
    //读入lex文件
    string filePath = "/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/SeuLex/lex.txt";
    ifstream in;
    in.open(filePath.c_str(), ios::in);
    if (!in)
    {
        cout << "Failed to open lex.txt!" << endl;
        exit(1);
    }
    string line;                //存储每行的字符串
    int state = 0;              //当前读写指针所处部分
    vector<string> splitResult; //暂存字符串分割结果
    bool hasError = false;      //标识是否有错误，有错误则终止解析
    string leftString;          //存储左边的字符串
    string rightString;         //存储右边的字符串
    vector<string> action;      //存储动作
    while (!in.eof() && !hasError)
    {
        getline(in, line);
        //若某行为空，则直接跳过
        if (line.empty())
            continue;
        //根据当前读写指针状态执行相应操作
        switch (state)
        {
        case 0: //当前为初始状态
            if (line.find("%{") == 0)
                state = 1; //进入辅助定义部分
            else
            {
                cout << "ERROR: NO ENTRY SIGH %{" << endl;
                hasError = true;
            }
            break;
        case 1: //当前为辅助定义部分
            if (line.find("%}") == 0)
                state = 2; //进入正规表达式部分
            else
            {
                P1.push_back(line); //将辅助定义部分内的行记录下来
            }
            break;
        case 2: //当前为正规表达式部分
            if (line.find("%%") == 0)
                state = 3; //进入识别规则部分
            else
            {
                //将正规表达式分割成两部分
                splitResult = split(line, " ");
                if (splitResult.size() == 2)
                //将分割结果存入表达式映射表
                    reMap.insert(pair<string, string>(splitResult[0], trim(splitResult[1])));
            }
            break;
        case 3: //当前为识别规则部分
            if (line.find("%%") == 0)
                state = 4; //进入用户子程序部分
            else
            {
                //处理规则左部为字符情况
                if (line[0] == '"')
                {
                    int i = 1;
                    for (i = 1; i < line.size(); i++)
                    {
                        if (line[i] == '"')
                            break;
                    }
                    //将两个引号内的内容作为规则的左部
                    leftString = line.substr(0, i + 1);
                    //特别处理规则左部为"""的情况
                    if (line[0] == '\"' && line[1] == '\"' && line[2] == '\"')
                    {
                        leftString = "\"\"\"";
                        i++; //右部读到第2个引号的时候就停了，所以要往后读一格
                    }
                    rightString = line.substr(i + 1);
                    trim(rightString);
                    //处理右部单挑规则有多个动作的情况
                    if (rightString[0] == '{')
                    {
                        //去掉两边的大括号
                        rightString = rightString.substr(1, rightString.find_first_of('}') - 1);
                        //提取第一个comment动作
                        string str1 = rightString.substr(0, rightString.find_first_of(' '));
                        action.push_back(str1);
                        //提取第二个return动作
                        string str2 = rightString.substr(rightString.find_first_of(' ') + 1);
                        action.push_back(str2);
                    }
                    else
                        action.push_back(rightString);
                }
                //处理规则左部为表达式情况
                else if (line[0] == '{')
                {
                    int i = 1;
                    for (i = 1; i < line.size(); i++)
                    {
                        if (line[i] == '}')
                            break;
                    }
                    leftString = line.substr(0, i + 1);
                    rightString = line.substr(i + 1);
                    trim(rightString);
                    action.push_back(rightString);
                }
                //新建规则
                Rules rule;
                rule.pattern = leftString;
                rule.actions = action;
                //在规则表中加入这条规则
                rules.push_back(rule);
                action.clear();
            }
            break;
        case 4: //当前为用户子程序部分
            P4.push_back(line);
            break;
        }
    }
    in.close();
}

//字符串分割函数
vector<string> split(const string &str, const string &delim)
{
    vector<string> res;
    if ("" == str)
        return res;
    //先将要切割的字符串从string类型转换为char*类型
    char *strs = new char[str.length() + 1]; //不要忘了
    strcpy(strs, str.c_str());
    
    //将切割符转换成char*
    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    //strtok()用来将字符串分割成一个个片段，strs指向欲分割的字符串，d为分割字符串
    char *p = strtok(strs, d);
    while (p)
    {
        //分割结果p不为空的时候
        string s = p;     //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        //继续分割
        //在第一次调用时，strtok()必需给予strs字符串，往后的调用则将strs设置成NULL，每次调用成功则返回下一个分割后的字符串指针
        p = strtok(NULL, d);
    }
    delete[] strs;
    delete[] d;

    return res;
}

//去除字符串的首尾空格
string &trim(string &s)
{
    if (s.empty())
    {
        return s;
    }
    //erase: 删除从pos开始的n个字符
    //找到第一个不是空格的字符，设它的位置为x
    //则所要删除的空格字符位置为x-1，那么erase中所要删除的字符个数就是x
    s.erase(0, s.find_first_not_of(' '));
    //删除制表符
    s.erase(0, s.find_first_not_of('\t'));
    //删除pos处的一个字符
    //找到最后一个不是空格的字符位置，则它后面+1的位置处就是空格
    s.erase(s.find_last_not_of(' ') + 1);
    //删除回车
    s.erase(s.find_last_not_of('\r') + 1);
    //删除回车换行
    s.erase(s.find_last_not_of('\n') + 1);
    //删除制表符
    s.erase(s.find_last_not_of('\t') + 1);
    return s;
}
