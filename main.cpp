#include <iostream>
#include <sstream>
#include <vector>
#include "./StaticLib1/framework.h"

using namespace std;

int main()
{
    //string expression = "out";
    //stringstream stream{ expression };
    //Lexer lex(stream);
    //auto l = lex.getNextLexem().first;
    //cout << l;

    MakeGramm();
    LR spasite;
    //spasite.MakeTables();
    spasite.Analysis(std::cin);

    return 0;
}