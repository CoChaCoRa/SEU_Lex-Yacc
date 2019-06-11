#include "Translate.h"
#include "First.h"
#include "CFG2LRDFA.h"
#include "LRDFAtoLALRDFA.h"
#include "LALRDFAtoTable.h"
#include "Generate.h"
#include "ParseYaccFile.h"
#include <ctime>
using namespace std;

int main()
{
    clock_t start = clock();
    
    cout<<"\nOpening File: yacc.c ..."<<endl;
    read_and_parse_yacc();
    clock_t point1 = clock();
    cout<<"...Finish reading!  time = "<<(point1-start)/1000.0<<" ms\n\nStart translating all tokens to integers..."<<endl;
    
    translate_expression();
    clock_t point2 = clock();
    cout << "...Translation finished!  time = "<<(point2-point1)/1000.0<<" ms\n\nStart First-Operation..." << endl;
    
    first_operation();
    clock_t point3 = clock();
    cout<<"...First-Operation finished!  time = "<<(point3-point2)/1000.0<<" ms\n\nStart converting to LR(1) DFA..."<<endl;
    
    cfg2lrdfa();
    clock_t point4 = clock();
    cout<<"...LR(1) DFA constructed!  time = "<<(point4-point3)/1000.0<<" ms\n\nStart converting to LALR DFA..."<<endl;
    
    lrdfa_to_lalrdfa();
    clock_t point5 = clock();
    cout<<"...LALR DFA constructed!  time = "<<(point5-point4)/1000.0<<" ms\n\nStart generating y.tab.cpp..."<<endl;
    
    lalrdfa_to_table();
    generate();
    clock_t point6 = clock();
    cout<<"...Completed!  time = "<<(point6-start)/1000.0<<" ms"<<endl;
    
    cout<<"\nAll tasks completed!  Total time = "<<(point6-start)/CLOCKS_PER_SEC<<"s\n"<<endl;
    return 0;
}
