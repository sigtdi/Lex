#pragma once
#include "grammar.h"
#include "map"

struct Item
{
    Rule rule = ZeroRule;
    int position = 0;
    std::string gramm_symbol = "end";
    bool operator==(Item const& rhs) const {
        return this->rule == rhs.rule && this->position == rhs.position && this->gramm_symbol == rhs.gramm_symbol;
    }
};

struct Action
{
    std::string act = "Error";
    int shift = -1;
    Rule reduce = ZeroRule;
};

using SetOfItems = std::vector<Item>;
using FSM = std::map<std::pair<SetOfItems, std::string>, std::pair<std::string, Rule>>;

class LR
{
public:
    LR();
    virtual ~LR();
    void MakeTables();
    void DumpLRdata();
    bool Analysis(std::istream&);

protected:

private:
    std::vector<SetOfItems> C_Items = {};
    std::map<std::pair<int, std::string>, int> goto_graph = {};
    std::map<std::pair<int, std::string>, Action> action_table = {};

    void MakeItems();
    SetOfItems closure(SetOfItems);
    SetOfItems GOTO(SetOfItems&, std::string);
    void ACTION(Item, int);
    std::set<std::string> first(std::vector<std::string>);
    void ReadTables();
    std::vector<std::string> getTermsAfter(Item);
    Rule addLexemToActionReduce(std::vector<Lexem>& lexList, Action& action);
};





