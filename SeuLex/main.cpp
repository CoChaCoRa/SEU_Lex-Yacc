#include "ParseLexFile.h"
#include "ReStandardization.h"
#include "ReToSuffix.h"
#include "SuffixToNfa.h"
#include "NFA2DFA2VEC.h"
#include "Generate.h"
#include "headers.h"
#include <ctime>
using namespace std;

int main()
{
    NFA finalNFA;
    vector<string> P1;
    map<string, string> reMap;
    vector<Rules> rules;
    vector<string> P4;

    clock_t start = clock();
    cout << "Opening file: lex.txt..." << endl;
    read_and_parse_lex(P1, reMap, rules, P4);
    clock_t point1 = clock();
    cout << "...Finish reading!  time = " << (point1 - start) / 1000.0 << " ms\n\nStart standardizing RE..." << endl;

    re_standardize(rules, reMap);
    clock_t point2 = clock();
    cout << "...Finish RE_Standardization!  time = " << (point2 - point1) / 1000.0 << " ms\n\nStart converting to suffix..." << endl;

    re_to_suffix(rules);
    clock_t point3 = clock();
    cout << "...Finish RE_to_Suffix!  time = " << (point3 - point2) / 1000.0 << " ms\n\nStart converting to NFA..." << endl;

    suffix_to_nfa(rules, finalNFA);
    clock_t point4 = clock();
    cout << "...Finish Suffix_to_NFA!  time = " << (point4 - point3) / 1000.0 << " ms\n\nStart converting to DFA..." << endl;

    NFA2DFA2VEC n2d2v(finalNFA);
    n2d2v.nfa2dfa();
    clock_t point5 = clock();
    cout << "...Finish NFA_to_DFA!  time = " << (point5 - point4) / 1000.0 << " ms\n\nStart minimizing DFA..." << endl;

    n2d2v.dfa_minimize();
    clock_t point6 = clock();
    cout << "...Finish DFA_minimization!  time = " << (point6 - point5) / 1000.0 << " ms\n\nStart generating lex.cpp..." << endl;

    n2d2v.dfa2vec(rules);
    auto minidfa = n2d2v.get_mini_dfa();
    auto vecs = n2d2v.get_vecs();
    auto endVec = n2d2v.get_endVec();
    generate_c_code(vecs, endVec, P1, P4, minidfa.startState);
    clock_t point7 = clock();
    cout << "...Finish!  time = " << (point7 - point6) / 1000.0 << " ms" << endl;

    cout << "\nAll tasks completed!  Total time = " << (point7 - start) / CLOCKS_PER_SEC << "s\n"
         << endl;
}
