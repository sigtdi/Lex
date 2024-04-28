#pragma once
#include <set>

class Rule
{
public:
    Rule(std::string Right, std::vector<std::string> Left);
    virtual ~Rule();

    std::string getLeft();
    std::vector<std::string> getRight();
    int getRightSize();
    bool operator==(Rule const& rhs) const;
protected:

private:
    int sizeRight = 0;
    std::string Left = "";
    std::vector<std::string> Right = {};
};

extern std::vector<Rule> rules;
extern std::set<std::string> gramm_elems;
extern std::set<std::string> nonterms;

bool isNonTerminal(std::string);
bool isTerminal(std::string);
bool isGrammElem(std::string);
std::string getTermByItem(Rule, int);
bool isEndOfRule(Rule, int);
void MakeGramm();

const Rule ZeroRule = { "", {""} };