#include "pch.h"
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <fstream>
#include "grammar.h"


Rule::Rule(std::string LeftTerm, std::vector<std::string> RightTerms) {
        Left = LeftTerm;
        Right = RightTerms;
        sizeRight = Right.size();
    };

Rule::~Rule() {
        //dtor
    };

std::string Rule::getLeft() {
        return Left;
    };

std::vector<std::string> Rule::getRight() {
        return Right;
    };

int Rule::getRightSize() {
        return sizeRight;
    };
bool Rule::operator==(Rule const& rhs) const {
    return this->Left == rhs.Left && this->Right == rhs.Right;
}

bool isNonTerminal(std::string elem) {
    if (std::find(nonterms.begin(), nonterms.end(), elem) != nonterms.end()) {
        return true;
    }
    return false;
};

bool isTerminal(std::string elem) {
    if (std::find(nonterms.begin(), nonterms.end(), elem) == nonterms.end()) {
        return true;
    }
    return false;
};

bool isGrammElem(std::string elem) {
    if (std::find(gramm_elems.begin(), gramm_elems.end(), elem) != gramm_elems.end()) {
        return true;
    }
    return false;
};

std::string getTermByItem(Rule rule, int pos) {
    if (isEndOfRule(rule, pos)) {
        return "";
    }
    return rule.getRight()[pos];
}

bool isEndOfRule(Rule rule, int pos) {
    if (pos >= rule.getRightSize()) {
        return true;
    }
    return false;
}

std::vector<Rule> rules = {};

std::set<std::string> gramm_elems = {"end"};

std::set<std::string> nonterms = {};

void MakeGramm() {
    std::ifstream fin("gramm.txt");
    std::string rule;
    if (fin.is_open())
    {
        while (std::getline(fin, rule))
        {
            int index = rule.find(" ");
            std::string Left = rule.substr(0, index);
            gramm_elems.insert(std::string(Left));
            nonterms.insert(std::string(Left));
            std::vector<std::string> Right = {};
            std::string temp = "";
            for (int i = index + 1; i < rule.size(); i++) {
                if (rule[i] != ' ') {
                    temp += rule[i];
                }
                else {
                    Right.push_back(temp);
                    gramm_elems.insert(std::string(temp));
                    temp = "";
                }
            }
            Right.push_back(temp);
            gramm_elems.insert(std::string(temp));
            rules.push_back(Rule(Left, Right));
        }
    }
    fin.close();
}