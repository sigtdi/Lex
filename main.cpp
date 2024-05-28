#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include "./StaticLib1/framework.h"

using namespace std;

int main()
{
    MakeGramm();
    LR spasite;
    ifstream fin;
    fin.open("input.txt");
    //spasite.MakeTables();
    spasite.Analysis(fin);

    return 0;
}