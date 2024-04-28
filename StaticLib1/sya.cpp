#include "pch.h"
#include "sya.h"
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "lexer.h"
#include "fsm.h"


ShuntingYard::ShuntingYard(std::stringstream& stream): stream { stream }
{
    res = ShuntingYard_alg(stream);
}

ShuntingYard::~ShuntingYard()
{
    //dtor
}

Lexem ShuntingYard::getNextLexem() {
    if (output_count < res.size()) {
        ++output_count;
        return res[output_count - 1];
    }
    return LEX_EMPTY;
}

std::vector<Lexem> ShuntingYard_alg(std::stringstream& stream) {
    std::vector<Lexem> inf_notation = {};
    Lexer lex(stream);
    Lexem l = lex.getNextLexem();
    while (l.first != "end" && l.first != "error") {
        inf_notation.push_back(l);
        l = lex.getNextLexem();
    }
    inf_notation.push_back(l);

    std::vector<Lexem> queue = {};
    std::vector<Lexem> stack = {};
    bool is_last_elem_num = false;
    std::map<std::string, int> priority = { {"opmul", 2 }, { "opplus", 1 }, { "opminus", 1 } };

    for (Lexem& elem : inf_notation) {
        if (elem.first == "num" || elem.first == "id") {
            if (is_last_elem_num) {
                queue.push_back(LEX_ERROR);
                break;
            }
            is_last_elem_num = true;
            queue.push_back(elem);
        }
        else if (priority.find(elem.first) != priority.end()) {
            if (!is_last_elem_num) {
                queue.push_back(LEX_ERROR);
                break;
            }
            while (!stack.empty() && stack[stack.size() - 1].first != "lpar" && priority[stack[stack.size() - 1].first] >= priority[elem.first]) {
                queue.push_back(stack[stack.size() - 1]);
                stack.pop_back();
            }
            stack.push_back(elem);
            is_last_elem_num = false;
        }
        else if (elem.first == "lpar") {
            stack.push_back(elem);
            is_last_elem_num = false;
        }
        else if (elem.first == "rpar") {
            while (!stack.empty() && stack[stack.size() - 1].first != "lpar") {
                queue.push_back(stack[stack.size() - 1]);
                stack.pop_back();
            }
            if (stack.empty() || !is_last_elem_num) {
                queue.push_back(LEX_ERROR);
                break;
            }
            is_last_elem_num = true;
            stack.pop_back();
        }
        else if (elem == LEX_EOF) {
            if (!is_last_elem_num) {
                queue.push_back(LEX_ERROR);
                break;
            }
            while (!stack.empty() && stack[stack.size() - 1].first != "lpar") {
                queue.push_back(stack[stack.size() - 1]);
                stack.pop_back();
            }
            if (stack.empty()) {
                queue.push_back(LEX_EOF);
                break;
            }
            else if (stack[stack.size() - 1].first == "lpar") {
                queue.push_back(LEX_ERROR);
                break;
            }
        }
        else {
            queue.push_back(LEX_ERROR);
            break;
        }
    }

    return queue;
}
