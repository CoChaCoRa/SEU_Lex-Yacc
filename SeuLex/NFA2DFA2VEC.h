//
//
//将NFA确定化为DFA，DFA最小化，将DFA存储为数组的形式，用于后续生成.cpp文件
//
//

#pragma once

#include "GlobalData.h"
#include "structs.h"
#include <queue>
using namespace std;

class NFA2DFA2VEC
{
public:
    NFA2DFA2VEC(NFA &nfa);
    void epsilon_closure(set<int> &initStatesSet);
    bool actions_update(const set<int> &nfaStatesInDFAState, int &actions);
    bool subset_construct(const set<int> &originSet, set<int> &constructedSet, const char edge);
    void nfa2dfa();
    bool split_loop();
    void split_sets();
    void dfa_minimize();
    void dfa2vec(vector<Rules> rules);

    NFA get_m_nfa()
    {
        return m_nfa;
    }
    DFA get_tmp_dfa()
    {
        return tmp_dfa;
    }
    DFA get_mini_dfa()
    {
        return mini_dfa;
    }
    vector<pair<int *, int> > get_vecs()
    {
        return vecs;
    }
    vector<Rules> get_endVec()
    {
        return endVec;
    }

private:
    NFA m_nfa;                                //获取到的NFA
    int num_NFAstates;                        //NFA的状态数
    DFA tmp_dfa;                              //由NFA转化成的DFA
    DFA mini_dfa;                             //最小化DFA
    map<int, int> statesSetsMap;    //存储最小化时DFA状态标号和对应集合标号
    int set_count;                            //最小化时DFA集合标号
    vector<set<int> > statesSetsVec; //最小化时DFA集合
    vector<pair<int *, int> > vecs;            //DFA转化成的Vec,item->first为一维数组，item->second为对应的值
    vector<Rules> endVec;                     //存储终态对应Rules
};

NFA2DFA2VEC::NFA2DFA2VEC(NFA &nfa)
{
	m_nfa = nfa;
	num_NFAstates = nfa.statesMap.size();
	set_count = 0;
}

//求特定NFA状态子集的epsilon闭包
//@param:
//nordered_set<int> &initStatesSet，NFA状态子集
void NFA2DFA2VEC::epsilon_closure(set<int> &initStatesSet)
{
	stack<int> stack;
	vector<bool> handledFlagVec(num_NFAstates);

	//将初始集合中所有状态入栈
	for (const auto &s : initStatesSet)
	{
		stack.push(s);
		handledFlagVec[s] = true; //true表示该状态已经处理过
	}

	while (!stack.empty())
	{
		//找到栈顶状态通过@边可以到达的状态
		auto itsPair = m_nfa.statesMap.find(stack.top())->second.EdgeMultiMap.equal_range('@');
		stack.pop();
		auto beginIt = itsPair.first, endIt = itsPair.second;
		while (beginIt != endIt)
		{
			int unhandledItem = (*beginIt).second;
			//如果已经处理过该状态则跳过
			if (handledFlagVec[unhandledItem])
			{
				continue;
			}
			//否则压入栈并将其插入状态集合中
			else
			{
				stack.push(unhandledItem);
				initStatesSet.emplace(unhandledItem);
				++beginIt;
			}
		}
	}
}

//构造子集
//@params:
//set<int>& originSet，初始的NFA状态子集
//set<int>& constructedSet，构造出的NFA状态子集
//char edge，边对应的字符
//@output:true--该集合有发出edge边，false--该集合没有发出edge边
bool NFA2DFA2VEC::subset_construct(const set<int> &originSet, set<int> &constructedSet, const char edge)
{
	bool flag = false; //false表示集合中没有状态发出edge边
	for (const auto &state : originSet)
	{
		//从NFA状态集合中找到所有对应edge字符的边
		auto itsPair = m_nfa.statesMap.find(state)->second.EdgeMultiMap.equal_range(edge);
		auto beginIt = itsPair.first, endIt = itsPair.second;
		//如果该状态没有发出edge边则跳过
		if (beginIt == endIt)
		{
			continue;
		}
		//将边指向的状态编号插入到子集中
		while (beginIt != endIt)
		{
			constructedSet.insert(beginIt->second);
			++beginIt;
		}
		flag = true;
	}
	return flag;
}

//确定终态对应的动作
//@params：
//set<int> &nfaStatesInDFAState，DFA状态包含的NFA状态
//int& actions，DFA终态对应动作
//@output:true--该DFA状态为终态，false--该DFA状态不是终态
bool NFA2DFA2VEC::actions_update(const set<int> &nfaStatesInDFAState, int &actions)
{
	decltype(m_nfa.endStatesMap.find(0)) endStateIt; //要选择的终态
	bool find = false;
	//遍历DFA中所有的NFA状态
	for (const auto &nfaState : nfaStatesInDFAState)
	{
		//判断该NFA状态是不是终态
		auto it = m_nfa.endStatesMap.find(nfaState);
		if (it != m_nfa.endStatesMap.end())
		{
			if (find)
			{
				if (endStateIt->first > it->first)
					endStateIt = it;
			}
			else
			{
				endStateIt = it;
				find = true;
			}
		}
	}
	//如果该终态对应的动作在.l文件中更靠前则该DFA状态对应的动作更新为该动作
	if (find)
	{
		actions = endStateIt->second;
		return true;
	}
	else
		return false;
}

//NFA转化成DFA
void NFA2DFA2VEC::nfa2dfa()
{
	string edgeSet(ALLSET);		  //设置边的全集
	queue<int> unMarkedDFAstates; //用队列存储未处理的DFA状态编号
	int counter = 0;			  //用于给DFA状态编号

	DFAstate state;
	//state初始化为I0
	state.num = counter++;
	state.subSet.insert(m_nfa.startState);
	epsilon_closure(state.subSet);

	tmp_dfa.statesVec.push_back(state);
	unMarkedDFAstates.push(state.num); //将I0压入未处理状态队列

	while (!unMarkedDFAstates.empty())
	{
		//从队列头部取出一个要处理的状态的编号
		int current_num = unMarkedDFAstates.front();
		unMarkedDFAstates.pop();
		for (const char &c : edgeSet)
		{
			set<int> tempSet;
			if (subset_construct(tmp_dfa.statesVec[current_num].subSet, tempSet, c))
			{
				int toStateNum;			  //边指向的状态的编号
				epsilon_closure(tempSet); //对构造出的子集再进行epsilon闭包
				bool same_exist = false;
				//判断该子集是否已经存在
				for (const auto &s : tmp_dfa.statesVec)
				{
					if (s.subSet == tempSet)
					{
						//该子集已经存在
						toStateNum = s.num; //指向vec中已有的状态
						same_exist = true;
						break;
					}
				}
				//不存在，新建一个DFAstate
				if (!same_exist)
				{
					DFAstate newState;
					newState.num = counter++;  //新标号
					toStateNum = newState.num; //指向新建的这个状态
					newState.subSet = tempSet;
					tmp_dfa.statesVec.push_back(newState); //新建状态入状态子集vec
					unMarkedDFAstates.push(toStateNum);	//新状态压入未处理状态栈

					//如果包含一个终态则决定这个dfa终态对应的动作
					int actions;
					if (actions_update(newState.subSet, actions))
					{
						tmp_dfa.endStatesMap.insert(pair<int, int>(newState.num, actions));
					}
				}
				//构造DFA的一条边
				tmp_dfa.statesVec[current_num].EdgeMap.insert(pair<char, int>(c, toStateNum));
			}
		}
	}
}

//循环拆分tmp_dfa的“每一次操作”
bool NFA2DFA2VEC::split_loop()
{
	bool flag = false;		   //是否有集合被拆
	int splitSetNumber = 0;	//被拆分的集合标号
	set<int> newSet; //被拆分出的新集合

	for (int k = 0; k < statesSetsVec.size(); ++k)
	{
		auto &s = statesSetsVec[k];
		if (s.size() == 1)
			continue; //如果集合中只有一个状态则跳过
		else
		{
			auto &standard = tmp_dfa.statesVec[*(s.begin())]; //将其中一个state作为基准
			//遍历基准状态的所有边
			for (const auto &c : ALLSET)
			{
				//检查集合每一个状态
				for (const auto &i : s)
				{
					const auto &state = tmp_dfa.statesVec[i];
					if (state.num == standard.num)
						continue;									//避免“自己和自己比较”
					auto findStateIt = state.EdgeMap.find(c);		//在state中找c边
					auto findStandardIt = standard.EdgeMap.find(c); //在基准state中找c边
					//不等价条件1:其中一个状态无c边而另一个有c边
					if (findStateIt == state.EdgeMap.end() && findStandardIt != standard.EdgeMap.end())
					{
						flag = true;
						newSet.insert(i);
					}
					else if ((findStateIt != state.EdgeMap.end()) && (findStandardIt == standard.EdgeMap.end()))
					{
						flag = true;
						newSet.insert(i);
					}
					//不等价条件2:都有c边，但目的状态不同
					else if ((findStateIt != state.EdgeMap.end()) && (findStandardIt != standard.EdgeMap.end()))
					{
						if (*(statesSetsMap.find(findStandardIt->second)) != *(statesSetsMap.find(findStateIt->second)))
						{
							flag = true;
							newSet.insert(i);
						}
					}
				}
				if (flag)
					break; //每次只分成两个集合
			}
			if (flag)
			{
				splitSetNumber = k;
				break;
			}
		}
	}
	//删除被拆分集合中的被拆分状态
	for (const auto &s : newSet)
	{
		statesSetsVec[splitSetNumber].erase(s);
		statesSetsMap.insert_or_assign(s, set_count);
	}

	if (newSet.size() != 0)
		statesSetsVec.push_back(newSet);
	++set_count;

	return flag;
}

//构造并拆分tmp_dfa的状态集合
void NFA2DFA2VEC::split_sets()
{
	auto &endStatesMap = tmp_dfa.endStatesMap;

	//遍历tmp_dfa，将每个终态单独存入一个集合中，其余非终态放入一个集合
	for (const auto &p : endStatesMap)
	{
		statesSetsVec.push_back(set<int>{p.first});
		statesSetsMap.emplace(p.first, set_count++);
	}

	set<int> tempSet;
	for (const auto &e : tmp_dfa.statesVec)
	{
		if (endStatesMap.find(e.num) == endStatesMap.end())
		{
			tempSet.insert(e.num);
			statesSetsMap.emplace(e.num, set_count);
		}
	}
	statesSetsVec.push_back(tempSet);
	++set_count;

	while (split_loop())
		; //不停从头循环拆分直到不可分为止
}

//DFA最小化
void NFA2DFA2VEC::dfa_minimize()
{
	//拆分
	split_sets();
	//初态标号=tmp_dfa初态所在集合标号
	mini_dfa.startState = statesSetsMap.find(tmp_dfa.startState)->second;
	//终态
	for (const auto &p : tmp_dfa.endStatesMap)
	{
		mini_dfa.endStatesMap.emplace(statesSetsMap.find(p.first)->second, p.second);
	}

	//其他状态
	for (int k = 0; k < statesSetsVec.size(); ++k)
	{
		auto &pivotState = tmp_dfa.statesVec[*statesSetsVec[k].cbegin()];
		DFAstate newState;
		newState.num = k;
		for (const auto &p : pivotState.EdgeMap)
		{
			newState.EdgeMap.emplace(p.first, statesSetsMap.find(p.second)->second);
		}
		mini_dfa.statesVec.push_back(newState);
	}
}

void NFA2DFA2VEC::dfa2vec(vector<Rules> rules)
{
	string allset(ALLSET);
	const int set_size = allset.size();

	//state_row表：index为DFA状态标号，对应的值为行数*宽度。
	const int DFA_size = mini_dfa.statesVec.size();
	int *state_row = new int[DFA_size];
	memset(state_row, 0, DFA_size * sizeof(int));

	//edge_col表：index为char的ascii码，对应的值为列数
	int *edge_col = new int[256];
	memset(edge_col, 0, 256 * sizeof(int));
	//从index=1开始标值，edge'0'为invalid char
	for (int i = 0; i < set_size; ++i)
	{
		edge_col[int(allset[i])] = i + 1;
	}
	vecs.push_back(pair<int *, int>(edge_col, 256));

	//next表：index = state_row[state标号]+edge_col[char的ascii码]，对应的值是跳转状态标号。
	//表的大小为state_row.size()*edge_col.size()
	int sizeOfNext = DFA_size * (set_size + 1);
	int *next = new int[sizeOfNext]; //set从1开始计数
	//初始化-1
	for (int i = 0; i < sizeOfNext; i++)
	{
		next[i] = -1;
	}

	for (int i = 0; i < mini_dfa.statesVec.size(); i++)
	{
		state_row[mini_dfa.statesVec[i].num] = i * (set_size + 1);
		for (auto it = mini_dfa.statesVec[i].EdgeMap.begin(); it != mini_dfa.statesVec[i].EdgeMap.end(); it++)
		{
			next[state_row[mini_dfa.statesVec[i].num] + edge_col[int(it->first)]] = it->second;
		}
	}
	vecs.push_back(pair<int *, int>(state_row, DFA_size));
	vecs.push_back(pair<int *, int>(next, sizeOfNext));

	//accept表：index是终态标号，对应的值是对应的动作。
	int *accept = new int[DFA_size];
	memset(accept, 0, DFA_size * sizeof(int));

	int numOfend = 0;
	for (auto it = mini_dfa.endStatesMap.begin(); it != mini_dfa.endStatesMap.end(); ++it)
	{
		accept[it->first] = numOfend + 1;
		endVec.push_back(rules[it->second]);
		++numOfend;
	}
	vecs.push_back(pair<int *, int>(accept, DFA_size));
}
