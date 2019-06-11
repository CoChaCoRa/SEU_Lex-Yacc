//
//
//将DFA数组和其他辅助函数写入.c文件
//
//

#include <fstream>
#include <iomanip>
#include "structs.h"
using namespace std;

//打印数组，name为数组名，size为数组大小，value为数组值，out为写入的文件流
static void print_vector(string name, int size, const int *value, ofstream &out);

//生成.cpp文件
int generate_c_code(vector<pair<int *, int>> &vecs, vector<Rules> &endVec, vector<string> &part1, vector<string> &part4, int startState)
{
    
    ofstream out;
    out.open("/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/LexRun/lex.cpp", ios::out);
    //首先判断size的大小是否等于4，不等则报错
    if (vecs.size() != 4)
        return -1;
    //开始生成
    //头部文件
    out << "#include <stdlib.h>" << endl;
    out << "#include <string.h>" << endl;
    out << "#include <fstream>" << endl;
    out << "using namespace std;" << endl;
    out << "#define START_STATE " << startState << endl;
    //输出P1
    for (int i = 0; i < part1.size(); i++)
    {
        out << part1[i] << endl;
    }
    //getCharPtr--读取文件，返回字符指针
    //findAction--根据对应序号返回相应动作，用switch-case实现
    out << "char* getCharPtr(char* fileName);" << endl;
    out << "string findAction(int action);" << endl;
    
    //输出edge_col表,state_row表,next表,accept表
    vector<string> vec_name;
    vec_name.push_back(string("yy_edge_col"));
    vec_name.push_back(string("yy_state_row"));
    vec_name.push_back(string("yy_next"));
    vec_name.push_back(string("yy_accept"));
    
    for (int i = 0; i < 4; i++)
    {
        print_vector(vec_name[i], vecs[i].second, vecs[i].first, out);
    }
    
    //定义若干变量
    out << " int yy_current_state = START_STATE;" << endl;//当前状态号
    out << " int yy_last_accepting_state = -1;" << endl;//最近一次接受的状态
    out << " char *yy_cp = NULL;" << endl;//字符指针
    out << " char *yy_last_accepting_cpos = NULL;" << endl;//最近一次接受时字符指针的位置
    out << " int yy_act = 0;" << endl;//动作
    out << " int isEnd=0;" << endl;//是否读到文件尾部
    out << " int yy_c=-1;" << endl;//字符指针指向字符的ascii码
    out << " int correct=1;" << endl;//是否正确
    
    //一些初始化动作
    out << "void lex_init(char* fileName)" << endl;
    out << "{" << endl;
    //调用char* getCharPtr(char* fileName)得到文件字符指针
    out << "    yy_cp=getCharPtr(fileName);" << endl;
    out << "}" << endl;
    //主函数
    out << "int main(int argc,char** argv)" << endl;
    out << "{" << endl;
    //读取文件
    out << "    if(argc==2)" << endl;
    out << "        lex_init(argv[1]);" << endl;
    
    out << "    else" << endl;
    out << "    {" << endl;
    out << "        printf(\" ERROR : invalid argument ! \");" << endl;
    out << "        return -1;" << endl;
    out << "    }" << endl;
    
    out << "    if (isEnd&&correct)" << endl;
    out << "        return -1;" << endl;
    out << "    else if (isEnd && !correct)" << endl;
    out << "        return -2;" << endl;
    
    out << "    ofstream printfile(\"/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/LexRun/print_tokens.txt\");" << endl;
    out << "    ofstream tokens(\"/Users/gutingxuan/Desktop/SeuLex\&SeuYacc/LexRun/tokens.txt\");" << endl;
    out << "    string result;" << endl;
    //1.去字符yy_c(ascii码）
    //2.如果yy_current_state是终态
    //  yy_last_accepting_state = yy_current_state;yy_last_accepting_cpos = yy_cp;
    //3.在next表中找不到下一状态，且yy_last_accepting_state有效
    //  1）指针回退，状态号回退
    //  2）根据yy_last_accepting_state调用findAction
    //  3）yy_current_state = yy_last_accepting_state;yy_cp = yy_last_accepting_cpos;
    //     在next表中寻找下一状态
    //  4）++yy_cp
    //4.找不到下一状态，且yy_last_accepting_state无效->ERROR
    //5.找得到下一状态->继续跳转
    //6.读到终点
    //  1）yy_cunrrent_state是终态->findAction
    //  2）否则->ERROR
    out << "    while (*yy_cp != 0) " << endl;
    out << "    {" << endl;
    out << "        if(*yy_cp=='\\n')" << endl;
    out << "        {" << endl;
    out << "            yy_cp++;" << endl;
    out << "            continue;" << endl;
    out << "        }" << endl;
    out << "        yy_c = yy_edge_col[(int)*yy_cp];" << endl;
    out << "        if (yy_accept[yy_current_state])" << endl;
    out << "        {" << endl;
    out << "            yy_last_accepting_state = yy_current_state;" << endl;
    out << "            yy_last_accepting_cpos = yy_cp;" << endl;
    out << "        }" << endl;
    out << "        if (yy_next[yy_state_row[yy_current_state] + yy_c] == -1 && yy_last_accepting_state != -1)" << endl;
    out << "        {" << endl;
    out << "            yy_current_state = yy_last_accepting_state;" << endl;
    out << "            yy_cp = yy_last_accepting_cpos;" << endl;
    out << "            yy_act = yy_accept[yy_current_state];" << endl;
    out << "            result = findAction(yy_act);" << endl;
    out << "            printfile<<result;" << endl;
    out << "            if(result!=\" \")" << endl;
    out << "            {" << endl;
    out << "                tokens<<result;" << endl;
    out << "                printf(\"%s\\n\", result.c_str());" << endl;
    out << "                tokens<<\"\\n\"; " << endl;
    out << "            }" << endl;
    out << "            if(*(yy_cp-1)=='\\n')" << endl;
    out << "            {" << endl;
    out << "                printfile<<*(yy_cp-1);" << endl;
    //out << "                tokens<<*(yy_cp-1);" << endl;
    out << "            }" << endl;
    out << "            yy_current_state = START_STATE;" << endl;
    out << "            yy_last_accepting_state = -1;" << endl;
    out << "            yy_c = yy_edge_col[(int)*yy_cp];" << endl;
    out << "            yy_current_state = yy_next[yy_state_row[yy_current_state] + yy_c];" << endl;
    out << "            ++yy_cp;" << endl;
    out << "            continue;" << endl;
    
    out << "        }" << endl;
    out << "        if (yy_next[yy_state_row[yy_current_state] + yy_c] == -1 && yy_last_accepting_state == -1)" << endl;
    out << "        {" << endl;
    out << "            printf(\"ERROR DETECTED IN INPUT FILE !\");" << endl;
    out << "            return -1;" << endl;
    out << "        }" << endl;
    
    out << "        if (yy_next[yy_state_row[yy_current_state] + yy_c] != -1) " << endl;
    out << "        {" << endl;
    out << "            yy_current_state = yy_next[yy_state_row[yy_current_state] + yy_c];" << endl;
    out << "            ++yy_cp;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    
    out << "    if (*yy_cp == 0)" << endl;
    out << "    {" << endl;
    out << "        isEnd = 1;" << endl;
    //out << "        if (yy_accept[yy_current_state] && yy_cp == yy_last_accepting_cpos + 1)" << endl;
    out << "        if (yy_accept[yy_current_state])" << endl;
    out << "        {" << endl;
    out << "            yy_act = yy_accept[yy_current_state];" << endl;
    out << "            result = findAction(yy_act);" << endl;
    out << "            printfile<<result;" << endl;
    out << "            if(result!=\" \")" << endl;
    out << "                tokens<<result;" << endl;
    out << "        }" << endl;
    
    out << "        else " << endl;
    out << "        {" << endl;
    out << "            printf(\"ERROR DETECTED IN INPUT FILE !\");" << endl;
    out << "            correct = 0;" << endl;
    out << "            return -1;" << endl;
    out << "        }" << endl;
    
    out << "    }" << endl;
    out << "    return 0;" << endl;
    out << "}" << endl;
    //lex_main函数结束
    
    //string findAction(int action)
    out << "string findAction(int action)" << endl;
    out << "{" << endl;
    out << "switch (action) " << endl; //打印switch语句
    out << "{" << endl;
    out << "case 0:" << endl;
    out << "break;" << endl;
    for (int i = 0; i < endVec.size(); i++)
    {
        out << "case " << i + 1 << ":" << endl;
        for (int j = 0; j < endVec[i].actions.size(); j++)
        {
            out << endVec[i].actions[j] << endl;
        }
        out << "break;" << endl;
    }
    out << "default:" << endl;
    out << "break;" << endl;
    out << "}" << endl; //int findAction(int state）函数的下括号
    out << "return \"DONE\";" << endl;
    out << "}" << endl; //int findAction(int action)函数的下括号
    //char* getCharPtr(char* fileName)函数
    out << "char* getCharPtr(char* fileName)" << endl;
    out << "{" << endl;
    out << "    char* cp=NULL;" << endl;
    out << "    FILE *fp;" << endl;
    out << "    fp=fopen(fileName,\"r\");" << endl;
    out << "    if(fp==NULL)" << endl;
    out << "    {" << endl;
    out << "        printf(\"can't open file\");" << endl;
    out << "        exit(0);" << endl;
    out << "    }" << endl;
    out << "    fseek(fp,0,SEEK_END);" << endl;
    out << "    int flen = ftell(fp);" << endl;             //得到文件大小
    out << "    cp = (char *)malloc(flen + 1);" << endl; //根据文件大小动态分配内存空间
    out << "    if (cp == NULL)" << endl;
    out << "    {" << endl;
    out << "        fclose(fp);" << endl;
    out << "            return 0;" << endl;
    out << "    }" << endl;
    out << "    rewind(fp);" << endl; //定位到文件开头
    out << "    memset(cp,0,flen+1);" << endl;
    out << "    fread(cp, sizeof(char), flen, fp);" << endl; //一次性读取全部文件内容
    out << "    cp[flen] = 0; " << endl;                     //字符串结束标志:0
    out << "    return cp;" << endl;
    out << "}" << endl;
    //输出P4
    for (int i = 0; i < part4.size(); i++)
    {
        out << part4[i] << endl;
    }
    
    out.close();
    return 0;
}
void print_vector(string name, int size, const int *value, ofstream &out)
{
    
    const int *array_buf = value;
    out << "static int    " << name << "[" << size << "]"
    << " =" << endl;
    out << "    {    0," << endl;
    for (int i = 1; i < size; i++)
    {
        
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
