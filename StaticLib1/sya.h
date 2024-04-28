#pragma once


using Lexem = std::pair<std::string, std::string>;

class ShuntingYard
{
public:
    ShuntingYard(std::stringstream&);
    virtual ~ShuntingYard();
    Lexem getNextLexem();

protected:

private:
    int output_count = 0;
    std::stringstream& stream;
    std::vector<Lexem> res = { LEX_EMPTY };
    
};

std::vector<Lexem> ShuntingYard_alg(std::stringstream& stream);