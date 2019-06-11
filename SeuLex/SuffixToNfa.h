//
//
//将后缀的正规表达式转换为NFA并将NFA合并
//
//

#include "structs.h"
#include <stack>
#include <iostream>
using namespace std;

int num = 0;
void suffix_to_nfa(vector<Rules> &suffixRules, NFA &finalNFA)
{
	stack<NFA> NFAStack;
	for (int i = 0; i < suffixRules.size(); i++)
	{
		string pattern = suffixRules[i].pattern;
		int tempActionInt = -1;
		for (int j = 0; j < pattern.length(); j++)
		{
			NFA upNFA, downNFA;
			NFAstate start, end;
			char cur = pattern[j];
			if (cur == '|' && j != 0 && j != 1)
			{
				//是或|操作，但不属于||'这个表达式
				//取出最上面的NFA进行操作
				upNFA = NFAStack.top();
				NFAStack.pop();
				downNFA = NFAStack.top();
				NFAStack.pop();
				start.num = num;
				++num;
				end.num = num;
				++num;
				//将down的终态和up的终态都连接到end
				upNFA.statesMap.find(upNFA.endStatesMap.begin()->first)->second.EdgeMultiMap.insert(pair<char, int>('@', end.num));
				downNFA.statesMap.find(downNFA.endStatesMap.begin()->first)->second.EdgeMultiMap.insert(pair<char, int>('@', end.num));
				//将start连接到down和up的初态
				start.EdgeMultiMap.insert(pair<char, int>('@', upNFA.startState));
				start.EdgeMultiMap.insert(pair<char, int>('@', downNFA.startState));
				//连接关系定义好后存入map
				downNFA.statesMap.insert(pair<int, NFAstate>(start.num, start));
				downNFA.statesMap.insert(pair<int, NFAstate>(end.num, end));
				//将upNFA的stateMap拷贝到down中
				downNFA.statesMap.insert(upNFA.statesMap.begin(), upNFA.statesMap.end());
				//修改downNFA的初态
				downNFA.startState = start.num;
				//修改downNFA的终态
				downNFA.endStatesMap.clear();
				downNFA.endStatesMap.insert(pair<int, int>(end.num, tempActionInt));
				NFAStack.push(downNFA);
			}
			else if (j - 1 > 0 && j + 1 < pattern.length() && cur == '*' && pattern.length() != 1 && pattern[j - 1] != '/' && pattern[j + 1] != '=')
			{
				//取出最上面的NFA进行操作
				upNFA = NFAStack.top();
				NFAStack.pop();
				//新建两个状态
				start.num = num;
				++num;
				end.num = num;
				++num;
				//将start和upNFA初态连接
				start.EdgeMultiMap.insert(pair<char, int>('@', upNFA.startState));
				//将start和end连接
				start.EdgeMultiMap.insert(pair<char, int>('@', end.num));
				//将upNFA的终态和初态连接
				upNFA.statesMap.find(upNFA.endStatesMap.begin()->first)->second.EdgeMultiMap.insert(pair<char, int>('@', upNFA.startState));
				upNFA.statesMap.find(upNFA.endStatesMap.begin()->first)->second.EdgeMultiMap.insert(pair<char, int>('@', end.num));
				//更改初态
				upNFA.startState = start.num;
				//连接关系定义好后存入map
				upNFA.statesMap.insert(pair<int, NFAstate>(start.num, start));
				upNFA.statesMap.insert(pair<int, NFAstate>(end.num, end));
				//更改终态
				upNFA.endStatesMap.clear();
				upNFA.endStatesMap.insert(pair<int, int>(end.num, tempActionInt));
				NFAStack.push(upNFA);
			}
			else if (cur == '$')
			{
				//取出栈顶的两个NFA
				upNFA = NFAStack.top();
				NFAStack.pop();
				downNFA = NFAStack.top();
				NFAStack.pop();
				//取出downNFA的终态
				downNFA.statesMap.find(downNFA.endStatesMap.begin()->first)->second //得到终态
					//将压在下面的（也就是更前面的）NFA用@与后面的进行连接
					.EdgeMultiMap.insert(pair<char, int>('@', upNFA.startState));
				//更改终态
				downNFA.endStatesMap.clear();
				downNFA.endStatesMap = upNFA.endStatesMap;
				//把upNFA的状态map拷贝到downNFA中，
				downNFA.statesMap.insert(upNFA.statesMap.begin(), upNFA.statesMap.end());
				NFAStack.push(downNFA);
			}
			else
			{
				//遇到字符，构造一个NFA，压栈
				NFA pushNFA; //用于压栈的NFA
				//新建一个起始状态
				start.num = num;
				//更改初态
				pushNFA.startState = start.num;
				++num;
				//新建一个终止状态
				end.num = num;
				++num;
				//将起始状态和终止状态连接
				start.EdgeMultiMap.insert(pair<char, int>(cur, end.num));
				//连接关系定义好后存入map
				pushNFA.statesMap.insert(pair<int, NFAstate>(start.num, start));
				pushNFA.statesMap.insert(pair<int, NFAstate>(end.num, end));
				//标识当前终态，因此先传入空vector。
				pushNFA.endStatesMap.insert(pair<int, int>(end.num, tempActionInt));
				//压栈
				NFAStack.push(pushNFA);
			}
		}
		//将action赋给栈顶的NFA的终态
		NFAStack.top().endStatesMap.begin()->second = i;
	}
	//现在得到了装着所有NFA的nfaStack,合并为一个大的NFA。
	finalNFA = NFAStack.top();
	NFA downNFA;
	NFAStack.pop();
	while (!NFAStack.empty())
	{
		NFAstate start;
		//依次，把栈顶NFA与大NFA合并
		downNFA = NFAStack.top();
		NFAStack.pop();
		//新建一个start状态
		start.num = num;
		num++;
		//将start连接到finalNFA和dowNFA的初态
		start.EdgeMultiMap.insert(pair<char, int>('@', finalNFA.startState));
		start.EdgeMultiMap.insert(pair<char, int>('@', downNFA.startState));
		//修改finalNFA的起始状态
		finalNFA.startState = start.num;
		finalNFA.statesMap.insert(pair<int, NFAstate>(start.num, start));
		//添加finalNFA的终止状态
		finalNFA.endStatesMap.insert(downNFA.endStatesMap.begin(), downNFA.endStatesMap.end());
		//把downNFA的状态map拷贝到finalNFA中
		finalNFA.statesMap.insert(downNFA.statesMap.begin(), downNFA.statesMap.end());
	}
}
