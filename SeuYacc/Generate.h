#pragma once
#ifndef GENERATE_H
#define GENERATE_H

#include "headers.h"
#include "structs.h"
#include <fstream>
#include <iomanip>
using namespace std;

//在yacc.cpp中打印所需要的数组
void print_array(string name, int size, void *value, ofstream &out);
//为了yacc.cpp，将tokensMap转换成两个数组（因为之前写的时候生成文件是.c，不能用map）
void createTokenMap(string *tokensMapStr, int *tokensMapNum);

void generate()
{
    string *tokensMapStr = new string[tokensMap.size()];
    int *tokensMapNum = new int[tokensMap.size()];
    ofstream out;

    //生成y.tab.cpp
    out.open("/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/YaccRun/y.tab.cpp", ios::out);
    if (!out)
    {
        cout << "ERROR: can't open file y.tab.cpp !" << endl;
    }
    out << "#include <stdio.h>" << endl;
    out << "#include <stdlib.h>" << endl;
    out << "#include <string.h>" << endl;
    out << "#include <fstream>" << endl;
    out << "#include <assert.h>" << endl;
    out << "using namespace std;" << endl;

    out << "#define TERMINATED_SYMBOL " << boundT << endl;
    out << "#define boundN " << boundN << endl;
    for (const auto &s : funcVec)
    {
        out << s << endl;
    }

    out << "int *read_tokens(string filename);" << endl;
    //栈定义部分
    //栈中结点
    out << "struct Node" << endl;
    out << "{" << endl;
    out << "    int data;" << endl;
    out << "    struct Node* next;" << endl;
    out << "};" << endl;
    //堆栈定义
    out << "struct Stack" << endl;
    out << "{" << endl;
    out << "    struct Node* head;" << endl;
    out << "    int size;" << endl;
    out << "};" << endl;
    //堆栈初始化
    out << "void StackInit(struct Stack* stack)" << endl;
    out << "{" << endl;
    out << "	stack->head = NULL;" << endl;   //附加头结点
    out << "	stack->size = 0;" << endl;
    out << "}" << endl;
    
    out << "void StackPush(struct Stack* stack, const int data)" << endl;
    out << "{" << endl;
    out << "    struct Node* node;" << endl;
    out << "    node = (struct Node*)malloc(sizeof(struct Node));" << endl;
    out << "    assert(node != NULL);" << endl;
    out << "    node->data = data;" << endl;
    out << "    node->next = stack->head;" << endl;
    out << "    stack->head = node;" << endl;
    out << "    ++stack->size;" << endl;
    out << "}" << endl;

    //判断堆栈是否为空
    out << "int StackEmpty(struct Stack* stack)" << endl;
    out << "{" << endl;
    out << "    return (stack->size == 0);" << endl;
    out << "}" << endl;

    //弹出栈顶元素，data为上一个栈顶元素的值
    out << "int StackPop(struct Stack *stack, int *data)" << endl;
    out << "{" << endl;
    out << "    if (StackEmpty(stack))" << endl;
    out << "        return 0;" << endl;
    out << "	struct Node* tmp = stack->head;" << endl;
    out << "	*data = stack->head->data;" << endl;
    out << "	stack->head = stack->head->next;" << endl;
    out << "	free(tmp);" << endl;
    out << "    --stack->size;" << endl;
    out << "	return 1;" << endl;
    out << "}" << endl;

    //返回栈顶元素
    out << "int StackTop(struct Stack* stack, int* data)" << endl;
    out << "{" << endl;
    out << "	if (StackEmpty(stack))" << endl;
    out << "	{" << endl;
    out << "		return 0;" << endl;
    out << "	}" << endl;
    out << "	*data = stack->head->data;" << endl;
    out << "	return 1;" << endl;
    out << "}" << endl;


    print_array("yy_next", table_vec[0].second, table_vec[0].first, out);
    print_array("yy_producer_data", table_vec[1].second, table_vec[1].first, out);
    print_array("yy_index", table_vec[2].second, table_vec[2].first, out);
    createTokenMap(tokensMapStr, tokensMapNum);
    table_vec.push_back(pair<string *, int>(tokensMapStr, tokensMap.size()));
    table_vec.push_back(pair<int *, int>(tokensMapNum, tokensMap.size()));
    print_array("tokensMapNum", table_vec[4].second, table_vec[4].first, out);

    //对字符串类型数组单独处理打印部分
    out << "static char* yy_productions[]=" << endl;
    out << "{" << endl;
    int i = 0;
    for (const auto &e : productionsVec)
    {
        if(e == "simple_expression -> \" IDENTIFIER \"")
        {
            out << "\"simple_expression -> \\\" IDENTIFIER \\\"\"" << ", ";
            ++i;
            continue;
        }
        out << "\"" << e << "\""
            << ", ";
        ++i;
        if (i % 5 == 0)
        {
            out << endl;
        }
        if (i == productionsVec.size() - 1)
            break;
    }
    //最后一个元素没有逗号
    out << "\"" << productionsVec[i] << "\"" << endl
        << "};" << endl;

    out << "static string tokensMapStr[]=" << endl;
    out << "{" << endl;
    int last = table_vec[3].second - 1;
    for (int i = 0; i < table_vec[3].second; i++)
    {
        if (i == table_vec[3].second - 1)
            break;
        if(((string *)table_vec[3].first)[i] == "\"")
        {
            out << "\"\\\"\"" << ", ";
            continue;
        }
        out << "\"" << ((string *)table_vec[3].first)[i] << "\""
            << ", ";
        if (i % 5 == 0)
        {
            out << endl;
        }
    }
    //最后一个元素没有逗号
    out << "\"" << ((string *)table_vec[3].first)[last] << "\"" << endl
        << "};" << endl;

    //main()函数部分
    out << "int main(int argc,char** argv)" << endl;
    out << "{" << endl;
    out << "	ofstream printfile(\"/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/YaccRun/syntax_analysis_result.txt\");" << endl;
    out << "    int *tokens;" << endl;
    out << "	if(argc==2)" << endl;
    out << "	{" << endl;
    out << "		tokens = read_tokens(argv[1]);" << endl;
    out << "	}" << endl;
    out << "	else" << endl;
    out << "    {" << endl;
    out << "		printf(\"ERROR: invalid argument!\\n\");" << endl;
    out << "		return -1;" << endl;
    out << "	}" << endl;

    //创建两个栈，状态栈和符号栈
    out << "	struct Stack stateStack;" << endl;
    out << "	struct Stack symbolStack;" << endl;
    out << "	StackInit(&stateStack);" << endl;
    //初始状态压入状态栈
    out << "    StackPush(&stateStack," << lalrdfa.Start << ");" << endl;
    out << "	StackInit(&symbolStack);" << endl;
    //#压入符号栈，将第一个非终结符的标号作为#
    out << "    StackPush(&symbolStack, TERMINATED_SYMBOL + 1);" << endl;

    out << "	int ctoken=0;" << endl;         //存放当前读取的token
    out << "	int item=0;" << endl;           //存放转移表的表项
    out << "	int producerStart=0;" << endl;  //存放产生式开始坐标
    out << "	int producerLength=0;" << endl; //存放产生式的长度

    //开始读取token进行语法分析
    out << "    int i = 0;" << endl;
    out << "    ctoken = tokens[i];" << endl;
    out << "    do" << endl;
    out << "    {" << endl;
    out << "        int top;" << endl;                     //记录状态栈顶元素
    out << "        StackTop(&stateStack, &top);" << endl; //提取栈顶元素
    //第一步，查表，看转移结果是移进还是归约
    out << "        item = yy_next[top * (boundN + 2) + ctoken];" << endl;

    //如果是移进，则将下一个转移状态和当前字符分别压栈
    out << "		if(item>0)" << endl;
    out << "		{" << endl;
    out << "			StackPush(&stateStack,item);" << endl;
    out << "			StackPush(&symbolStack,ctoken);" << endl;
    //读取下一个token
    out << "			i++;" << endl;
    out << "			ctoken = tokens[i];" << endl;
    out << "			continue;" << endl;
    out << "		}" << endl;

    //如果是归约
    out << "		else if(item<0)" << endl;
    out << "		{" << endl;
    out << "			if(item == -1){" << endl; //规约式为S’->S则表示语法分析成果
    out << "				printf(\"finish!\\n\");" << endl;
    out << "				printfile<<\"finish!\\n\";" << endl;
    out << "				break;" << endl;
    out << "			}" << endl;
    //找到归约所用的产生式，对两个栈进行pop
    //因为item对应的产生式是从1开始编号的，而存储产生式左部位置和右部长度的数组是从位置0开始的
    //所以要用(-item)-1来获取产生式
    out << "			producerStart=yy_index[2*(-item-1)];" << endl;
    out << "			producerLength=yy_index[2*(-item-1)+1];" << endl;
    out << "			int stackTopItem=0;" << endl; //存储上一个被pop掉的元素
    //弹出产生式右部长度个数量的状态和符号
    out << "			for(int j=0;j<producerLength;j++)" << endl;
    out << "			{" << endl;

    out << "				if(StackPop(&stateStack,&stackTopItem)==0)" << endl;
    out << "				{" << endl;
    out << "					printf(\"ERROR:STACK EMPTY!\");" << endl;
    out << "					printfile<<\"ERROR:STACK EMPTY!\\n\";" << endl;
    out << "					return -1;" << endl;
    out << "				}" << endl;

    out << "				if(StackPop(&symbolStack,&stackTopItem)==0)" << endl;
    out << "				{" << endl;
    out << "					printf(\"ERROR:STACK EMPTY!\");" << endl;
    out << "					printfile<<\"ERROR:STACK EMPTY!\\n\";" << endl;
    out << "					return -1;" << endl;
    out << "				}" << endl;

    out << "			}" << endl;
    //因为yy_productions没有第一条空白产生式和第二条S'->S
    out << "            printf(\"%s\\n\", yy_productions[-item-2]);" << endl;
    out << "            printfile<<yy_productions[-item-2]<<\"\\n\";" << endl;
    //将产生式左侧的符号压入符号栈以及GOTO(当前栈顶状态，产生式左侧符号)压入状态栈
    out << "			StackPush(&symbolStack,yy_producer_data[producerStart]);" << endl;
    out << "			int top;" << endl;
    out << "			StackTop(&stateStack, &top);" << endl;
    //因为GOTO在next表中是在#列之后，所以查表的时候要额外+1
    out << "			item = yy_next[top * (boundN + 2) + yy_producer_data[producerStart] + 1];" << endl;
    out << "			StackPush(&stateStack,item);" << endl;
    out << "		}" << endl;
    out << "		else" << endl;
    out << "        {" << endl;
    out << "			printf(\"ERROR:SYNTAX ERROR!\");" << endl;
    out << "			printfile<<\"ERROR:SYNTAX ERROR!\\n\";" << endl;
    out << "			return -1;" << endl;
    out << "		}" << endl;
    out << "	}while(1);" << endl;
    out << "    return 0;" << endl;
    out << "}" << endl;

    out << "int *read_tokens(string filename)" << endl;
    out << "{" << endl;
    //首先记录tokens个数
    out << "    int countToken=0;" << endl;
    out << "    ifstream in(filename);" << endl;
    out << "    while (!in.eof())" << endl;
    out << "    {" << endl;
    out << "        string temp;" << endl;
    out << "        in >> temp;" << endl;
    out << "        if(temp==\"SINGLE_LINE_COMMENT\"||temp==\"MULTI_LINE_COMMENT\")" << endl;
    out << "              continue;" << endl;
    out << "        countToken++;" << endl;
    out << "    }" << endl;
    //记录tokens
    out << "    int *tokens = new int[countToken+1];" << endl;
    out << "    ifstream in1(filename);" << endl;
    out << "    int i = 0;" << endl;
    out << "    while (!in1.eof())" << endl;
    out << "    {" << endl;
    out << "        string temp;" << endl;
    out << "        in1 >> temp;" << endl;
    out << "        if(temp==\"SINGLE_LINE_COMMENT\"||temp==\"MULTI_LINE_COMMENT\")" << endl;
    out << "            continue;" << endl;
    out << "        for(int j=0;j<sizeof(tokensMapNum)/sizeof(int);j++)" << endl;
    out << "        {" << endl;
    out << "            if(temp==tokensMapStr[j])" << endl;
    out << "            {" << endl;
    out << "                  tokens[i]=tokensMapNum[j];" << endl;
    out << "                  break;" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "        i++;" << endl;
    out << "    }" << endl;
    out << "    tokens[countToken]=TERMINATED_SYMBOL+1;" << endl;
    out << "    return tokens;" << endl;
    out << "}" << endl;
}

void print_array(string name, int size, void *value, ofstream &out)
{
    int *array_buf = (int *)value;

    if (name == "yy_index")
    {
        out << "static int	" << name << "[" << size - 2 << "]"
            << " =" << endl;
    }
    else
        out << "static int	" << name << "[" << size << "]"
            << " =" << endl;
    out << "{	" << endl;
    for (int i = 0; i < size; i++)
    {
        //因为第一条产生式是为了将label从1开始而设置的空白产生式，即-1 -> ()
        //而有效的产生式位置记录应该是从第二条S'->S产生式开始的，对应归约产生式标号item=-1
        //所以在yy_index中将第一条空白产生式删去，也就是yy_index记录的第一条是S'->S
        //producer_data表是根据producerData建立的
        //而yy_index也是根据producerData表建立的，所以producer_data表中不能将空白产生式删去
        //如果删去，而yy_index中的位置记录还是根据原表建立的，则位置记录就会错位
        if (name == "yy_index" && (i == 0 || i == 1))
            continue;
        out << setw(4) << left << array_buf[i];
        if (i != size - 1)
        {
            out << ",";
        }
        if (i % 10 == 0)
        {
            out << endl;
        }
    }
    out << "};" << endl;
}

void createTokenMap(string *tokensMapStr, int *tokensMapNum)
{
    int i = 0;
    for (auto &pair : tokensMap)
    {
        tokensMapStr[i] = pair.first;
        tokensMapNum[i] = pair.second;
        i++;
    }
}

#endif
