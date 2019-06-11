//
//
//将规范化的正规表达式转换为后缀形式
//
//
#include "headers.h"
#include "structs.h"

//中缀转成后缀
void re_to_suffix(vector<Rules> &rules)
{
    for (auto &rule : rules)
    {
        string pattern = rule.pattern; //对pattern进行操作
        string temp;                   //用于临时装载正确pattern
        queue<char> q;                 //最终正确的队列
        stack<char> s;                 //操作符栈
        if (pattern.length() == 1)
            continue;
        for (int i = 0; i < pattern.length(); i++)
        {
            char cur = pattern[i];
            //如果遇到左括号，将其放入栈中
            if (cur == '(')
                s.push(cur);
            
            //如果遇到右括号，则将栈元素弹出，将弹出的操作符输出直到遇到左括号为止
            //注意，左括号只弹出并不输出
            else if (cur == ')')
            {
                while (!s.empty() && s.top() != '(')
                {
                    q.push(s.top());
                    s.pop();
                }
                //弹出左括号
                s.pop();
            }
            else if (cur == '|')
            {
                //如果是|$|情况，作为操作符压栈
                if (pattern[i + 1] == '$' && pattern[i + 2] == '|')
                {
                    s.push('$');
                    s.push('|');
                    s.push('|');
                    i = i + 2;
                    continue;
                }
            //如果遇到任何其他的操作符，从栈中弹出元素直到遇到发现更低优先级的元素(或者栈为空)为止
            //弹出完这些元素后，才将遇到的操作符压入到栈中
            //优先级顺序：*>$>|
                while (!s.empty() && (s.top() == '$' || s.top() == '|' || s.top() == '*'))
                {
                    q.push(s.top());
                    s.pop();
                }
                s.push(cur);
            }
            else if (cur == '$')
            {
                //栈顶优先级不低于当前优先级，从栈中弹出这些元素，再将当前操作符压栈
                while (!s.empty() && (s.top() == '$' || s.top() == '*'))
                {
                    q.push(s.top());
                    s.pop();
                }
                s.push(cur);
            }
            else
                //如果遇到操作数，就直接将其输出
                q.push(cur);
        }
        //栈里还有操作符，就冲刷出来
        while (!s.empty())
        {
            q.push(s.top());
            s.pop();
        }
        while (!q.empty())
        {
            temp += q.front();
            q.pop();
        }
        rule.pattern = temp;
    }
}
